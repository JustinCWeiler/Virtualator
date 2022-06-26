SPEED = -Og
CFLAGS_EXTRA = 
LDFLAGS_EXTRA = 

XED_DIR = xed-install-base-2022-06-26-lin-x86-64

# END OF CONFIGURABLE OPTIONS

# create list of object files
OBJS = $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJS := $(notdir $(OBJS))
OBJS := $(patsubst %.c, %.o, $(OBJS))
OBJS := $(addprefix bin/, $(OBJS))

# xed paths
XED_INC = xed/kits/$(XED_DIR)/include
XED_LIB = xed/kits/$(XED_DIR)/lib/libxed-ild.a

# set vpath to src directory and any direct subdir of src
vpath %.c src $(wildcard src/*)

CFLAGS = -Wall -Wextra -Werror -MD $(SPEED) -I$(XED_INC) $(CFLAGS_EXTRA)
LDFLAGS = $(LDFLAGS_EXTRA)

all: out/bmve

run: out/bmve
	@out/bmve

clean:
	rm -rf out bin

cleanxed:
	xed/mfile.py --clean
	rm -rf xed/kits

cleanall: clean cleanxed

out/bmve: $(OBJS) $(XED_LIB) | out
	gcc $(LDFLAGS) $^ -o $@

$(XED_LIB):
	cd xed; ./mfile.py --no-encoder --limit-strings install

bmve: out/bmve

bin/%.o: %.c | $(XED_LIB) bin
	gcc $(CFLAGS) -c $< -o $@

%.o: bin/%.o

out:
	mkdir out
bin:
	mkdir bin

.PHONY: all clean cleanxed cleanall
