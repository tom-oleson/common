########################################################################
#
#       Makefile:       makefile
#       Description:    makefile for cmf lib cppunit tests  
#       Type:           makefile
#       Instance:       1
#       %derived_by:    g702176 %
#       %date_modified: Tue Aug 16 17:48:43 2005 %
#       %version:       8 %
#######################################################################
#######################################################################
#   Copyright(c) TSI Telecommunication Services Incorporated (TSI).
#   Unpublished Computer Software - All rights reserved under U.S. and
#   International copyright laws.  This Licensed Software contains
#   trade secrets of TSI and its Licensors.  Possession of the
#   Licensed Software does not convey any right to disclose, reproduce
#   or permit others to use this Licensed Software without specific
#   written authorization of TSI.
#######################################################################

include ../version.mk

TOP=$(PWD)

EXE = run_tests

OBJS =	utilTest.o \
	main.o

default: all

CC=g++
CM_LIB_DIR=../
INCLUDE = -I. -I$(CM_LIB_DIR)
LDFLAGS = -m64 -g -lcm_64 -ldl -lcppunit -lpthread -L$(CM_LIB_DIR) 
CCFLAGS = -m64 -g $(INCLUDE) -c -fPIC -D__LINUX_BOX__ -D_REENTRANT -D_LARGEFILE64_SOURCE

POSIXFLAGS = -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

%.o: %.cpp
	$(CC) $(CCFLAGS) $(POSIXFLAGS) $<

$(EXE): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXE)

clean:
	-@rm -rf *.o $(EXE) *.log cppunit_results.xml core.*
	@echo "$(EXE) $(@)ed"

all: prod

prod: $(EXE)
	clear
	$(PWD)/$(EXE)
