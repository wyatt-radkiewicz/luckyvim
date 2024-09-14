#
# Simple makefile by __ e k L 1 p 5 3 d __
#

# Variables
SRCS	:=$(shell find src/ -name "*.c")
BUILD	:=build
OUT	:=$(BUILD)/luv
OBJS	:=$(patsubst %.c,$(BUILD)/%.o,$(SRCS))

# Environment variables
CFLAGS :=$(CFLAGS) -Isrc $(shell pkg-config tree-sitter --cflags) -O0 -g -std=gnu99
LDFLAGS :=$(LDFLAGS) $(shell pkg-config tree-sitter --libs) -lm

# Build the main executable
$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

# Clean the project directory
.PHONY: clean
clean:
	rm -rf $(BUILD)

# Remember that when this call is evaluated, it is expanded TWICE!
define COMPILE
$$(BUILD)/$(dir $(2))$(1)
	mkdir -p $$(dir $$@)
	$$(CC) $$(CFLAGS) -c $(2) -o $$@
endef

# Go through every source file use gcc to find its pre-reqs and create a rule
$(foreach src,$(SRCS),$(eval $(call COMPILE,$(shell $(CC) $(CFLAGS) -M $(src) | tr -d '\\'),$(src))))

