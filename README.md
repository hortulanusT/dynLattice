# Rod-based Lattice Simulations
This repository contains JEM/JIVE code and corresponding scripts to execute a FEA based on nonlinear Beam elements implemented after [Simo/Vu-Quoc](https://dx.doi.org/10.1016/0045-7825(86)90079-4) with some minor adaptations due to [Crisfield/Jelenić](https://dx.doi.org/10.1098/rspa.1999.0352). It also contains some python scripts for easier execution, they all should be executed from the main directory.

**JEM/JIVE Version: 3.0**

**C++17 used in compilation**

## Organization
```
.
├── .gitignore
├── .gitmodules
├── LICENSE
├── README.md
├── NOTES.md
├── Makefile
├── bin                 <- Compiled and external code, ignored by git
├── src
│   ├── main.cpp        <- origin file for the JEM/JIVE program
│   ├── models          <- custom JIVE models
│   ├── modules         <- custom JIVE modules
│   ├── misc            <- other custom JIVE files
│   └── utils           <- utility functions and macros
├── scripts
│   ├── testing         <- ptyhon scripts for some testing runs
│   ├── running         <- ptyhon scripts for the real runs
├── tests
│   ├── testing.mk      <- makefile for testing setup
│   ├── element         <- test setups for single elments
│   └── beam            <- tests from the Simo/Vu-Quoc Paper
└── studies
    ├── running.mk      <- makefile for running studies
    ├── configs         <- folder containing some standard settings
    ├── geometries      <- folder containing the different geometries
    ├── programs        <- folder containing different studies to conducts
    └── <class>         <- folder for geometry classes (re-entrant, doubleArrowhead3D, ...)
        ├── output      <- raw data output
        └── results     <- processed results
```


## License
[MIT License](LICENSE)
