# Rod-based Lattice Simulations
This repository contains JEM/JIVE code and corresponding scripts to execute a FEA based on nonlinear Beam elements implemented after [Simo/Vu-Quoc](https://dx.doi.org/10.1016/0045-7825(86)90079-4) with some minor adaptations due to [Crisfield/Jelenić](https://dx.doi.org/10.1098/rspa.1999.0352). It also contains some python scripts for easier execution, they all should be executed from the main directory.

## Requirements
- **Apptainer 1.2**
  - JEM/JIVE 3.0
  - OpenMPI 4.1.1
  - GMSH 4.9.5
  - HDF5 1.12.2

To build the container put all the required archives into a folder called `zip` and run the command `sudo apptainer build /path/to/jive.sif /path/to/jive.def`.

To create an alias used in the recipes run `alias jive='apptainer -s exec -e /path/to/jive.sif'`.

## Organization
```
.
├── .gitignore
├── .gitmodules
├── LICENSE
├── README.md
├── NOTES.md
├── Makefile
├── jive.def
├── bin                 <- Compiled and external code, ignored by git
├── src
│   ├── main.cpp        <- origin file for the JEM/JIVE program
│   ├── models          <- custom JIVE models
│   ├── modules         <- custom JIVE modules
│   ├── modules         <- custom JIVE materials
│   ├── misc            <- other custom JIVE files
│   └── utils           <- utility functions and macros
├── scripts
│   ├── testing         <- ptyhon scripts for some testing runs
│   ├── running         <- ptyhon scripts for the real runs
│   └── cluster         <- bash scripts for running on the clusters
├── tests
│   ├── testing.mk      <- makefile for testing setup
│   ├── element         <- test setups for single elments
│   ├── beam            <- test setups for static beam (networks)
│   ├── transient       <- test setups for dynamic computations
│   ├── plastic         <- test setups for dynamic beam (networkds)
│   └── manual          <- test setup for manual test runs
└── studies
    ├── running*.mk     <- makefiles for running studies
    ├── configs         <- folder containing some standard settings
    ├── geometries      <- folder containing the different geometries
    ├── programs        <- folder containing different studies to conducts
    ├── output          <- raw data output
    └── results         <- processed results
```


## License
[MIT License](LICENSE)
