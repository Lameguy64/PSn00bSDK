# This LZP library builds off the lzp sources in libpsn00b/lzp. The only
# difference is this is built with compression enabled specified in the
# lzconfig.h file and the library is built for the host platform.

TARGET	= liblzp.a

CFILES	= $(wildcard ../../../libpsn00b/lzp/*.c)
OFILES	= $(addprefix build/, $(notdir $(CFILES:.c=.o)))

INCLUDE = -I../include -I.

CFLAGS	= -g -O2

CC		= $(PREFIX)gcc
AR		= $(PREFIX)ar
RANLIB	= $(PREFIX)ranlib

all: $(TARGET)

$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)

# Dunno if there's a better way to do this but it works at least
build/%.o: ../../../libpsn00b/lzp/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -Rf build $(TARGET)