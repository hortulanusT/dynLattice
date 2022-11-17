#######################################################################
##   Run this makefile with the maximum number of cores              ##
#######################################################################
MAKEFLAGS := --jobs=$(shell nproc)

#######################################################################
##   Name of the Programm																	           ##
#######################################################################
program     = bin/nonlinRod

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
##   Defining C++ Standard                                           ##
#######################################################################
MY_CXX_STD_FLAGS := '-std=c++17'

#######################################################################
##   report git hash                                                 ##
#######################################################################
$(shell touch $(SRCDIR)/modules/GitReportModule.cpp)
GIT_COMMIT := $(shell git rev-parse HEAD)
GIT_DIRTY := $(shell git status --porcelain | wc -l)
ifeq ($(GIT_DIRTY), 0)
	MY_CXX_STD_FLAGS += '-DGIT_HASH="$(GIT_COMMIT)"'
else
	MY_CXX_STD_FLAGS += '-DGIT_HASH="$(GIT_COMMIT) !!! $(GIT_DIRTY) UNCOMITTED CHANGES !!!"'
endif

#######################################################################
##   Include submodules specifics                                    ##
#######################################################################
include ${shell find $(SRCDIR) -name *.mk}

#######################################################################
##   add opemMPI libraries                                           ##
#######################################################################
export LD_RUN_PATH := $(MPIDIR)/lib:$(LD_RUN_PATH)

#######################################################################
##   Include Jive package and create executable                      ##
#######################################################################
include $(JIVEDIR)/makefiles/packages/*.mk
include $(JIVEDIR)/makefiles/prog.mk

MKDIR_P	= mkdir -p --

#######################################################################
##   Include Test Cases													                     ##
#######################################################################
include tests/testing.mk

#######################################################################
##   Include the Study Cases  									                     ##
#######################################################################
include studies/running.mk


#######################################################################
##   make git hash stay always current                               ##
#######################################################################
GitReportObj = $(OBJDIR)/GitReportModule.o $(OBJDIR_OPT)/GitReportModule.o $(OBJDIR_DBG)/GitReportModule.o

src/modules/GitReportModule.cpp: $(filter-out $(GitReportObj), $(objects))