PREFIX	= mipsel-unknown-elf-

TARGET	= ../libpsxspu.a

CFILES	= $(notdir $(wildcard ./*.c))
AFILES	= $(notdir $(wildcard ./*.s))
OFILES	= $(addprefix build/,$(CFILES:.c=.o) $(AFILES:.s=.o))

INCLUDE = -I../include

CFLAGS	= -O2 -g -msoft-float -Wa,--strip-local-absolute
AFLAGS	= -g -msoft-float --strip-local-absolute

CC		= $(PREFIX)gcc
AS		= $(PREFIX)as
AR		= $(PREFIX)ar
RANLIB	= $(PREFIX)ranlib

all: $(TARGET)

$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -Rf build $(TARGET)
