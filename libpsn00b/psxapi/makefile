# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 Lameguy64 / Meido-Tek Productions

include ../common.mk

TARGET	= ../libpsxapi.a

SOURCES	= stdio fs sys

AFILES	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s))
OFILES	= $(addprefix build/,$(AFILES:.s=.o))

AFLAGS	= -g -msoft-float -Wa,--strip-local-absolute

CC		= $(PREFIX)gcc
AS		= $(PREFIX)as
AR		= $(PREFIX)ar
RANLIB	= $(PREFIX)ranlib

all: $(TARGET)
	
$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)
	
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -Rf build $(TARGET)
