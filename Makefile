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
##   Defining C++ Standard                                           ##
#######################################################################
MY_CXX_STD_FLAGS := '-std=c++17'

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
##   report git hash                                                 ##
#######################################################################
GIT_COMMIT := $(shell git rev-parse HEAD)
GIT_DIRTY := $(shell git status --porcelain | wc -l)
ifeq ($(GIT_DIRTY), 0)
	MY_CXX_STD_FLAGS += '-DGIT_HASH="$(GIT_COMMIT)"'
else
	MY_CXX_STD_FLAGS += '-DGIT_HASH="$(GIT_COMMIT) !!! $(GIT_DIRTY) UNCOMITTED !!!"'
endif

srcfiles := $(shell find $(SRCDIR) -type f -name '*.cpp')
$(OBJDIR)/GitReportModule.o: $(srcfiles)
$(OBJDIR_OPT)/GitReportModule.o: $(srcfiles)
$(OBJDIR_DBG)/GitReportModule.o: $(srcfiles)

#######################################################################
##   Documentation targets                                           ##
#######################################################################

# Generate documentation
docs:
	@echo "Generating documentation..."
	doxygen doxygen.conf

# Clean and regenerate documentation
docs-clean:
	@echo "Cleaning documentation directory..."
	rm -rf doc/*
	@echo "Generating fresh documentation..."
	doxygen doxygen.conf

# Clean documentation only
docs-clean-only:
	@echo "Cleaning documentation directory..."
	rm -rf doc/*

# Clean all including documentation
clean-all: docs-clean-only

# Phony targets
.PHONY: docs docs-clean docs-clean-only
