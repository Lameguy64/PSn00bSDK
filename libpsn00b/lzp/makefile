# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 - 2020 Lameguy64 / Meido-Tek Productions

include ../../template/psn00bsdk-setup.mk

TARGET		= liblzp.a

INCLUDE		= -I../include

CFILES		= $(notdir $(wildcard ./*.c))
OFILES		= $(addprefix build/,$(CFILES:.c=.o))

CFLAGS		= -g -O2 -msoft-float -fno-builtin -nostdlib -fdata-sections \
			  -ffunction-sections -Wa,--strip-local-absolute

ifndef PSN00BSDK_LIBS

PSN00BSDK_LIBS	= ..

endif

all: $(TARGET)

$(TARGET): $(OFILES)
	$(AR) cr $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)
	
build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

install:
ifneq ($(PSN00BSDK_LIBS), "..")
	@mkdir -p $(PSN00BSDK_LIBS)
endif
	cp $(TARGET) $(PSN00BSDK_LIBS)/$(TARGET)
	
clean:
	rm -Rf build $(OFILES) $(TARGET)
