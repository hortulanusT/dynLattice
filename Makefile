program     = bin/nonlinRod

#######################################################################
##   Add desired directories (include sub-dirs seperately)           ##
#######################################################################
SRCDIR			:= src
subdirs		 	:= ${shell find $(SRCDIR) -type d}
MY_INCDIRS 	:= $(SRCDIR)

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