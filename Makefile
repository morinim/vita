# Further details / ideas:
# * http://gpwiki.org/index.php/Makefile
# * http://mad-scientist.net/make/autodep.html



# Build type: debug, profile, release.
TYPE = release

# Boost library <-- PLEASE CHECK THE PATH!
BOOST_INCLUDE = ./boost
BOOST_LIB = $(BOOST_INCLUDE)/stage/lib



# NO USER SERVICEABLE PARTS BELOW THIS LINE
# -------------------------------------------------------------------------

# Which libraries are linked
LIB = $(BOOST_LIB)/libboost_program_options.a
DEBUG_LIB = $(BOOST_LIB)/libboost_unit_test_framework.a

# Add directories to the include path.
INCPATH = ./ $(BOOST_INCLUDE)

WARN = -pedantic --std=c++0x -Wall -Wextra
DEFS = -march=native

# The next blocks change some variables depending on the build type.
ifeq ($(TYPE), debug)
  TYPE_PARAM = -g
endif

ifeq ($(TYPE), profile)
  TYPE_PARAM = -Wall -pg $(WARN) $(DEFS) -DNDEBUG
endif

ifeq ($(TYPE), release)
  TYPE_PARAM = -s -O3 -fomit-frame-pointer -DNDEBUG
endif

CXXFLAGS = $(TYPE_PARAM) $(WARN) $(DEFS)

CXX = g++ -pipe $(CXXFLAGS)

KERNEL_SRC = $(wildcard kernel/*.cc)
KERNEL_OBJ = $(KERNEL_SRC:.cc=.o)
EXAMPLES_SRC = $(wildcard examples/*.cc)
SR_SRC = $(wildcard sr/*.cc)
TESTS_SRC = $(wildcard test/*.cc)

ALL_SRC = $(KERNEL_SRC) $(EXAMPLES_SRC) $(SR_SRC) $(TESTS_SRC)

.PRECIOUS: %.o

all: kernel sr

sr: sr/sr.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(CXX) $< $(KERNEL_OBJ) -o sr/$@ $(LIB)

examples: example1 example2 example3 example4 example5 example6 example7 example8

example%: examples/example%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(CXX) $< $(KERNEL_OBJ) -o examples/$@

tests: test_evolution test_individual test_primitive test_ttable

test%: test/test%.o $(KERNEL_OBJ)
	@echo Linking $@
	@$(CXX) $< $(KERNEL_OBJ) -o test/$@ $(DEBUG_LIB)
	@test/$@ --show_progress

kernel: $(KERNEL_OBJ)
	@echo Linking libvita.a
	@ar rcs kernel/libvita.a $(KERNEL_OBJ)

%.o : %.cc Makefile
	@echo Creating object file for $*...
	@$(CXX) $(foreach INC,$(INCPATH),-I$(INC)) -MMD -o $@ -c $<
	@cp $*.d $*.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; rm -f $*.d

-include $(ALL_SRC:.cc=.P)

.phony:	clean
clean:
	@echo Making clean...
	@find ./ -name '*~' -exec rm '{}' \; -print -o -name ".*~" -exec rm {} \; -print -o -name "*.P" -exec rm {} \; -print -o -name "#*#" -exec rm {} \; -print
	@rm -f sr/sr test/test? examples/example? kernel/*.o sr/*.o test/*.o examples/*.o kernel/libvita.a

.phony:	backup
backup:
	@echo Making backup...
	@-if [ ! -e ../backup ]; then mkdir ../backup; fi;
	@make clean
	@tar --exclude="boost" --exclude="backup" --exclude="doxygen/latex" --exclude="doxygen/html" --exclude=".make-debug" --exclude=".make-profile" --exclude=".make-release" --xz -cvf ../backup/vita_`date +%y-%m-%d_%H.%M`.tar.xz ../../vita
