# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 Lameguy64 / Meido-Tek Productions

include ../common.mk

TARGET	= ../libpsxapi.a

SOURCES	= stdio fs sys

AFILES	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s))
OFILES	= $(addprefix build/,$(AFILES:.s=.o))

AFLAGS	= -g -msoft-float -strip-local-absolute

all: $(TARGET)

$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)

build/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(AFLAGS) $(INCLUDE) $< -o $@

clean:
	rm -Rf build $(TARGET)
