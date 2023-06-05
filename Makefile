# the compiler: gcc for C program, g++ for C++
CC = g++

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
CFLAGS = -g -Wall -O3 -fopenmp

# define any directories containing header files other than /usr/include
#
INCLUDES = -I./include

# Define source folder
#
SOURCE = src/

# Define build folder
#
BUILD = build/

# define the executable file 
#
MAIN = $(BUILD)wikiracer.exe

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#
# LFLAGS = -L../lib 

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
#
LIBS = -lcurl

# define the C++ source files
#
SRCS = $(SOURCE)main.cc $(SOURCE)WikiRacer.cc $(SOURCE)UrlRequest.cc

# define the C++ object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cc=.o)


#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#
.PHONY: depend clean

all: $(MAIN)

$(BUILD):
	mkdir $(BUILD)

$(MAIN): $(OBJS) $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)
	@echo "Finished building $(MAIN)"

# this is a suffix replacement rule for building .o's from .cc's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .cc file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) $(OBJS) *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it