# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 - 2020 Lameguy64 / Meido-Tek Productions

include ../../psn00bsdk-setup.mk

TARGET		= libpsxspu.a

INCLUDE		= -I../include

CFILES		= $(notdir $(wildcard ./*.c))
AFILES		= $(notdir $(wildcard ./*.s))
OFILES		= $(addprefix build/,$(CFILES:.c=.o) $(AFILES:.s=.o))

CFLAGS		= -g -O2 -msoft-float -fdata-sections -ffunction-sections \
			  -Wa,--strip-local-absolute
AFLAGS		= -g -msoft-float -strip-local-absolute

ifndef PSN00BSDK_LIBS

PSN00BSDK_LIBS	= ..

endif

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
	
install:
ifneq ($(PSN00BSDK_LIBS), "..")
	@mkdir -p $(PSN00BSDK_LIBS)
endif
	cp $(TARGET) $(PSN00BSDK_LIBS)/$(TARGET)

clean:
	rm -Rf build $(TARGET)
