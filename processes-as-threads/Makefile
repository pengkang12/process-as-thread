SOURCE_DIR = .
INCLUDE_DIR = .

#SOURCE_DIR = source
#INCLUDE_DIR = include

#SRCS_IBS= $(SOURCE_DIR)/IBS/ibsop.c  
SRCS_IBS= 
SRCS =  $(SOURCE_DIR)/libmem.cpp \
	$(SOURCE_DIR)/libfuncs.cpp         \
	$(SRCS_IBS) \
	$(SOURCE_DIR)/libmythread.cpp \
	$(SOURCE_DIR)/mythread.cpp \
	$(SOURCE_DIR)/xmemory.cpp \
	$(SOURCE_DIR)/myrun.cpp \
	$(SOURCE_DIR)/map_parse/pmparser.c

INCS =  $(INCLUDE_DIR)/xdefines.h     \
	$(INCLUDE_DIR)/xmemory.h         \
	$(INCLUDE_DIR)/objectheader.h \
	$(INCLUDE_DIR)/xoneheap.h     \
	$(INCLUDE_DIR)/xpheap.h \
	$(INCLUDE_DIR)/privateheap.h \
	$(INCLUDE_DIR)/mm.h \
	$(INCLUDE_DIR)/libmythread.h \
 	$(INCLUDE_DIR)/mythread.h \
 	$(INCLUDE_DIR)/debug.h \
	$(INCLUDE_DIR)/myrun.h \
	$(INCLUDE_DIR)/mydeterm.h \
	$(INCLUDE_DIR)/map_parse/pmparser.h \
	$(INCLUDE_DIR)/myglobals.h 


INCLUDE_DIRS = -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/heaplayers -I$(INCLUDE_DIR)/heaplayers/util
DEPS = $(SRCS) $(INCS)

CXX = g++ -g -O0 -DEBUG
# O2
# Detection on 32bit
# CXX = g++ -DSSE_SUPPORT -m32 -DX86_32BIT -O3 -fno-omit-frame-pointer -DDETECT_FALSE_SHARING
# Detection on 64bit
#CXX = g++ -DSSE_SUPPORT -m64 -fno-omit-frame-pointer -DDETECT_FALSE_SHARING


# -march=core2 -msse3 -DSSE_SUPPORT 
#CFLAGS   = -Wall -msse3 -DSSE_SUPPORT -fno-omit-frame-pointer
#CFLAGS   = -fno-omit-frame-pointer -DPREDICTION
CFLAGS   = -Wno-unused-result -fno-omit-frame-pointer -DPREDICTION -DUSING_SIGUSR2
#CFLAGS   = -Wno-unused-result -fno-omit-frame-pointer -DUSING_IBS -DPREDICTION -DUSING_SIGUSR2
#-DOUTPUT_WORD_ACCESSES
#CFLAGS   = -fno-omit-frame-pointer -DPREDICTION -DOUTPUT_WORD_ACCESSES
#-DIMPROVE_PERF 
#-DOUTPUT_WORD_ACCESSES
#CFLAGS   = -fno-omit-frame-pointer 
CFLAGS32 = $(CFLAGS) -m32 -DX86_32BIT # -O3
CFLAGS64 = $(CFLAGS) #-m64 # -O3
#CFLAGS64 = $(CFLAGS) -m64 # -O3

#GET_CHARACTERISTICS

TARGETS = libdthread.so

all: $(TARGETS)

libdthread.so: $(DEPS)
	$(CXX) $(CFLAGS64) $(INCLUDE_DIRS) -shared -fPIC -D'CUSTOM_PREFIX(x)=mem_##x' $(SRCS) -o $@ -ldl -lpthread 

clean:
	rm -f $(TARGETS)

