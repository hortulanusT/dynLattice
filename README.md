# Rod-based Lattice Simulations
This repository contains JEM/JIVE code to simulate lattices undergoing high strain-rate dynamic deformation based on nonlinear beam elements. It also contains some test files to verify the correctness of the implementation.

## Requirements
- **Apptainer**
  - [JEM/JIVE 3.0](https://dynaflow.com/software/jive/jive-downloads/)
  - [OpenMPI 4.1.1](https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.1.tar.gz)
  - [GMSH 4.9.5](https://gmsh.info/bin/Linux/gmsh-4.9.5-Linux64-sdk.tgz)
  - [HDF5 1.12.2](https://support.hdfgroup.org/archive/support/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/hdf5-1.12.2.tar.gz)

To build the container put all the required archives into a folder called `zip` and run the command `sudo apptainer build jive.sif jive.def`.
You can subsequently move the `jive.sif` to any location you prefer.

To create an alias for easier executing use `alias jive='apptainer -s exec -e </full/path/to/>jive.sif'`.
The test-cases can subsequently be run using `jive make tests`.

## Organization
```
.
├── jive.def            ← apptainer definition file needed to run everything
├── README.md           ← this README
├── Makefile            ← makefile for the project
├── bin                 ← compiled and external code, ignored by git
├── doc                 ← output folder for the documentation, ignored by git
├── src
│   ├── main.cpp        ← origin file for the JEM/JIVE program
│   ├── models          ← custom JIVE models
│   ├── modules         ← custom JIVE modules
│   ├── modules         ← custom JIVE materials
│   ├── misc            ← other custom JIVE files
│   └── utils           ← utility functions and macros
└── tests
    ├── testing.mk      ← makefile for testing setup
    ├── element         ← test setups for single elements
    ├── beam            ← test setups for static beam (networks)
    ├── transient       ← test setups for dynamic computations
    └── plastic         ← test setups for inelastic beam (networks)
```

## License
[MIT License](LICENSE)
