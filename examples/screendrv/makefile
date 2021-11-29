
DESTDIR ?=
BLD ?= ../../bld/examples/screendrv
PROJECT ?= screendrv
PROJECTS = $(subst /,,$(wildcard */))
BACKSLASH?=/
OUTFILE?=../../bld/$(PROJECT)
MKDIR?=mkdir -p
convert=$(subst /,$(BACKSLASH),$1)
INCLUDES += -I../../examples/$(PROJECT) -I../../src

CXXFLAGS +=  -fno-rtti

CCFLAGS += -MD -O3 -g -w -ffreestanding $(INCLUDES) -Wall -Werror \
	-Wl,--gc-sections -ffunction-sections -fdata-sections \
	$(EXTRA_CCFLAGS)
LDFLAGS += -L../../bld/liboled_linux -lwiringPi -loled_linux -lpthread

default: all

.SUFFIXES: .bin .hex .srec

SRCS += $(wildcard *.cpp)

OBJS = $(addprefix $(BLD)/, $(addsuffix .o, $(basename $(SRCS))))


$(BLD)/%.o: %.c
	-$(MKDIR) $(call convert,$(dir $@))
	$(CC) -std=gnu11 $(CCFLAGS) $(CCFLAGS-$@) $(CCFLAGS-$(basename $(notdir $@))) -c $< -o $@

$(BLD)/%.o: %.cpp
	-$(MKDIR) $(call convert,$(dir $@))
	$(CXX) -std=c++11 $(CCFLAGS) $(CXXFLAGS) $(CCFLAGS-$(basename $(notdir $@))) -c $< -o $@
	
.PHONY: clean all

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	-$(MKDIR) $(call convert,$(dir $@))
	$(CXX) -o $(OUTFILE) $(CCFLAGS) $(OBJS) $(LDFLAGS)
	systemctl stop $(PROJECT)
	cp $(OUTFILE) /usr/bin/$(PROJECT)
#	systemctl start $(PROJECT)

clean:
	rm -rf $(BLD)

-include $(OBJS:%.o=%.d)