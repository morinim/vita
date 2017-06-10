# Build type: debug, profile, release
TYPE := release

# Compiler: clang++, g++
# CXX := g++

# -DCLONE_SCALING
#
# -DMUTUAL_IMPROVEMENT
#
# -DUNIFORM_CROSSOVER / -DONE_POINT_CROSSOVER / -DTREE_CROSSOVER /
# -DTWO_POINT_CROSSOVER (default)
#
# -DUSE_BOOST_ANY
#
# -DVITA_CONCEPTS_THROW
DEFS :=



# NO USER SERVICEABLE PARTS BELOW THIS LINE
# -------------------------------------------------------------------------
$(info Building: $(TYPE))
$(info Using: $(CXX))

# Boost library
LIB := -static -lboost_program_options
DEBUG_LIB := -static -lboost_unit_test_framework

ifneq ($(wildcard ./boost/.),)
  $(info Boost directory: custom)
  BOOST_INCLUDE := ./boost
  BOOST_LIB_PATH := $(BOOST_INCLUDE)/stage/lib
  LIB := -L$(BOOST_LIB_PATH) $(LIB)
  DEBUG_LIB := -L$(BOOST_LIB_PATH) $(DEBUG_LIB)
  #LIB := -l:$(BOOST_LIB_PATH)/libboost_program_options.a
  #DEBUG_LIB := -l:$(BOOST_LIB_PATH)/libboost_unit_test_framework.a
endif

# Add directories to the include path.
# This should work in POSIX compliant environment (see "The Open Group Base
# Specifications Issue 7" and
# <http://pubs.opengroup.org/onlinepubs/9699919799/utilities/c99.html>)
INCPATH := ../../src/ ../src/ ./
SYSTEMINCPATH := $(BOOST_INCLUDE) ./third_party/

# -Wconversion -Wsign-conversion -Weffc++ are other interesting warning
# switches to try from time to time (they gives many false positives).
WARN_FLAGS := -Wpedantic -Wall -Wextra -Winvalid-pch -Wpedantic -Wformat=2 -Wfloat-equal -Wshadow -Wdouble-promotion
ifeq ($(CXX), g++)
  WARN_FLAGS += -Wzero-as-null-pointer-constant
endif

OPTIMIZE_FLAGS := -O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS

# Build type specific flags.
ifeq ($(TYPE), debug)
TYPE_FLAGS := -g

ifeq ($(CXX), g++)
  TYPE_FLAGS += -Og
endif
endif

ifeq ($(TYPE), profile)
  TYPE_FLAGS := -pg $(OPTIMIZE_FLAGS)
endif

ifeq ($(TYPE), release)
  TYPE_FLAGS := $(OPTIMIZE_FLAGS)
endif

CXXFLAGS := -pipe -march=native --std=c++14 $(TYPE_FLAGS) $(WARN_FLAGS) $(DEFS)
LDFLAGS := $(CXXFLAGS)

COMPILE := $(CXX) $(CXXFLAGS)

EXAMPLES_SRC := $(wildcard examples/*.cc)
FOREX_SRC := $(wildcard examples/forex/*.cc)
KERNEL_SRC := $(wildcard kernel/*.cc) $(wildcard kernel/src/*.cc) $(wildcard kernel/src/primitive/*.cc) $(wildcard kernel/ga/*.cc)
SR_SRC := $(wildcard sr/*.cc)
TESTS_SRC := $(wildcard test/*.cc)
THIRD_PARTY_SRC := third_party/tinyxml2/tinyxml2.cc
UTILITY_SRC := $(wildcard utility/*.cc)
FRAMEWORK_SRC := $(KERNEL_SRC) $(THIRD_PARTY_SRC) $(UTILITY_SRC)

FRAMEWORK_OBJ := $(FRAMEWORK_SRC:.cc=.o)
FOREX_OBJ := $(FOREX_SRC:.cc=.o)
THIRD_PARTY_OBJ := $(THIRD_PARTY_SRC:.cc=.o)

ALL_SRC := $(FRAMEWORK_SRC) $(EXAMPLES_SRC) $(FOREX_SRC) $(SR_SRC) $(TESTS_SRC)

MKDIR := mkdir -p
OUTDIR := build

.PRECIOUS: %.o

all: directories vita sr

forex: directories $(FOREX_OBJ) $(FRAMEWORK_OBJ)
	@echo Linking $@
	@$(COMPILE) $(FOREX_OBJ) $(FRAMEWORK_OBJ) -o $(OUTDIR)/$@ $(LIB)
ifeq ($(TYPE), release)
ifdef MSYSTEM
	@strip $(OUTDIR)/$@.exe
else
	@strip $(OUTDIR)/$@
endif
endif

sr: sr/sr.o $(FRAMEWORK_OBJ) directories
	@echo Linking $@
	@$(COMPILE) $< $(FRAMEWORK_OBJ) -o $(OUTDIR)/$@ $(LIB)
ifeq ($(TYPE), release)
ifdef MSYSTEM
	@strip $(OUTDIR)/$@.exe
else
	@strip $(OUTDIR)/$@
endif
endif

examples: $(EXAMPLES_SRC:.cc=) forex directories
	@echo $(notdir $^) successfully built

examples/%: examples/%.o $(FRAMEWORK_OBJ) directories
	@echo Linking $@
	@$(COMPILE) $< $(FRAMEWORK_OBJ) -o $(OUTDIR)/$(@F)

tests: test/tests.o $(FRAMEWORK_OBJ) directories
	@echo Linking $@
	@$(COMPILE) $< $(FRAMEWORK_OBJ) -o $(OUTDIR)/$@ $(DEBUG_LIB)
	@cd $(OUTDIR) && ./$@ --show_progress --build_info

test/%: test/%.o $(FRAMEWORK_OBJ) directories
	@echo Linking $@
	@$(COMPILE) $< $(FRAMEWORK_OBJ) -o $(OUTDIR)/$(@F) $(DEBUG_LIB)
	@cd $(OUTDIR) && ./$(@F) --show_progress --build_info

.phony: directories
directories: $(OUTDIR)

$(OUTDIR):
	@$(MKDIR) $(OUTDIR)

# For HG the version ID is:
# `hg log -r . --template '{latesttag}-{latesttagdistance}-{node|short}`
.phony:	version
version:
	@echo Updating version
	@echo -n '#define VITA_COMMIT_VERSION "' > kernel/version.h
	@echo -n git describe --long >> kernel/version.h
	@echo '"' >> kernel/version.h

# In case of errors try switching to gcc-ar.
# See <https://stackoverflow.com/a/25878408/3235496>
vita: $(FRAMEWORK_OBJ)
	@echo Linking libvita.a
	@ar rcs kernel/libvita.a $(FRAMEWORK_OBJ)
ifeq ($(TYPE), release)
	@strip kernel/libvita.a
endif

# Disable all warnings for third_party libraries.
$(THIRD_PARTY_OBJ): COMPILE += -w

%.o : %.cc Makefile
	@echo Creating object file for $*...
	@$(COMPILE) $(foreach INC,$(INCPATH),-I$(INC)) $(foreach INC,$(SYSTEMINCPATH),-isystem$(INC)) -MMD -o $@ -c $<
	@cp $*.d $*.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; $(RM) $*.d

-include $(ALL_SRC:.cc=.P)

.phony:	check
check: check_cppcheck check_cpplint

.phony:	check_cppcheck
check_cppcheck:
	@command -v ../tools/cppcheck/cppcheck > /dev/null && ../tools/cppcheck/cppcheck --enable=all --std=c++11 kernel/ || { echo >&2 "Error: cppcheck not found."; }

# Without `|| true` make cplains when cpplint found errors.
.phony:	check_cpplint
check_cpplint:
	@../tools/cpplint.py --filter=-whitespace/braces,-whitespace/newline,-build/header_guard,-build/include_what_you_use,-readability/streams $(FRAMEWORK_SRC) || true

.phony:	clean
clean:
	@echo Making clean...
	@find ./ ./kernel/ ./examples/ ./sr/ ./test/ ./utility/ -name "*~" -type f -delete -print
	@find ./ ./kernel/ ./examples/ ./sr/ ./test/ ./utility/ -name ".*~" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ ./third_party/ ./utility/ -name "*.P" -type f -delete -print
	@find ./ ./kernel/ ./examples/ ./sr/ ./test/ ./utility/ -name "#*#" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ ./third_party/ ./utility/ -name "*.o" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ ./utility/ -name "*.gch" -type f -delete -print
	@$(RM) -rf $(OUTDIR)
	@$(RM) sr/sr kernel/libvita.a examples/forex/forex cppcheck.txt cpplint.txt

.phony:	backup
backup:
	@echo Making backup...
	@-if [ ! -e ../backup ]; then mkdir ../backup; fi;
	@make clean
	@tar --exclude="boost" --exclude="backup" --exclude="doxygen/latex" --exclude="doxygen/html" --xz -cvf ../backup/vita_`date +%y-%m-%d_%H.%M`.txz ../../vita


# Further details / ideas:
# * http://gpwiki.org/index.php/Makefile
# * http://mad-scientist.net/make/autodep.html
# * Target-specific variable values: http://stackoverflow.com/q/5127977/3235496
# * Variable assignment: http://stackoverflow.com/q/448910/3235496
# * Check if a program exists: http://stackoverflow.com/q/592620/3235496
