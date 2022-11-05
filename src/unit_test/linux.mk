#
# Copyright (C)2019, Tom Oleson <tom dot oleson at gmail dot com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * Neither the name of Redis nor the names of its contributors may be used
#     to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# libcm cppunit unit tests make file
#

include ../version.mk

TOP=$(PWD)

EXE = run_tests

OBJS =	utilTest.o \
    recordTest.o \
	assertTest.o \
	listTest.o \
	hashTest.o \
    bufferTest.o \
    cacheTest.o \
    threadTest.o \
    networkTest.o \
    queueTest.o \
    storeTest.o \
    configTest.o \
    timewatcherTest.o \
    logTest.o \
    process_scannerTest.o \
	xml_readerTest.o \
    main.o

default: all

#note: you may need to point to libxml headers with something like this
#if you get libxml directory not found.
#$ sudo ln -s /usr/include/libxml2/libxml/ /usr/local/include/libxml

CC=g++
CM_LIB_DIR=../
INCLUDE = -I. -I$(CM_LIB_DIR) -I/usr/include/libxml2
LDFLAGS = -m64 -g -lcm_64 -ldl -lcppunit -pthread -lssl -lcrypto -lxml2 -L$(CM_LIB_DIR)
CCFLAGS = -m64 -g $(INCLUDE) -c -fPIC -D__LINUX_BOX__ -D_REENTRANT -D_LARGEFILE64_SOURCE -DASSERT

POSIXFLAGS = -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT

%.o: %.cpp
	$(CC) $(CCFLAGS) $(POSIXFLAGS) $<

$(EXE): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(EXE)

clean:
	-@rm -rf *.o $(EXE) ./log/* cppunit_results.xml core.*
	@echo "$(EXE) $(@)ed"

build: $(EXE)


all: clean build

run:
	clear
	export LD_LIBRARY_PATH=$(CM_LIB_DIR):$(LD_LIBRARY_PATH);$(PWD)/$(EXE)


# workaround to get LD_LIBRARY_PATH into vscode environment
vscode:
	clear
	export LD_LIBRARY_PATH=$(CM_LIB_DIR):$(LD_LIBRARY_PATH);code

