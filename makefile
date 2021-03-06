
DESTDIR ?=
BLD ?= bld
PROJECT ?= oled_linux
PROJECTS = $(subst /,,$(wildcard */))
BACKSLASH?=/
OUTFILE?=$(BLD)/$(PROJECT)
MKDIR?=mkdir -p
convert=$(subst /,$(BACKSLASH),$1)
INCLUDES += -Isrc

CXXFLAGS +=  -fno-rtti

CCFLAGS += -MD -g -Os -w -ffreestanding $(INCLUDES) -Wall -Werror \
	-Wl,--gc-sections -ffunction-sections -fdata-sections \
	$(EXTRA_CCFLAGS)
LDFLAGS += -L$(BLD) -loled_linux -loled_linux

default: all

.SUFFIXES: .bin .hex .srec

SRCS += $(wildcard SSD1306ClockDemo/*.cpp)

OBJS = $(addprefix $(BLD)/, $(addsuffix .o, $(basename $(SRCS))))


$(BLD)/%.o: %.c
	-$(MKDIR) $(call convert,$(dir $@))
	$(CC) -std=gnu11 $(CCFLAGS) $(CCFLAGS-$@) $(CCFLAGS-$(basename $(notdir $@))) -c $< -o $@

$(BLD)/%.o: %.cpp
	-$(MKDIR) $(call convert,$(dir $@))
	$(CXX) -std=c++11 $(CCFLAGS) $(CXXFLAGS) $(CCFLAGS-$(basename $(notdir $@))) -c $< -o $@
	
.PHONY: clean oled_linux all help


oled_linux:
	$(MAKE) -C src
SSD1306ClockDemo:
	$(MAKE) -C examples/SSD1306ClockDemo
SSD1306UiDemo:
	$(MAKE) -C examples/SSD1306UiDemo
SSD1306DrawingDemo:
	$(MAKE) -C examples/SSD1306DrawingDemo
screendrv:
	$(MAKE) -C examples/screendrv

all: oled_linux SSD1306ClockDemo SSD1306UiDemo SSD1306DrawingDemo screendrv

$(OUTFILE): $(OBJS) oled_linux
	-$(MKDIR) $(call convert,$(dir $@))
	$(CC) -o $(OUTFILE) $(CCFLAGS) $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(BLD)

-include $(OBJS:%.o=%.d)