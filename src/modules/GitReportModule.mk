#######################################################################
##   GitReportModule build configuration                             ##
##   Author: Til Gärtner                                             ##
##   Purpose: Add current Git information to the build               ##
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
