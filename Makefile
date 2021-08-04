
program     = nonlinRod

#######################################################################
##   Add desired directories (include sub-dirs seperately)           ##
#######################################################################

# Source file location
subdirs 	 := models		 			# include all the custom models
subdirs 	 += modules       	# include all the custom modules
subdirs    += utils						# include all the utilities
# Include directories
MY_INCDIRS := $(subdirs)


#######################################################################
##   Defining C++ Standard										                       ##
#######################################################################

# MY_CXX_STD_FLAGS := '-std=c++17'

#######################################################################
##   Include Jive package and create executable                      ##
#######################################################################

include $(JIVEDIR)/makefiles/packages/*.mk
include $(JIVEDIR)/makefiles/prog.mk