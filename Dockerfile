# syntax=docker/dockerfile:1
##
## Stage 1 — build the PCBCase binaries
##
FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential git ca-certificates pkg-config libpopt-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
# If you place this Dockerfile at the repo root, this will copy sources in.
COPY . /src

# Pull submodules if present (repo has a .gitmodules file)
RUN [ -f .gitmodules ] && git submodule update --init --recursive || true

# Build via the project's Makefile
RUN make

##
## Stage 2 — minimal runtime with OpenSCAD CLI to render STL/3MF
##
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    openscad libpopt0 \
 && rm -rf /var/lib/apt/lists/*

# Where you'll mount your boards/files from the host
WORKDIR /workbench

# Copy the compiled binary and the OpenSCAD library
COPY --from=build /src/case /usr/local/bin/case
COPY --from=build /src/case-scad /opt/case-scad

# Make the case-scad modules discoverable by OpenSCAD
ENV OPENSCADPATH=/opt/case-scad

# Default shows the tool's help so usage is self-discoverable
CMD ["case", "--help"]
