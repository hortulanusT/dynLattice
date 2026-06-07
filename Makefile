#######################################################################
##   Run this makefile with the maximum number of cores              ##
#######################################################################
MAKEFLAGS := --jobs=$(shell nproc)

#######################################################################
##   Name of the Programm																	           ##
#######################################################################
program     = bin/dynLattice

#######################################################################
##   Move OBJ-dirs to the bin dir							                       ##
#######################################################################
OBJDIR			= bin/OBJ

#######################################################################
##   Add desired directories (include sub-dirs seperately)           ##
#######################################################################
SRCDIR			:= src
subdirs		 	:= ${shell find $(SRCDIR) -type d}
MY_INCDIRS 	:= $(SRCDIR)

#######################################################################
##   Defining C++ Standard & stricter warnings                       ##
#######################################################################
# Default build: regular warnings only.
# Hygienic build: enable stricter warning hygiene with:
#   make HYGIENE=1
HYGIENE ?= 0

MY_WARN_FLAGS :=
ifeq ($(HYGIENE),1)
MY_WARN_FLAGS += '-Wextra'
MY_WARN_FLAGS += '-Wpedantic'
MY_WARN_FLAGS += '-Wshadow'
MY_WARN_FLAGS += '-Wsuggest-override'
MY_WARN_FLAGS += '-Wold-style-cast'
MY_WARN_FLAGS += '-Woverloaded-virtual'
MY_WARN_FLAGS += '-Wdouble-promotion'
MY_WARN_FLAGS += '-Wundef'
MY_WARN_FLAGS += '-Wnon-virtual-dtor'
MY_WARN_FLAGS += '-Winit-self'
MY_WARN_FLAGS += '-Wconversion'
MY_WARN_FLAGS += '-Wsign-conversion'
MY_WARN_FLAGS += '-Wfloat-equal'
endif

MY_CXX_STD_FLAGS := '-std=c++17' $(MY_WARN_FLAGS)
#######################################################################
##   Include Jive package and create executable                      ##
#######################################################################
include $(JIVEDIR)/makefiles/packages/*.mk
include $(JIVEDIR)/makefiles/prog.mk

MKDIR_P	= mkdir -p --

#######################################################################
##   Include submodules specifics                                    ##
#######################################################################
include ${shell find $(SRCDIR) -name *.mk}

#######################################################################
##   add openMPI libraries                                           ##
#######################################################################
export LD_RUN_PATH := $(MPIDIR)/lib:$(LD_RUN_PATH)

#######################################################################
##   Include Test targets 											                     ##
#######################################################################
include tests/testing.mk

#######################################################################
##   Include Documentation targets                                   ##
#######################################################################
include doc/doc.mk