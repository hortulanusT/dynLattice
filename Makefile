program     = bin/nonlinRod

#######################################################################
##   Add desired directories (include sub-dirs seperately)           ##
#######################################################################
subdirs		 	:= ${shell find src -type d}
MY_INCDIRS 	:= src

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