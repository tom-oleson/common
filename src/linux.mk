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
#   * The names of its contributors may NOT be used to endorse or promote
#     products derived from this software without specific prior written
#     permission.
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
#
# linux.mk - libcm make file


include ./version.mk

TOP=$(PWD)
PROD_DIR=~/common
WORD_SIZE=64

POSIX_FLAGS = -D_POSIX_PTHREAD_SEMANTICS
CM_LIB_VERSION=$(CM_VERSION)
DEBUG= -g
INCLUDE = -I. -I$(TOP)/ 
CCFLAGS = $(INCLUDE) -c -m$(WORD_SIZE) $(DEBUG) -fPIC -D__LINUX_BOX__ -DASSERT
LDFLAGS = -pthread -ldl -lssl $(DEBUG) 
CC=g++

CM_OBJS = OBJDIR_$(WORD_SIZE)/config.o \
	  OBJDIR_$(WORD_SIZE)/record.o \
	  OBJDIR_$(WORD_SIZE)/assert.o \
	OBJDIR_$(WORD_SIZE)/ssl.o \
    OBJDIR_$(WORD_SIZE)/cache.o \
    OBJDIR_$(WORD_SIZE)/network.o \
    OBJDIR_$(WORD_SIZE)/thread.o \
    OBJDIR_$(WORD_SIZE)/queue.o \
	OBJDIR_$(WORD_SIZE)/store.o \
	OBJDIR_$(WORD_SIZE)/util.o \
    OBJDIR_$(WORD_SIZE)/base64.o \
	OBJDIR_$(WORD_SIZE)/log.o \
	OBJDIR_$(WORD_SIZE)/process_scanner.o \
	OBJDIR_$(WORD_SIZE)/xml_reader.o \
	OBJDIR_$(WORD_SIZE)/timewatcher.o

CM_LIB = libcm_$(WORD_SIZE)

default: all

all: clean build install

build: libcm_$(WORD_SIZE).so libcm_$(WORD_SIZE).a

all_arch:	$(CM_LIB).so $(CM_LIB).a

clean:
	@find . -name '*.o' -print -exec rm -f {} \; >/dev/null
	@find . -name '$(CM_LIB)*' -print -exec rm -f {} \; >/dev/null
	@rm -rf OBJDIR_$(WORD_SIZE)
	@mkdir -p OBJDIR_$(WORD_SIZE)
	@echo "$(CM_LIB) $(@)ed" 

install:
	@mkdir -p $(PROD_DIR)/lib
	@mkdir -p $(PROD_DIR)/include
	@cp -f *.h $(PROD_DIR)/include
	@chmod 0644 $(PROD_DIR)/include/*.h
	@mv $(CM_LIB).so $(CM_LIB).so.$(CM_LIB_VERSION)
	@cp $(CM_LIB)* $(PROD_DIR)/lib
	@ln -s -f $(PROD_DIR)/lib/$(CM_LIB).so.$(CM_LIB_VERSION) $(PROD_DIR)/lib/$(CM_LIB).so
	@ln -s -f $(CM_LIB).so.$(CM_VERSION) $(CM_LIB).so
	@echo "$(CM_LIB) $(@)ed" 

	
OBJDIR_$(WORD_SIZE)/%.o: %.cpp
	$(CC) $(CCFLAGS) $(POSIX_FLAGS) -o $@ $<

$(CM_LIB).so: $(CM_OBJS)
	$(CC) -m$(WORD_SIZE) -fPIC $(LDFLAGS) -shared -o $@ $(CM_OBJS)

$(CM_LIB).a: $(CM_OBJS)
	ar rcs $@ $(CM_OBJS)
	
