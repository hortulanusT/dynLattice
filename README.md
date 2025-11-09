# dynLattice

[![JOSS status](https://joss.theoj.org/papers/5061ee2b9ca05710f511f2b61c4933e4/status.svg)](https://joss.theoj.org/papers/5061ee2b9ca05710f511f2b61c4933e4)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Docs: 1.11](https://img.shields.io/badge/Docs-1.11-00a6d6)](https://hortulanust.github.io/dynLattice/)

dynLattice is a finite element environment for dynamic simulation of beam networks and lattice metamaterials. It implements geometrically exact beam elements based on the [Simo & Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4) formulation with support for inelastic materials, explicit time marching, and contact mechanics.

## Features

- **Geometrically exact beam elements** with support for large deformations
- **Inelastic material models** for rod elements including elastoplastic behavior
- **Explicit time integration** for dynamic simulations
- **Contact mechanics** between beam elements
- **GMSH integration** for mesh generation
- **Containerized environment** with Apptainer for reproducible simulations
- **Comprehensive test suite** with validation benchmarks

## Publications
<!-- Journal Articles -->
[![Article: Mech. Mater.](https://img.shields.io/badge/Article-Mech.%20Mater.%20(2024)-orange?logo=DOI)](https://doi.org/10.1016/j.mechmat.2024.104952)
[![Article: Comput. Mech.](https://img.shields.io/badge/Article-Comput.%20Mech.%20(2025)-orange?logo=DOI)](https://doi.org/10.1007/s00466-024-02572-3)
[![Article: Int. J. Impact Eng.](https://img.shields.io/badge/Article-Int.%20J.%20Impact%20Eng.%20(2025)-orange?logo=DOI)](https://doi.org/10.1016/j.ijimpeng.2025.105402)
[![Preprint: Mater. Today Adv.](https://img.shields.io/badge/Article-Mater.%20Today%20Adv.%20(2025)-orange?logo=DOI)](https://doi.org/10.1016/j.mtadv.2025.100656)
<!-- Other Publications -->
[![Dissertation: TU Delft](https://img.shields.io/badge/Dissertation-TU%20Delft%20(2025)-00a6d6?logo=DOI)](https://doi.org/10.4233/uuid:9d0b230e-1008-46a2-a193-b9dd3f8c472f)

## Quick Start

### Prerequisites

- [Apptainer](https://apptainer.org/) installed on your system

### Installation

1. Download the [latest release](https://github.com/hortulanusT/dynLattice/releases/latest)

2. Set up the container alias `alias jive="apptainer exec $(pwd)/jive.sif"`

3. Build the code `jive make`

4. Run tests to verify installation: `jive make tests`

### Basic Usage

After building, the main executable `dynLattice` will be available in the `bin/` directory. Run simulations using:

```bash
jive bin/dynLattice input_file.pro
```

Example input files can be found in the `tests/` directory.

## Documentation

- **API Documentation**: Build with `jive make doc` and open `doc/html/index.html`
- **Installation Guide**: [doc/install.md](doc/install.md)
- **Usage Instructions**: [doc/usage.md](doc/usage.md)
- **Benchmarks**: [tests/Benchmarks.md](tests/Benchmarks.md)

## Requirements

The code requires the JIVE finite element toolkit and its dependencies. All requirements are provided through the included Apptainer container (`jive.sif`). To build the documentation, use:

```bash
jive make doc
```

The documentation is also available as [GitHub Pages](https://hortulanust.github.io/dynLattice/)

## Project Structure

```
.
├── .gitignore            ← gitignore file to ignore compiled files and other artifacts
├── .gitattributes        ← git attributes file
├── .github/              ← GitHub workflows and configuration
├── .vscode/              ← VS Code workspace configuration
├── CITATION.cff          ← Citation metadata file
├── LICENSE               ← MIT license file
├── README.md             ← this file
├── CONTRIBUTING.md       ← contribution guidelines
├── codemeta.json         ← software metadata
├── Makefile              ← makefile for building the code and documentation
├── jive.def              ← Apptainer definition file for building the container
├── jive.sif              ← Apptainer container for running the code
├── bin/                  ← Compiled code (ignored by git)
│   ├── dynLattice        ← main executable for the program
│   ├── dynLattice-opt    ← optimized executable for the program
│   └── dynLattice-debug  ← executable with debug symbols
├── doc/                  ← Documentation files
│   ├── doc.mk            ← Makefile for documentation
│   ├── doxygen.conf      ← Doxygen configuration for API documentation
│   ├── install.md        ← Installation instructions
│   ├── main.md           ← Main documentation content
│   ├── usage.md          ← Usage instructions
│   └── html/             ← Generated HTML documentation (ignored by git)
├── paper/                ← JOSS paper and related materials
│   ├── paper.md          ← Main paper content in Markdown
│   ├── paper.bib         ← Bibliography file
│   └── beam_concept.pdf  ← Beam concept illustration
├── src/                  ← Source code
│   ├── main.cpp          ← Main file for the program
│   ├── models/           ← Custom JIVE models
│   ├── modules/          ← Custom JIVE modules
│   ├── materials/        ← Custom JIVE materials
│   ├── misc/             ← Other custom files
│   └── utils/            ← Utility functions and macros
└── tests/                ← Test cases and benchmarks
    ├── testing.mk        ← Makefile for running tests
    ├── Benchmarks.md     ← Overview of all benchmarks
    ├── BeamBenchmarks.md ← Beam element benchmarks
    ├── ContactBenchmarks.md ← Contact mechanics benchmarks
    ├── PlasticBenchmarks.md ← Plasticity benchmarks
    ├── TransientBenchmarks.md ← Transient analysis benchmarks
    ├── beam/             ← Beam test cases
    ├── contact/          ← Contact test cases
    ├── plastic/          ← Plasticity test cases
    ├── transient/        ← Transient analysis test cases
    └── *.png             ← Result images from benchmark tests (ignored by git)
```

## Contributing

We welcome contributions to dynLattice! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:

- Setting up the development environment
- Code organization and style
- Submitting pull requests
- Running tests

## Examples and Validation

The `tests/` directory contains numerous validation cases and benchmarks:

- **Beam Benchmarks**: Validation of beam element formulation
- **Contact Benchmarks**: Contact mechanics validation
- **Plasticity Benchmarks**: Material nonlinearity validation  
- **Transient Benchmarks**: Dynamic analysis validation

Each benchmark includes input files (`.pro`, `.geo`) and reference results.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.