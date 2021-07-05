
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
##   Include Jive package and create executable                      ##
#######################################################################

include $(JIVEDIR)/makefiles/packages/*.mk
include $(JIVEDIR)/makefiles/prog.mk