# Further details / ideas:
# * http://gpwiki.org/index.php/Makefile
# * http://mad-scientist.net/make/autodep.html 



# Specify the main target.
TARGET = sr

# Build type: debug, profile, release.
TYPE = release

#-------------------------------------------------------------------------

# Which libraries are linked
LIB = lib/libboost_program_options.a

# Add directories to the include path.
INCPATH = ./kernel ./boost

WARN = -pedantic -ansi -Wall -Wextra
DEFS = -march=native

# The next blocks change some variables depending on the build type.
ifeq ($(TYPE),debug)
  TYPE_PARAM = -g
endif

ifeq ($(TYPE),profile)
  TYPE_PARAM = -Wall -pg $(WARN) $(DEFS) -DNDEBUG
endif

ifeq ($(TYPE), release)
  TYPE_PARAM = -s -O2 -fomit-frame-pointer -DNDEBUG
endif

CXXFLAGS = $(TYPE_PARAM) $(WARN) $(DEFS)

CXX = g++ -pipe $(CXXFLAGS)

KERNEL_SRC = $(wildcard kernel/*.cc)
KERNEL_OBJ = $(KERNEL_SRC:.cc=.o)

ifeq ($(TARGET),sr)
  ifeq ($(strip $(OUTDIR)),)
    OUTDIR = sr
  endif
  MAIN_SRC = sr/sr.cc
else
  ifeq ($(strip $(OUTDIR)),)
    OUTDIR = test
  endif
  MAIN_SRC = test/$(TARGET).cc
endif

MAIN_OBJ = $(MAIN_SRC:.cc=.o)

ALL_SRC = $(KERNEL_SRC) $(MAIN_SRC)

$(TARGET): $(MAIN_OBJ) $(KERNEL_OBJ)
	@echo Linking $(TARGET)...
	$(CXX) $(MAIN_OBJ) $(KERNEL_OBJ) -o $(OUTDIR)/$(TARGET) $(LIB)

%.o : %.cc Makefile
	@echo Creating object file for $*...
	$(CXX) $(foreach INC,$(INCPATH),-I$(INC)) -MMD -o $@ -c $<
	@cp $*.d $*.P; sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; rm -f $*.d
	@echo ; echo

-include $(ALL_SRC:.cc=.P)

.phony:	clean
clean:
	@echo Making clean...
	@find ./ -name '*~' -exec rm '{}' \; -print -o -name ".*~" -exec rm {} \; -print -o -name "*.P" -exec rm {} \; -print -o -name "#*#" -exec rm {} \; -print
	@rm -f sr/sr test/test? test/test kernel/*.o sr/*.o test/*.o

.phony:	backup
backup:
	@echo Making backup...
	@-if [ ! -e ../backup ]; then mkdir ../backup; fi;
	@make clean
	@tar --exclude="boost" --exclude="backup" --exclude="doxygen/latex" --exclude="doxygen/html" --exclude=".make-debug" --exclude=".make-profile" --exclude=".make-release" --xz -cvf ../backup/vita_`date +%y-%m-%d_%H.%M`.tar.xz ../../vita
