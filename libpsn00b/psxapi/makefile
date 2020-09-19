# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 - 2020 Lameguy64 / Meido-Tek Productions

include ../../psn00bsdk-setup.mk

TARGET		= libpsxapi.a

INCLUDE 	= -I../include

SOURCES		= stdio fs sys

AFILES		= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s))
OFILES		= $(addprefix build/,$(AFILES:.s=.o))

AFLAGS		= -g -msoft-float -Wa,--strip-local-absolute

ifndef PSN00BSDK_LIBS

PSN00BSDK_LIBS	= ..

endif

all: $(TARGET)
	
$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)
	
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS) $(INCLUDE) -c $< -o $@

install:
ifneq ($(PSN00BSDK_LIBS), "..")
	@mkdir -p $(PSN00BSDK_LIBS)
endif
	cp $(TARGET) $(PSN00BSDK_LIBS)/$(TARGET)

clean:
	rm -Rf build $(TARGET)
