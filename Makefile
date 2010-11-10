DEBUG = -g
OPTIMIZATION = -O2 -s -fomit-frame-pointer -DNDEBUG
WARN = -ansi -Wall
INCLUDE = -Ikernel/ -Iboost/
DEFS = -march=native 
CXXFLAGS = $(DEBUG) $(WARN) $(INCLUDE) $(DEFS)

CXX = g++ -pipe $(CXXFLAGS)

KERNEL = $(wildcard kernel/*.cc)
KERNEL_OBJ = $(KERNEL:.cc=.o)

TESTS =	test/test.cc \
        test/test1.cc test/test2.cc test/test3.cc test/test4.cc test/test5.cc \
	test/test6.cc test/test7.cc test/test8.cc test/test9.cc
SR    = sr/sr.cc
ALL   = $(KERNEL) $(SR) $(TESTS)

all:	sr test1 test2 test3 test4 test5 test6 test7 test8 test9

sr:	sr/sr.o lib/libboost_program_options.a $(KERNEL_OBJ)
	$(CXX) $? lib/libboost_program_options.a -o sr/$@

test:	test/test.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test1:	test/test1.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test2:	test/test2.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test3:	test/test3.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test4:	test/test4.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test5:	test/test5.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test6:	test/test6.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test7:	test/test7.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test8:	test/test8.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

test9:	test/test9.o $(KERNEL_OBJ)
	$(CXX) $? -o test/$@

%.o :	%.cc
	$(CXX) -c -MMD -o $@ $<
	@cp $*.d $*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
            rm -f $*.d

-include $(ALL:.cc=.P)

.phony:	clean
clean:
	@find ./ -name '*~' -exec rm '{}' \; -print -o -name ".*~" -exec rm {} \; -print -o -name "*.P" -exec rm {} \; -print -o -name "#*#" -exec rm {} \; -print
	@rm -f sr/sr test/test? test/test kernel/*.o sr/*.o test/*.o

.phony:	backup
backup:
	@make clean
	@tar --exclude="boost" --exclude="backup" --xz -cvf ../backup/vita.tar.xz ../../vita