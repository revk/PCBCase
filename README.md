# PCB Case

This is designed to take an KICad 7 `.kicad_pcb` file and produce an openscad file that is the case. See `--help` for arguments.

![275724777_4932056986873909_2086496272107808800_n](https://user-images.githubusercontent.com/996983/158376722-9541f6dd-25f3-4107-ac4b-4513a761b210.jpg)

[Models](models/README.md)

## 🐳 Using Docker

You can run PCBCase without installing local toolchains by using the provided Docker image.

### Quick start
Build once from the repo root:
```bash
docker build -t pcbcase .
```

Show help:

```bash
docker run --rm -v "$PWD":/workbench -w /workbench pcbcase case --help
```

### Convert the included test board (SCAD)

This reproduces the Makefile rule:

```bash
docker run --rm -v "$PWD":/workbench -w /workbench pcbcase \
  case -o PCB/Test/Test.scad PCB/Test/Test.kicad_pcb -M models
```

### Render the SCAD to STL

```bash
docker run --rm -v "$PWD":/workbench -w /workbench pcbcase \
  openscad -o PCB/Test/Test.stl PCB/Test/Test.scad
```

## 🐳 Using Docker Compose

A ready-to-use `docker-compose.yml` is provided. It already chains:
1) Generate SCAD from the test KiCad board
2) Render STL from the generated SCAD

### Run the built-in one-liner for the test board
```bash
docker compose build
docker compose run --rm pcbcase
````

This produces:

* `PCB/Test/Test.scad`
* `PCB/Test/Test.stl`
