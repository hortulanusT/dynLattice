# GMSHInputModule 

This Repository contains a JEM/JIVE Module, that enables inputting .geo files and/or directly setting up the geometry using the GMSH API

In order to include this first execute the command
```
git submodule add --name GMSHInput https://gitlab.tudelft.nl/cm/phds/gmshinputmodule <desired folder>
```
whereas the desired folder needs to be empty. After this make sure, the Module is found by the JEM/JIVE compile enviroment, e.g. by adding to your makefile the following lines
```
include <desired folder>/gmsh.mk
```
also, you need to ensure, that the libraries are found by the compiler, this may be done by setting the environment variable `GMSHDIR` just like `JEMDIR` and `JIVEDIR`, e.g.
```
export GMSHDIR=/path/to/the/gmsh-sdk
```
The GMSH sdk can be downloaded [here](http://gmsh.info/bin/Linux/) (version 4.9.5) 