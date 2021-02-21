# Run using make (Linux) or gmake (BSD)
# Part of the PSn00bSDK Project
# 2019 - 2020 Lameguy64 / Meido-Tek Productions

include ../../template/psn00bsdk-setup.mk

TARGET	= libc.a

INCLUDE			= -I../include

CFLAGS			= -g -O2 -msoft-float -fno-builtin -fdata-sections \
				  -ffunction-sections -Wa,--strip-local-absolute
AFLAGS			= -g -msoft-float -Wa,-strip-local-absolute

CFILES			= $(notdir $(wildcard ./*.c))
CXXFILES		= $(notdir $(wildcard ./*.cxx))
AFILES			= $(notdir $(wildcard ./*.s))

OFILES			= $(addprefix build/,$(CFILES:.c=.o) $(CXXFILES:.cxx=.o) \
				  $(AFILES:.s=.o))

ifndef PSN00BSDK_LIBS

PSN00BSDK_LIBS	= ..

endif

all: $(TARGET)

$(TARGET): $(OFILES)
	cp $(GCC_BASE)/lib/gcc/$(PREFIX)/$(GCC_VERSION)/libgcc.a ./$(TARGET)
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
	$(CC) $(AFLAGS) $(INCLUDE) -c $< -o $@

install:
ifneq ($(PSN00BSDK_LIBS), "..")
	@mkdir -p $(PSN00BSDK_LIBS)
endif
	cp $(TARGET) $(PSN00BSDK_LIBS)/$(TARGET)
	
clean:
	rm -Rf build $(TARGET)
