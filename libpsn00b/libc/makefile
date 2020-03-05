# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 Lameguy64 / Meido-Tek Productions

include ../common.mk

TARGET	= ../libc.a

CFILES		= $(notdir $(wildcard ./*.c))
CXXFILES	= $(notdir $(wildcard ./*.cxx))
AFILES		= $(notdir $(wildcard ./*.s))
OFILES		= $(addprefix build/,$(CFILES:.c=.o) $(CXXFILES:.cxx=.o) $(AFILES:.s=.o))

CFLAGS	= -g -O2 -msoft-float -fno-builtin -fdata-sections -ffunction-sections -Wa,--strip-local-absolute
AFLAGS	= -g -msoft-float --strip-local-absolute


all: $(TARGET)

$(TARGET): $(OFILES)
	cp $(GCC_BASE)/lib/gcc/mipsel-unknown-elf/$(GCC_VERSION)/libgcc.a ./$(TARGET)
	$(AR) r $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

build/%.o: %.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

build/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -Rf build $(TARGET)
