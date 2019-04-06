TARGET      := lzpack

CPPFILES	= main.cpp filelist.cpp
CFLAGS		= -O2
LDFLAGS		= -s

LIBS		= -ltinyxml2 -llzp

CC			= gcc
CXX			= g++

OFILES		= $(addprefix build/,$(CPPFILES:.cpp=.o))

ifeq "$(OS)" "Windows_NT"

# Config for Windows
INCLUDE		= -I/c/tinyxml2
LIBDIRS		= -L/c/tinyxml2
TARGET		:= $(TARGET).exe

else

# Config for anything else that isn't Linux

endif

INCLUDE		+= -I../../libpsn00b
LIBDIRS		+= -Llzp

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
all: $(OFILES)
	$(MAKE) -C lzp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) $(OFILES) $(LIBS) -o $(TARGET)
	
install:
	mkdir -p ../bin
	cp $(TARGET) ../bin/$(TARGET)

clean:
	$(MAKE) -C lzp clean
	rm -Rf build $(TARGET)

cleanall: clean
