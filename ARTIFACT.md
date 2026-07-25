# Artifact Guide

> **This document describes the FROZEN paper artifact**, which runs the
> original Python-orchestrated pipeline inside a Docker image. It is kept as
> published and is not updated to follow the tool.
>
> For the current tool — one binary, built and run locally, no container —
> see [ReadMe.md](ReadMe.md).

This document describes the reproducible artifact package for SMART/MAPminer,
located in [`artifact/`](artifact/). The artifact runs every experiment inside
a self-contained Docker image so no host toolchain is required beyond
Docker (or podman) and git.

## Prerequisites

- Docker or podman (select with `ENGINE=docker|podman`)
- git (used to fetch the pre-generated mutation benchmarks)
- ~20 GB free disk space for the image, mutation benchmarks, and results

## Getting the image

The artifact image is named `mapminer`. Either load the released archive:

```bash
cd artifact
# Verify and load the released image archive
sha256sum -c image.tar.gz.sha256
docker load -i image.tar.gz
```

or rebuild it from source with the release Dockerfile:

```bash
docker build -f artifact/Dockerfile -t mapminer .
```

`artifact/run.sh` will automatically `docker load` the archive if the
`mapminer` image is not already present.

## Quick start

All experiment scripts live in `artifact/` and take the config name
(without `.json`) followed by the benchmark name:

```bash
cd artifact
./run.sh smart c17          # single run: baseline SMART on c17
./run.sh block_msa s38417   # full MAPminer config on a large benchmark
```

Each run creates a fresh container, copies the mutation benchmark data in,
executes `python run.py <benchmark> Config/<config>.json`, and copies the
results out to `artifact/RawData/<config>_<benchmark>/`. The script refuses
to overwrite an existing output directory.

Environment overrides: `ENGINE`, `IMAGE`, `IMAGE_ARCHIVE`, `MB_CACHE_DIR`,
`MB_REPO_URL`, `HUGE_STABLE` (see `./run.sh` with no arguments for details).

## Experiment matrices

```bash
./run_smoke.sh              # small sanity matrix ({smart,parallel,block_msa} x {c17,s27})
./run_suggested.sh          # suggested subset of the paper experiments
./run_subset.sh             # medium subset
./run_all.sh -j 4           # full paper matrix, 4 runs in parallel
./run_caseStudy.sh          # NRU cache case study
```

The matrix scripts define their `CONFIGS` and `BENCHMARKS` arrays at the top
of each file; edit those lists to trim or extend a run. The largest
benchmarks (s13207, s15850, s35932, s38417, s38584, s9234) are fetched from
dedicated branches of the MutationBenchmark repository on first use and
cached under `artifact/MutationBenchmark/`.

## Generating tables and figures

```bash
cd artifact
make table      # analyzes RawData/ (falls back to PrecomputedRawData/ if empty)
make figures    # renders the paper figures from the table data
make case-study # runs and tabulates the NRU case study
make clean      # removes RawData/, Table/, Figures/ contents
```

`PrecomputedRawData/` contains the raw data used in the paper, so tables and
figures can be regenerated without re-running any experiments.

## Exporting a new image archive

To produce an updated `image.tar.gz` after changing the Dockerfile or code:

```bash
docker build -f artifact/Dockerfile -t mapminer .
docker save mapminer | gzip > artifact/image.tar.gz
(cd artifact && sha256sum image.tar.gz > image.tar.gz.sha256)
```

See also `scripts/release_artifact.sh` for the scripted release flow.

## Note on argument order

`artifact/run.sh` takes `<config> <benchmark>` (config first, no `.json`
suffix), while the underlying `run.py` takes `<benchmark> [Config/<config>.json]`
(benchmark first). The wrapper handles the translation; only keep this in
mind when invoking `run.py` directly inside the container.
