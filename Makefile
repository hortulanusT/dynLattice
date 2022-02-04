program     = bin/nonlinRod

#######################################################################
##   Add desired directories (include sub-dirs seperately)           ##
#######################################################################
subdirs    := src          # head of the source directory for the main file
subdirs    += src/models   # include all the custom models
subdirs    += src/modules  # include all the custom modules
subdirs    += src/modules/paraview  # include the paraview submodule
subdirs    += src/shapes   # include all the custom shapes
subdirs    += src/utils    # include all the utilities
MY_INCDIRS := $(subdirs)

#######################################################################
##   Defining C++ Standard                                           ##
#######################################################################
MY_CXX_STD_FLAGS := '-std=c++17' # for better directory control

#######################################################################
##   Move OBJ-dirs to the bin dir							                       ##
#######################################################################
OBJDIR			= bin/OBJ

#######################################################################
##   Include Jive package and create executable                      ##
#######################################################################
include $(JIVEDIR)/makefiles/packages/*.mk
include $(JIVEDIR)/makefiles/prog.mk