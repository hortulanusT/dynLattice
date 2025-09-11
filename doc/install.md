\page install Installation

# Requirements
- **[Apptainer](https://apptainer.org/)**

# 1. Install *apptainer*
1. Install `apptainer` following the [quick start guide](https://apptainer.org/get-started/).

# (Optional) build the *.sif* file yourself
A `.def` file is provided to build the container yourself.
1. Put all the required archives into a directory called `zips`.
    - [JEM/JIVE 3.0](https://dynaflow.com/software/jive/jive-downloads/)
    - [OpenMPI 4.1.1](https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.1.tar.gz)
    - [GMSH 4.9.5](https://gmsh.info/bin/Linux/gmsh-4.9.5-Linux64-sdk.tgz)
    - [HDF5 1.12.2](https://support.hdfgroup.org/archive/support/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/hdf5-1.12.2.tar.gz)
2. Run the command `sudo apptainer build jive.sif jive.def`.

# 2. Get the repository and build the documentation
1. Get [latest release](https://github.com/hortulanusT/dynLattice/releases/latest) of the repository.
2. Define an appropriate alias for running inside the apptainer `alias jive="apptainer exec $(pwd)/jive.sif"`
3. Build the documentation via `jive make docs` (this will execute the test cases as well)

# 3. Using the code
1. You can build the code via `jive make` and then execute any test via `jive bin/dynLattice path/to/inputs.pro`
2. Once you are done with your setup and sure everything works, build the optimized version via `jive make opt` and then run `bin/dynLattice-opt path/to/inputs.pro`
3. If you want to debug your own additions using the GNU Project debugger build `jive make debug` and then run `jive gdb bin/dynLattice-dbg`
    - proceed with `(gdb) run path/to/inputs.pro`
    - use `(gdb) call dprint(&variable)` to get the variable printed if it is a Vector or a Matrix
