SPEED = -Og
CFLAGS_EXTRA = -fno-toplevel-reorder
LDFLAGS_EXTRA = 

# END OF CONFIGURABLE OPTIONS

XED_DIR = $(shell ./gen_xed_dir_name)

# create list of object files
OBJS = $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJS := $(notdir $(OBJS))
OBJS := $(patsubst %.c, %.o, $(OBJS))
OBJS := $(addprefix bin/, $(OBJS))

# xed paths
XED_INC = xed/kits/$(XED_DIR)/include
XED_LIB = xed/kits/$(XED_DIR)/lib/libxed.a

# set vpath to src directory and any direct subdir of src
vpath %.c src $(wildcard src/*)

CFLAGS = -Wall -Wextra -Werror -MD $(SPEED) -I$(XED_INC) -Iinclude $(CFLAGS_EXTRA)
LDFLAGS = $(LDFLAGS_EXTRA)

# phony rules
all: out/bmve

run: out/bmve
	@out/bmve

clean:
	rm -rf out bin

cleanxed:
	xed/mfile.py --clean
	rm -rf xed/kits

cleanall: clean cleanxed

# build xed library
$(XED_LIB):
	cd xed; ./mfile.py --no-encoder --limit-strings install

# build 
out/bmve: $(OBJS) $(XED_LIB) | out
	gcc $(LDFLAGS) $^ -o $@

bin/%.o: %.c | $(XED_LIB) bin
	gcc $(CFLAGS) -c $< -o $@

out:
	mkdir out
bin:
	mkdir bin

bmve: out/bmve
%.o: bin/%.o

-include $(notdir $(OBJS:.o=.d))

.PHONY: all clean cleanxed cleanall
