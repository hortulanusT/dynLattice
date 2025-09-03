# dynLattice

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Static Badge](https://img.shields.io/badge/Docs-online-00a6d6)](https://hortulanust.github.io/dynLattice/)

dynLattice is a finite element environment for dynamic simulation of beam networks and lattice metamaterials. It implements geometrically exact beam elements based on the [Simo & Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4) formulation with support for inelastic materials, explicit time marching, and contact mechanics.

## Features

- **Geometrically exact beam elements** with support for large deformations
- **Inelastic material models** for rod elements including elastoplastic behavior
- **Explicit time integration** for dynamic simulations
- **Contact mechanics** between beam elements
- **GMSH integration** for mesh generation
- **Containerized environment** with Apptainer for reproducible simulations
- **Comprehensive test suite** with validation benchmarks

## Quick Start

### Prerequisites

- [Apptainer](https://apptainer.org/) installed on your system
- Git for cloning the repository

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/hortulanusT/dynLattice.git
   cd dynLattice
   ```

2. Set up the container alias:
   ```bash
   alias jive="apptainer exec $(pwd)/jive.sif"
   ```

3. Build the code:
   ```bash
   jive make
   ```

4. Run tests to verify installation:
   ```bash
   jive make tests
   ```

### Basic Usage

After building, the main executable `dynLattice` will be available in the `bin/` directory. Run simulations using:

```bash
jive ./bin/dynLattice input_file.pro
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
│   ├── Documentation.html ← Main documentation file
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