## specific source file
IP_INFO         := HV7607B_GammaVrr
INCLUDE_PATH    := ./
SRC_PATH        := ./


CC      = gcc  
CPP     = g++  
RM      = rm -rf  
UNAME_S := $(shell uname -s)
  
## CCOPT    =-O0 -g -c -fPIC
## APPCCOPT =-O0 -g   
## debug flag  
## DBG_ENABLE   = 1  

## target exec file name  
TARGET_EXE	:= $(IP_INFO)
CFLAGS		= -O3 -fPIC
ifeq ($(UNAME_S),Darwin)
    TARGET_SO := $(IP_INFO).dylib
    SHARED_LDFLAGS := -dynamiclib
else
    TARGET_SO := $(IP_INFO).so
    SHARED_LDFLAGS := -shared
    LDFLAGS += -ldl
endif
  
## get all source files  
SRCS         += $(wildcard $(SRC_PATH)/*.cpp)
## $(info SRCS = $(SRCS))

## debug for debug info, when use gdb to debug  
ifeq (1, ${DBG_ENABLE})   
    CFLAGS += -D_DEBUG -O0 -g -DDEBUG=1  
endif  
  
## get all include path  
CFLAGS  += $(foreach dir, $(INCLUDE_PATH), -I$(dir))  
  
## get all library path  
LDFLAGS += $(foreach lib, $(LIBRARY_PATH), -L$(lib))  
  
## get all librarys  
LDFLAGS += $(foreach lib, $(LIBS), -l$(lib))  
  
  
all: clean shared build  
  

build:  
#	$(CPP) -c $(CFLAGS) $(SRCS)  
	$(CPP) $(CFLAGS) -o $(TARGET_EXE) $(SRCS) $(LDFLAGS)  
	## $(RM) *.o  
	
shared:
	$(CPP) $(SHARED_LDFLAGS) $(CFLAGS) -o $(TARGET_SO) $(SRCS) $(LDFLAGS)
	
clean:  
	$(RM) *.o  $(TARGET_EXE) $(IP_INFO).so $(IP_INFO).dylib
