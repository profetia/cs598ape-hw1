#!/usr/bin/env python3

import argparse
import csv
import datetime
import re
import shutil
import subprocess
from pathlib import Path


root = Path(__file__).resolve().parent
runs = 5

ops = {
    "pianoroom": [
        "./main.exe", "-i", "inputs/pianoroom.ray", "--ppm",
        "--no-movie", "-o", "output/bench-pianoroom.ppm",
        "-H", "500", "-W", "500"
    ],
    "globe": [
        "./main.exe", "-i", "inputs/globe.ray", "--ppm",
        "--no-movie", "-a", "inputs/globe.animate", "-F", "24",
        "-o", "output/bench-globe"
    ],
    "sphere": [
        "./main.exe", "-i", "inputs/elephant.ray", "--ppm",
        "--no-movie", "-a", "inputs/elephant.animate", "-F", "24",
        "-W", "100", "-H", "100", "-o", "output/bench-sphere"
    ],
    "elephant": [
        "./main.exe", "-i", "inputs/bench-elephant.ray", "--ppm",
        "--no-movie", "-a", "inputs/elephant.animate", "-F", "24",
        "-W", "100", "-H", "100", "-o", "output/bench-elephant"
    ]
}


def docker(worktree, command, capture=False):
    result = subprocess.run(
        [str(root / "dockerrun.sh")] + command,
        cwd=worktree, check=True, text=True,
        stdout=subprocess.PIPE if capture else None
    )
    return result.stdout


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


def show_table(names, rows):
    table = [["commit"] + names]
    for commit, values in rows:
        table.append([commit] + ["%.6f" % values[name] for name in names])

    widths = []
    for column in range(len(table[0])):
        widths.append(max(len(row[column]) for row in table))

    for number, row in enumerate(table):
        print(" | ".join(row[n].ljust(widths[n]) for n in range(len(row))))
        if number == 0:
            print("-+-".join("-" * width for width in widths))


parser = argparse.ArgumentParser()
parser.add_argument("--commits", nargs="*")
parser.add_argument("--ops", nargs="*")
parser.add_argument("--csv", action="store_true")
args = parser.parse_args()

dirty = not args.commits
commits = args.commits or ["HEAD"]
names = args.ops or list(ops)

stamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
run_dir = root / "runs" / stamp
worktree = run_dir / "worktree"
run_dir.mkdir(parents=True)

first = subprocess.check_output(
    ["git", "rev-parse", commits[0]], cwd=root, text=True
).strip()
subprocess.run(
    ["git", "worktree", "add", "--detach", str(worktree), first],
    cwd=root, check=True
)

if dirty:
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

rows = []
timer = re.compile(r"Total time to create images=([0-9.]+) seconds")

for commit in commits:
    commit_id = subprocess.check_output(
        ["git", "rev-parse", commit], cwd=root, text=True
    ).strip()
    if not dirty:
        subprocess.run(
            ["git", "checkout", "--detach", "--force", commit_id],
            cwd=worktree, check=True
        )

    docker(worktree, ["make", "clean"])
    docker(worktree, ["make", "-j4"])
    if "elephant" in names:
        elephant_input(worktree)

    values = {}
    label = "working tree" if dirty else commit
    print("\n" + label + " (" + commit_id[:12] + ")")
    for name in names:
        times = []
        for number in range(runs):
            output = docker(worktree, ops[name], True)
            seconds = float(timer.search(output).group(1))
            times.append(seconds)
            print("  %-10s %2d/%d  %.6f" %
                  (name, number + 1, runs, seconds))
        values[name] = min(times)

    rows.append((label + " (" + commit_id[:12] + ")", values))

print()
show_table(names, rows)

if args.csv:
    csv_file = run_dir / "results.csv"
    with csv_file.open("w", newline="") as output:
        writer = csv.writer(output)
        writer.writerow(["commit"] + names)
        for commit, values in rows:
            writer.writerow([commit] + [values[name] for name in names])
    print("\n" + str(csv_file))
