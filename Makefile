# Further details / ideas:
# * http://gpwiki.org/index.php/Makefile
# * http://mad-scientist.net/make/autodep.html



# Build type: debug, profile, release.
TYPE = release

# Boost library <-- PLEASE CHECK THE PATHS!
BOOST_INCLUDE = ./boost
BOOST_LIB = $(BOOST_INCLUDE)/stage/lib

# Compiler (clang++, g++)
CXX = clang++

# -DCLONE_SCALING
# -DMUTUAL_IMPROVEMENT
# -DVITA_NO_LIB
# -DUNIFORM_CROSSOVER / -DONE_POINT_CROSSOVER / -DTWO_POINT_CROSSOVER (default)
DEFS =



# NO USER SERVICEABLE PARTS BELOW THIS LINE
# -------------------------------------------------------------------------

# Which libraries are linked
LIB = $(BOOST_LIB)/libboost_program_options.a
DEBUG_LIB = $(BOOST_LIB)/libboost_unit_test_framework.a

# Add directories to the include path.
# This should work in POSIX compliant environment (see "The Open Group Base
# Specifications Issue 7" and
# http://pubs.opengroup.org/onlinepubs/9699919799/utilities/c99.html)
INCPATH = ../vita/ ./
SYSTEMINCPATH = $(BOOST_INCLUDE)

WARN = -pedantic --std=c++11 -Wall -Wextra -Winvalid-pch

# The next blocks change some variables depending on the build type.
ifeq ($(TYPE), debug)
  TYPE_PARAM = -g
endif

ifeq ($(TYPE), profile)
  TYPE_PARAM = -pg -O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS
endif

ifeq ($(TYPE), release)
  TYPE_PARAM += -O3 -fomit-frame-pointer -DNDEBUG -DBOOST_DISABLE_ASSERTS
endif

CXXFLAGS = -pipe -march=native $(TYPE_PARAM) $(WARN) $(DEFS)
COMPILE = $(CXX) $(CXXFLAGS)

KERNEL_SRC = $(wildcard kernel/*.cc) $(wildcard kernel/src/*.cc) $(wildcard kernel/src/primitive/*.cc)
KERNEL_OBJ = $(KERNEL_SRC:.cc=.o)
EXAMPLES_SRC = $(wildcard examples/*.cc)
SR_SRC = $(wildcard sr/*.cc)
TESTS_SRC = $(wildcard test/*.cc)

ALL_SRC = $(KERNEL_SRC) $(EXAMPLES_SRC) $(SR_SRC) $(TESTS_SRC)

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

examples: example1 example2 example3 example4 example5 example6 example7 example8 example9a

example%: examples/example%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o examples/$@

titanic%: examples/titanic%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o examples/$@

tests: test/tests.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o test/$@ $(DEBUG_LIB)
	@test/$@ --show_progress

test_%: test/test_%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(COMPILE) $< $(KERNEL_OBJ) -o test/$@ $(DEBUG_LIB)
	@test/$@ --show_progress

kernel: $(KERNEL_OBJ)
	@echo Linking libvita.a
	@ar rcs kernel/libvita.a $(KERNEL_OBJ)
ifeq ($(TYPE), release)
	@strip kernel/libvita.a
endif

%.o : %.cc Makefile
	@echo Creating object file for $*...
	@$(COMPILE) $(foreach INC,$(INCPATH),-I$(INC)) $(foreach INC,$(SYSTEMINCPATH),-isystem$(INC)) -MMD -o $@ -c $<
	@cp $*.d $*.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; rm -f $*.d

-include $(ALL_SRC:.cc=.P)

.phony:	clean
clean:
	@echo Making clean...
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "*~" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name ".*~" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "*.P" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "#*#" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "*.o" -type f -delete -print
	@find ./kernel/ ./examples/ ./sr/ ./test/ -name "*.gch" -type f -delete -print
	@find ./test/ ./examples/ -executable -not -name "*.*" -type f -delete -print
	@find ./test/ ./examples/ -executable -name "*.exe" -type f -delete -print
	@rm -f sr/sr kernel/libvita.a

.phony:	backup
backup:
	@echo Making backup...
	@-if [ ! -e ../backup ]; then mkdir ../backup; fi;
	@make clean
	@tar --exclude="boost" --exclude="backup" --exclude="doxygen/latex" --exclude="doxygen/html" --xz -cvf ../backup/vita_`date +%y-%m-%d_%H.%M`.txz ../../vita
