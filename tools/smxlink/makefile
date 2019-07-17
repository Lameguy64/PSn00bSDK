TARGET      := smxlink

CPPFILES	= main.cpp timreader.cpp
CFLAGS		= -O2
LDFLAGS		= -s

INCLUDE		=
LIBDIRS		=
LIBS		= -ltinyxml2

CC			= gcc
CXX			= g++

OFILES		= $(addprefix build/,$(CPPFILES:.cpp=.o))

ifeq "$(OS)" "Windows_NT"

# Config for Windows (comment out if in msys2)
INCLUDE		= -I/c/tinyxml2
LIBDIRS		= -L/c/tinyxml2

TARGET		:= $(TARGET).exe

else

# Config for anything else that isn't Windows
EXE_SUFFIX	=

endif

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
all: $(OFILES)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) $(OFILES) $(LIBS) -o $(TARGET)
	
install:
	mkdir -p ../bin
	cp $(TARGET) ../bin/$(TARGET)

clean:
	rm -Rf build $(TARGET)

cleanall: clean
