#!/usr/bin/env python3

import argparse
import datetime
import shutil
import subprocess
from pathlib import Path


root = Path(__file__).resolve().parent

ops = {
    "pianoroom": [
        "./main.exe", "-i", "inputs/pianoroom.ray", "--ppm",
        "--no-movie", "-o", "profile/pianoroom.ppm",
        "-H", "500", "-W", "500"
    ],
    "globe": [
        "./main.exe", "-i", "inputs/globe.ray", "--ppm",
        "--no-movie", "-a", "inputs/globe.animate", "-F", "24",
        "-o", "profile/globe"
    ],
    "sphere": [
        "./main.exe", "-i", "inputs/elephant.ray", "--ppm",
        "--no-movie", "-a", "inputs/elephant.animate", "-F", "24",
        "-W", "100", "-H", "100", "-o", "profile/sphere"
    ],
    "elephant": [
        "./main.exe", "-i", "inputs/bench-elephant.ray", "--ppm",
        "--no-movie", "-a", "inputs/elephant.animate", "-F", "24",
        "-W", "100", "-H", "100", "-o", "profile/elephant"
    ]
}


def docker(worktree, command):
    subprocess.run(
        [str(root / "dockerrun.sh")] + command,
        cwd=worktree, check=True
    )


def elephant_input(worktree):
    source = worktree / "inputs" / "elephant.ray"
    target = worktree / "inputs" / "bench-elephant.ray"
    text = source.read_text()
    text = text.replace(
        "data/x.txt 1586 data/f.txt 3168 -1.58 -.43 2.7",
        "data/elepx.txt 62779 data/elepf.txt 111748 -1.58 -.43 2.7",
        1
    )
    target.write_text(text)


parser = argparse.ArgumentParser()
parser.add_argument("--commit")
parser.add_argument("--op", choices=ops, default="pianoroom")
parser.add_argument("--freq", type=int)
args = parser.parse_args()

stamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
run_dir = root / "runs" / stamp
worktree = run_dir / "worktree"
commit = args.commit or "HEAD"
run_dir.mkdir(parents=True)

subprocess.run(
    ["git", "worktree", "add", "--detach", str(worktree), commit],
    cwd=root, check=True
)

if not args.commit:
    patch = subprocess.check_output(
        ["git", "diff", "HEAD", "--binary"], cwd=root
    )
    subprocess.run(["git", "apply"], cwd=worktree, input=patch, check=True)
    untracked = subprocess.check_output(
        ["git", "ls-files", "--others", "--exclude-standard", "-z"],
        cwd=root
    ).decode().split("\0")
    for name in untracked[:-1]:
        target = worktree / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(root / name, target)

subprocess.run(
    ["git", "clone", "--depth", "1",
     "https://github.com/brendangregg/FlameGraph.git", "FlameGraph"],
    cwd=worktree, check=True
)

(worktree / "profile").mkdir()
if args.op == "elephant":
    elephant_input(worktree)

docker(worktree, ["make", "clean"])
docker(worktree, ["make", "-j4"])
perf = ["perf", "record", "-e", "cpu-clock:u"]
if args.freq:
    perf += ["-F", str(args.freq)]
perf += ["--call-graph", "dwarf", "-o", "profile/perf.data", "--"]
docker(worktree, perf + ops[args.op])
docker(worktree, [
    "bash", "-c",
    "perf script -i profile/perf.data > profile/perf.txt"
])
docker(worktree, [
    "bash", "-c",
    "FlameGraph/stackcollapse-perf.pl profile/perf.txt "
    "> profile/perf.folded"
])
docker(worktree, [
    "bash", "-c",
    "FlameGraph/flamegraph.pl --countname samples profile/perf.folded "
    "> profile/flamegraph.svg"
])
docker(worktree, [
    "bash", "-c",
    "perf report --stdio -i profile/perf.data > profile/report.txt"
])

print("\n" + str(worktree / "profile" / "flamegraph.svg"))
