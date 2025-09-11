\page install Installation

# Requirements
- **[Apptainer](https://apptainer.org/)**
- [JEM/JIVE 3.0](https://dynaflow.com/software/jive/jive-downloads/)
- [OpenMPI 4.1.1](https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.1.tar.gz)
- [GMSH 4.9.5](https://gmsh.info/bin/Linux/gmsh-4.9.5-Linux64-sdk.tgz)
- [HDF5 1.12.2](https://support.hdfgroup.org/archive/support/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/hdf5-1.12.2.tar.gz)

# 1. Install `apptainer` and get the `.sif` file
1. Install `apptainer` following the [quick start guide](https://apptainer.org/get-started/).
2. Download the pre-built `.sif` from the repository or build it yourself.
3. Define an alias for easier executing, e.g. `alias jive="apptainer -s exec -e --env DISPLAY=$DISPLAY </full/path/to/>jive.sif"` 

# (Optional) build the `.sif` file yourself
A `.def` file is provided to build the container yourself.
To build the container put all the required archives into a folder called `zip` and run the command `sudo apptainer build jive.sif jive.def`.
You can subsequently move the `jive.sif` to any location you prefer.

# 2. Get this repository and build the documentation
1. Get the repository via `git clone git@github.com:hortulanusT/dynLattice.git`
2. Build the documentation via `jive make docs` (this will execute the test cases as well)

# 3. Using the code
1. You can build the code via `jive make` and then execute any test via `bin/dynLattice path/to/inputs.pro`
2. Once you are done with your setup and sure everything works, build the optimized version via `jive make opt` and then run `bin/dynLattice-opt path/to/inputs.pro`
3. If you want to debug your own additions using the GNU Project debugger build `jive make debug` and then run `jive gdb bin/dynLattice-dbg`
    - proceed with `(gdb) run path/to/inputs.pro`
    - use `(gdb) call dprint(&variable)` to get the variable printed if it is a Vector or a Matrix
