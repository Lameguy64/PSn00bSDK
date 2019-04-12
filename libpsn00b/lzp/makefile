PREFIX	= mipsel-unknown-elf-

TARGET	= liblzp.a

CFILES	= $(notdir $(wildcard ./*.c))
OFILES	= $(addprefix build/,$(CFILES:.c=.o))

INCLUDE = -I../include

CFLAGS	= -g -O2 -msoft-float -fno-builtin -nostdlib -Wa,--strip-local-absolute
AFLAGS	= -msoft-float --strip-local-absolute

CC		= $(PREFIX)gcc
AR		= $(PREFIX)ar
RANLIB	= $(PREFIX)ranlib

all: $(TARGET)

$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)
	
build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -Rf build $(OFILES) $(TARGET)
