#############################################################################
# Makefile for building PathPlannerDemo
# Command: make planner-demo
#############################################################################

###### Configuration

CXX = g++
DEBUG_LEVEL = 1


##### Setup Flags

CXXFLAGS = -std=c++17 -fPIC -Wno-unknown-pragmas #-fstack-protector-all
LIBS = -lX11 -lGL -lpthread -lpng -lstdc++fs
WARN_ON = -Wall -Wextra -Wconversion
WARN_OFF = -Wno-narrowing

DEBUG_FLAGS = -g
RELEASE_FLAGS = -Ofast

ifeq ($(strip $(WARNINGS)),YES)
	CXXFLAGS += $(WARN_ON)
else
	CXXFLAGS += $(WARN_OFF) 
endif

ifeq ($(strip $(DEBUG_LEVEL)),1)
	CCFLAGS += $(DEBUG_FLAGS) -O3
	CXXFLAGS += $(DEBUG_FLAGS) -O3
else 
ifeq ($(strip $(DEBUG_LEVEL)),2)
	CCFLAGS += $(DEBUG_FLAGS) -O0
	CXXFLAGS += $(DEBUG_FLAGS) -O0
else
	CCFLAGS += $(RELEASE_FLAGS)
	CXXFLAGS += $(RELEASE_FLAGS)
endif
endif


####### Directories

SRCDIR = $(CURDIR)/src
OBJDIR = $(CURDIR)/obj
BINDIR = $(CURDIR)/bin
INCS  += -I$(CURDIR)/include
# INCS  += -I/home/jacob/.local/include/c++/11.2.1/

####### Files

OBJS = 	$(OBJDIR)/main.o \
	$(OBJDIR)/astar.o \
	$(OBJDIR)/gamemap.o


TARGET = planner-demo

####### Build rules

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(INCS) $(OBJS) $(LIBS) $(CXXFLAGS) -o $(BINDIR)/$(TARGET)

$(OBJDIR)/%.o: src/%.cpp include/%.hpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(INCS) -c -o $@ $< $(FLAGS) $(CXXFLAGS)

####### Phony Targets

.PHONY: clean
clean:
	cd obj && rm -f *.o && cd .. && rm -f $(BINDIR)/$(TARGET)

