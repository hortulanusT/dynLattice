# Rod-based Lattice Simulations
This repository contains JEM/JIVE code and corresponding scripts to execute a FEA based on nonlinear Beam elements implemented after [Simo, Vu-Quoc (1986)](https://doi.org/10.1016/0045-7825(86)90079-4) as well as some documentation.

## Requirements
You can use the code in this repository with the provided Apptainer container. To run the code, you need to have [Apptainer](https://apptainer.org/) installed on your system. The code is built using the `Makefile` in the root directory. Using the provided container, you can build also build the documentation using `<apptainer exec jive.sif> make doc`.

## Organization
```
.
├── .gitignore            <- gitignore file to ignore compiled files and other artifacts
├── .gitattributes        <- git attributes file
├── .github/              <- GitHub workflows and configuration
├── .vscode/              <- VS Code workspace configuration
├── LICENSE               <- license file
├── README                <- this file
├── Makefile              <- makefile for building the code and documentation
├── jive.def              <- Apptainer definition file for building the container
├── jive.sif              <- Apptainer container for running the code
├── bin/                  <- Compiled code, ignored by git
│   ├── dynLattice        <- main executable for the program
│   ├── dynLattice-opt    <- optimized executable for the program
│   └── dynLattice-debug  <- executable with debug symbols
├── doc/                  <- Documentation files
│   ├── Documentation.html <- Main documentation file
│   ├── doxygen.conf      <- Doxygen configuration for API documentation
│   ├── install.md        <- Installation instructions
│   ├── main.md           <- Main documentation content
│   ├── usage.md          <- Usage instructions
│   └── html/             <- Generated HTML documentation, ignored by git
├── paper/                <- JOSS paper and related materials
│   ├── paper.md          <- Main paper content in Markdown
│   ├── paper.bib         <- Bibliography file
│   └── beam_concept.pdf  <- Beam concept illustration
├── src/                  <- Source code
│   ├── main.cpp          <- Main file for the program
│   ├── models/           <- Custom JIVE models
│   ├── modules/          <- Custom JIVE modules
│   ├── materials/        <- Custom JIVE materials
│   ├── misc/             <- Other custom files
│   └── utils/            <- Utility functions and macros
└── tests/                <- Test cases and benchmarks
    ├── testing.mk        <- Makefile for running tests
    ├── Benchmarks.md     <- Overview of all benchmarks
    ├── BeamBenchmarks.md <- Beam element benchmarks
    ├── ContactBenchmarks.md <- Contact mechanics benchmarks
    ├── PlasticBenchmarks.md <- Plasticity benchmarks
    ├── TransientBenchmarks.md <- Transient analysis benchmarks
    ├── beam/             <- Beam test cases
    ├── contact/          <- Contact test cases
    ├── plastic/          <- Plasticity test cases
    ├── transient/        <- Transient analysis test cases
    └── *.png             <- Result images from benchmark tests, ignored by git
```

## License
[MIT License](LICENSE)