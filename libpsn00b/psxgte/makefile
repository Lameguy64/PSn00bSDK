# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 Lameguy64 / Meido-Tek Productions

include ../common.mk

TARGET	= ../libpsxgte.a

CFILES	= $(notdir $(wildcard ./*.c))
AFILES	= $(notdir $(wildcard ./*.s))
OFILES	= $(addprefix build/,$(CFILES:.c=.o) $(AFILES:.s=.o))

CFLAGS	= -g -O2 -msoft-float -fno-builtin -fdata-sections -ffunction-sections -Wa,--strip-local-absolute
AFLAGS	= -g -msoft-float -strip-local-absolute

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
