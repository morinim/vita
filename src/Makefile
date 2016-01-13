# Build type: debug, profile, release
TYPE := release

# Compiler: clang++, g++
CXX := g++
#CLANG := $(shell command -v clang++ >/dev/null; echo $$?)
#ifeq ($(CLANG), 0)
#  CXX := clang++
#else
#  CXX := g++
#endif

# -DCLONE_SCALING
#
# -DMUTUAL_IMPROVEMENT
#
# -DUNIFORM_CROSSOVER / -DONE_POINT_CROSSOVER / -DTREE_CROSSOVER /
# -DTWO_POINT_CROSSOVER (default)
#
# -DUSE_BOOST_ANY
DEFS :=



# NO USER SERVICEABLE PARTS BELOW THIS LINE
# -------------------------------------------------------------------------
$(info Building: $(TYPE))
$(info Using: $(CXX))

#VERSION := $(shell hg log -r . --template '{latesttag}-{latesttagdistance}-{node|short}')

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
INCPATH := ../src/ ./
SYSTEMINCPATH := $(BOOST_INCLUDE) ./third_party/

# -Wsign-conversion -Weffc++ are other interesting warning
# switches to try from time to time (they gives many false positives).
WARN_FLAGS := -Wpedantic -Wall -Wextra -Winvalid-pch -Wpedantic -Wformat=2 -Wfloat-equal -Wshadow -Wconversion
ifeq ($(CXX), g++)
  WARN_FLAGS += -Wdouble-promotion -Wzero-as-null-pointer-constant
endif

OPTIMIZE_FLAGS := -O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS
# Link time optimization has some issues with MinGW
ifeq ($(CXX), g++)
  ifneq ($(OS), Windows_NT)
    OPTIMIZE_FLAGS += -flto
  endif
endif

# The next blocks change some variables depending on the build type.
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

CXXFLAGS := -pipe -march=native --std=c++11 $(TYPE_FLAGS) $(WARN_FLAGS) $(DEFS)
LDFLAGS := $(CXXFLAGS)

COMPILE := $(CXX) $(CXXFLAGS)

KERNEL_SRC := $(wildcard kernel/*.cc) $(wildcard kernel/src/*.cc) $(wildcard kernel/src/primitive/*.cc) $(wildcard kernel/ga/*.cc) third_party/tinyxml2/tinyxml2.cc
KERNEL_OBJ := $(KERNEL_SRC:.cc=.o)
EXAMPLES_SRC := $(wildcard examples/*.cc)
SR_SRC := $(wildcard sr/*.cc)
TESTS_SRC := $(wildcard test/*.cc)

ALL_SRC := $(KERNEL_SRC) $(EXAMPLES_SRC) $(SR_SRC) $(TESTS_SRC)

.PRECIOUS: %.o

all: kernel sr

sr: sr/sr.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o sr/$@ $(LIB)
ifeq ($(TYPE), release)
ifdef MSYSTEM
	@strip sr/$@.exe
else
	@strip sr/$@
endif
endif

examples: $(EXAMPLES_SRC:.cc=)
	@echo $(notdir $^) successfully built

examples/%: examples/%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o $@

tests: test/tests.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o test/$@ $(DEBUG_LIB)
	@cd test; ./$@ --show_progress --build_info

test/%: test/%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o $@ $(DEBUG_LIB)
	@cd test; ./$(@F) --show_progress --build_info

kernel: $(KERNEL_OBJ)
	@echo Linking libvita.a
	@ar rcs kernel/libvita.a $(KERNEL_OBJ)
ifeq ($(TYPE), release)
	@strip kernel/libvita.a
endif

# Disable all warnings for TinyXML2
third_party/tinyxml2/tinyxml2.o: COMPILE += -w

%.o : %.cc Makefile
	@echo Creating object file for $*...
	@$(COMPILE) $(foreach INC,$(INCPATH),-I$(INC)) $(foreach INC,$(SYSTEMINCPATH),-isystem$(INC)) -MMD -o $@ -c $<
	@cp $*.d $*.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; $(RM) $*.d

-include $(ALL_SRC:.cc=.P)

.phony:	check
check: check_cppcheck check_cpplint

.phony:	check_cppcheck
check_cppcheck:
	@command -v ./tools/cppcheck/cppcheck >/dev/null && ./tools/cppcheck/cppcheck --enable=all --std=c++11 kernel/ 2> cppcheck.txt || { echo >&2 "cppcheck not installed."; }

.phony:	check_cpplint
check_cpplint:
	@command -v python >/dev/null || { echo >&2 "Python not installed."; } && ./tools/cpplint.py --filter=-whitespace/braces,-whitespace/newline,-build/header_guard,-readability/streams kernel/*.cc 2> cpplint.txt

.phony:	clean
clean:
	@echo Making clean...
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "*~" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name ".*~" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ ./third_party/ -name "*.P" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "#*#" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ ./third_party/ -name "*.o" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "*.gch" -type f -delete -print
	@find ./test/ ./examples/ -executable -not -name "*.*" -type f -delete -print
	@find ./test/ ./examples/ -executable -name "*.exe" -type f -delete -print
	@$(RM) sr/sr kernel/libvita.a cppcheck.txt cpplint.txt

.phony:	backup
backup:
	@echo Making backup...
	@-if [ ! -e ../backup ]; then mkdir ../backup; fi;
	@make clean
	@tar --exclude="boost" --exclude="backup" --exclude="doxygen/latex" --exclude="doxygen/html" --xz -cvf ../backup/vita_`date +%y-%m-%d_%H.%M`.txz ../../vita


# Further details / ideas:
# * http://gpwiki.org/index.php/Makefile
# * http://mad-scientist.net/make/autodep.html
# * http://stackoverflow.com/questions/5127977/makefile-define-compilation-variables-based-on-target-for
# * http://stackoverflow.com/questions/448910/makefile-variable-assignment
# * http://stackoverflow.com/questions/592620/how-to-check-if-a-program-exists-from-a-bash-script
