#######################################################################
##   Adding GMSH Library                                             ##
#######################################################################
MY_INCDIRS	+= $(GMSHDIR)/include
MY_LIBDIRS	+= $(GMSHDIR)/lib
MY_LIBS     += gmsh

export LD_RUN_PATH := $(GMSHDIR)/lib:$(LD_RUN_PATH)