########################################################################
#
#	Filename:       linux.mk
#	Description:    Makefile for libcm on linux
#	Type:			makefile
#
#######################################################################

include ./version.mk

TOP=$(PWD)
WORD_SIZE=64

POSIX_FLAGS = -D_POSIX_PTHREAD_SEMANTICS
CM_LIB_VERSION=$(CM_VERSION)
DEBUG= -g3
INCLUDE = -I. -I$(TOP)/ 
CCFLAGS = $(INCLUDE) -c -m$(WORD_SIZE) $(DEBUG) -fPIC -D__LINUX_BOX__
LDFLAGS = -lpthread -lrt $(DEBUG)
CC=g++

CM_OBJS = OBJDIR_$(WORD_SIZE)/util.o

CM_LIB = libcm_$(WORD_SIZE)

default: all

all: clean build

build: libcm_$(WORD_SIZE).so

all_arch:	$(CM_LIB).so $(CM_LIB).a

clean:
	find . -name '*.o' -print -exec rm -f {} \; >/dev/null
	find . -name '$(CM_LIB)*' -print -exec rm -f {} \; >/dev/null
	rm -rf OBJDIR_$(WORD_SIZE)
	mkdir -p OBJDIR_$(WORD_SIZE)
	@echo "$(CM_LIB) $(@)ed" 

	
OBJDIR_$(WORD_SIZE)/%.o: %.cpp
	$(CC) $(CCFLAGS) $(POSIX_FLAGS) -o $@ $<

$(CM_LIB).so: $(CM_OBJS)
	$(CC) -m$(WORD_SIZE) -fPIC $(LDFLAGS) -shared -o $@ $(CM_OBJS)
