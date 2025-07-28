#######################################################################
##   GMSHInputModule build configuration                             ##
##   Author: Til Gärtner                                             ##
##   Purpose: Add GMSH library dependencies for mesh import          ##
#######################################################################
MY_INCDIRS	+= $(GMSHDIR)/include
MY_LIBDIRS	+= $(GMSHDIR)/lib
MY_LIBS     += gmsh

export LD_RUN_PATH := $(GMSHDIR)/lib:$(LD_RUN_PATH)
