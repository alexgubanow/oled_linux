
BLD ?= ../bld/liboled_linux
BACKSLASH?=/
MKDIR?=mkdir -p
convert=$(subst /,$(BACKSLASH),$1)

CXXFLAGS +=  -fno-rtti

CCFLAGS += -O3 -MD -g -w -ffreestanding $(INCLUDES) -Wall -Werror \
	-Wl,--gc-sections -ffunction-sections -fdata-sections \
	$(EXTRA_CCFLAGS)


$(BLD)/%.o: %.c
	-$(MKDIR) $(call convert,$(dir $@))
	$(CC) -std=gnu11 $(CCFLAGS) $(CCFLAGS-$@) $(CCFLAGS-$(basename $(notdir $@))) -c $< -o $@

$(BLD)/%.o: %.cpp
	-$(MKDIR) $(call convert,$(dir $@))
	$(CXX) -std=c++11 $(CCFLAGS) $(CXXFLAGS) $(CCFLAGS-$(basename $(notdir $@))) -c $< -o $@
	

SRCS += $(wildcard *.cpp)

OBJS = $(addprefix $(BLD)/, $(addsuffix .o, $(basename $(SRCS))))

LDFLAGS += -L$(BLD) -lwiringPI

####################### Compiling library #########################

$(BLD)/liboled_linux.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

oled_linux: $(BLD)/liboled_linux.a

all: oled_linux

clean:
	rm -rf $(BLD)
