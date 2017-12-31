CXX = c++
LD = c++
CXXFLAGS += -O0 -Wall -std=c++17 -MP -MD -fPIC -Wno-unused-parameter -g -Wno-unused-private-field -Wnon-virtual-dtor -fno-omit-frame-pointer \
	-Wno-unused-lambda-capture -Wno-unknown-warning-option # clang 5.0 trunk is retarded
SOFLAGS += -shared
LDFLAGS +=
LIBRARIES += -pthread -lboost_system -lboost_filesystem -lboost_program_options -ldl

SOURCES := $(shell find lib -name "*.cpp")
MAINSRC := $(shell find src -name "*.cpp" 2>/dev/null)
TESTSRC := $(shell find tests -name "*.cpp")
OBJECTS := $(SOURCES:.cpp=.o)
MAINOBJ := $(MAINSRC:.cpp=.o)
TESTOBJ := $(TESTSRC:.cpp=.o)

PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)
BINDIR ?= $(EXEC_PREFIX)/bin
LIBDIR ?= $(EXEC_PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

LIBRARY = libvprc.so
EXECUTABLE = vprc

all: $(EXECUTABLE)

library: $(LIBRARY)

$(EXECUTABLE): $(MAINOBJ) $(LIBRARY)
	$(LD) $(CXXFLAGS) $(LDFLAGS) $(MAINOBJ) -o $@ $(LIBRARIES) -L. $(LIBRARY) -Wl,-rpath,'$$ORIGIN'

$(LIBRARY): $(OBJECTS)
	$(LD) $(CXXFLAGS) $(SOFLAGS) $(OBJECTS) -o $@ $(LIBRARIES)

test: ./tests/test

./tests/test: $(TESTOBJ) $(LIBRARY)
	$(LD) $(CXXFLAGS) $(LDFLAGS) $(TESTOBJ) -o $@ $(LIBRARIES) -lboost_iostreams -L. $(LIBRARY) -Wl,-rpath,'$$ORIGIN/..'

#install: $(LIBRARY) $(EXECUTABLE)
#	@cp $(EXECUTABLE) $(DESTDIR)$(BINDIR)/$(EXECUTABLE)
#	@cp $(LIBRARY) $(DESTDIR)$(LIBDIR)/$(LIBRARY).1
#	@ln -sfn $(DESTDIR)$(LIBDIR)/$(LIBRARY).1 $(DESTDIR)$(LIBDIR)/$(LIBRARY)
#	@mkdir -p $(DESTDIR)$(INCLUDEDIR)/reaver
#	@cp -RT include $(DESTDIR)$(INCLUDEDIR)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ -I./include/reaver

./tests/%.o: ./tests/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ -I./include/reaver

clean:
	@find . -name "*.o" -delete
	@find . -name "*.d" -delete
	@rm -f $(LIBRARY)
	@rm -f $(EXECUTABLE)
	@rm -f tests/test

.PHONY: install clean library test

-include $(SOURCES:.cpp=.d)
-include $(MAINSRC:.cpp=.d)
-include $(TESTSRC:.cpp=.d)
