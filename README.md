# 598APE-HW1

This repository contains our optimized C++ ray tracer for CS 598 APE Homework 1.

## 1. Build

The artifact requires Docker, Git, and Python 3. Build the Docker image and the
ray tracer from the repository root:

```bash
docker build -t "$USER/598ape" docker
./dockerrun.sh make clean
./dockerrun.sh make -j4
```

For example, render the Piano Room with:

```bash
./dockerrun.sh ./main.exe -i inputs/pianoroom.ray --ppm --no-movie \
  -o output/pianoroom.ppm -H 500 -W 500
```

## 2. Benchmark

`benchmark.py` benchmarks the specified commits and workloads. For example:

```bash
./benchmark.py --commits 1c64c5c 87c372a --ops pianoroom --csv
```

The available workloads are `pianoroom`, `globe`, `sphere`, and `elephant`.

## 3. Evaluation

The following commands reproduce the results presented in the report.

### Overall Performance

The overall performance figure in the report compares the baseline, O1
compiler configuration, O2 closest-intersection scan, O3 work-reduction
changes, and O4 OpenMP parallelization:

```bash
./benchmark.py \
  --commits 19bbc81 8ee75ee 0a2ff13 1c64c5c 87c372a \
  --ops pianoroom globe --csv
```

The Sphere baseline took about 74 minutes for one run on our VM. Baseline and
O1 Elephant did not finish within one hour.

### Compiler Selection

The compiler comparison table in the report evaluates GCC, Clang, icpx, Clang
with `-ffast-math -fhonor-infinities`, and icpx with `-fp-model=precise`, in
that order:

```bash
./benchmark.py \
  --commits 7e4b8ee 48228a7 8ee75ee d617129 7074d3f \
  --ops pianoroom globe --csv
```

### Profiling-Guided Optimization

The O3 breakdown figure in the report compares O2, passing rays by reference,
using dot products for the orthonormal box basis, and reusing surface normals:

```bash
./benchmark.py \
  --commits 0a2ff13 b8af7de ec22363 1c64c5c \
  --ops pianoroom globe sphere --csv
```

### OpenMP Scheduling

The OpenMP scheduling comparison in the report evaluates static, dynamic, and
guided scheduling, in that order:

```bash
./benchmark.py --commits bed8fec 2344db3 2038ab9 --ops globe --csv
```
