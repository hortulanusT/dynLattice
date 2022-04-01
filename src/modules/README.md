# GMSHInputModule 

This Repository contains a JEM/JIVE Module, that enables inputting .geo files and/or directly setting up the geometry using the GMSH API

In order to include this first execute the command
```
git submodule add --name GMSHInput https://gitlab.tudelft.nl/cm/phds/gmshinputmodule <desired folder>
```
whereas the desired folder needs to be empty. After this make sure, the Module is found by the JEM/JIVE compile enviroment, e.g. by adding to your makefile the following lines
```
subdirs     += <desired folder>
MY_INCDIRS  += <desired folder>
```
