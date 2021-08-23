#############################################################################
# Makefile for building PathPlannerDemo
# Command: make planner-demo
#############################################################################

CXX = g++
CXXFLAGS = -std=c++2a -fPIC -Wno-unknown-pragmas #-fstack-protector-all
LIBS = -lX11 -lGL -lpthread -lpng -lstdc++fs
# WARN_ON = -Wall -Wextra -Wconversion
# WARN_OFF = -Wno-narrowing
# 
# RELEASE_FLAGS = -Ofast

# ifeq ($(strip $(WARNINGS)),YES)
# 	CXXFLAGS += $(WARN_ON)
# else
# 	CXXFLAGS += $(WARN_OFF) 
# endif
# 
# ifeq ($(strip $(DEBUG_LEVEL)),1)
# 	CCFLAGS += -g -O3 -D_NVTX 
# 	CXXFLAGS += -g -O3 -D_NVTX
# else 
# ifeq ($(strip $(DEBUG_LEVEL)),2)
# 	CCFLAGS += -g -O0 -D_NVTX
# 	CXXFLAGS += -g -O0 -D_NVTX
# else
# 	CCFLAGS += $(RELEASE_FLAGS) -D_NVTX
# 	CXXFLAGS += $(RELEASE_FLAGS) -D_NVTX
# endif
# endif
# 
# # Setting OpenMP flags
# ifeq ($(strip $(OPENMP)),YES)
# 	CXXFLAGS += -fopenmp
# 	CUFLAGS += -Xcompiler -fopenmp
# 	CCFLAGS += -fopenmp
# 	FLAGS += -D_OMP
# endif


####### Directories

SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/bin
BINDIR = $(CURDIR)/bin
INCS  += -I$(CURDIR)/include

####### Files

OBJS = 	$(OBJDIR)/main.o

TARGET = planner-demo

####### Build rules

$(TARGET): $(OBJS)
	$(CXX) $(INCS) $(OBJS) $(LIBS) $(CXXFLAGS) -o $(BINDIR)/$(TARGET)

$(BINDIR)/%.o: src/%.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(INCS) -c -o $@ $< $(FLAGS) $(CXXFLAGS)

####### Phony Targets

# .PHONY: clean
# clean:
# 	cd obj && rm -f *.o && cd .. && rm -f $(BINDIR)/$(TARGET)

