LIBTOOL=libtool

# defines source files and vpaths
include Sources

# What source objects are we building?
OBJECTS=$(SOURCES:.cpp=.o)

CXXFLAGS += -fPIC

.PHONY: all clean cleandeps cleanobjs cleanlib libnoise libnoise.so libnoise.so

# hooks for future makefiles being able to make multiple SOs, or older SOs
libnoise: libnoise.so #libnoise.a libnoise.la

# Real build targets
libnoise.so: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -shared -Wl,-soname=libnoise.so -o $@ $(OBJECTS)

libnoise.a: $(OBJECTS)
	$(AR) $@ $(OBJECTS)

clean:	cleandeps cleanobjs cleanlib
cleandeps:
	-rm $(DEPENDS)
cleanobjs:
	-rm $(OBJECTS)
	-rm $(OBJECTS:.o=.lo) #clean up after libtool
	-rm -rf .libs model/.libs module/.libs
cleanlib:
	-rm libnoise.so
	-rm libnoise.a
	-rm libnoise.la

# Implicit build rules
%.o %.lo: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $(@)

%.o %.lo: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(@)

# If dependancies have never been built this will produce a horde of
# "file not found" warnings and *then* build the deps.  Very odd.
include $(DEPENDS)
