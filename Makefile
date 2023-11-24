Q ?= 
CC = arm-none-eabi-gcc
BUILD_DIR = output
NWLINK = npx --yes -- nwlink@0.0.17
LINK_GC = 1
LTO = 1

define object_for
$(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(1))))
endef

src = $(addprefix src/,\
  main.c \
  gnuchess.c \
  opening.c \
)

CFLAGS = -std=c99
CFLAGS += $(shell $(NWLINK) eadk-cflags)
CFLAGS += -Os -Wall
#~ CFLAGS += -ggdb
LDFLAGS = -s -Wl,--relocatable
LDFLAGS += -nostartfiles
LDFLAGS += --specs=nano.specs
#~ LDFLAGS += --specs=nosys.specs # Alternatively, use full-fledged newlib

ifeq ($(LINK_GC),1)
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level
LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(LTO),1)
CFLAGS += -flto -fno-fat-lto-objects
CFLAGS += -fwhole-program
CFLAGS += -fvisibility=internal
LDFLAGS += -flinker-output=nolto-rel
endif

.PHONY: build
build: $(BUILD_DIR)/chess.nwa

.PHONY: check
check: $(BUILD_DIR)/chess.bin

.PHONY: run
run: $(BUILD_DIR)/chess.nwa
	@echo "INSTALL $<"
	$(Q) $(NWLINK) install-nwa $<

$(BUILD_DIR)/chess.nwa: $(call object_for,$(src)) $(BUILD_DIR)/icon.o
	@echo "LD      $@"
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lm

$(addprefix $(BUILD_DIR)/,%.o): %.c | $(BUILD_DIR)
	@echo "CC      $^"
	$(Q) $(CC) $(CFLAGS) -c $^ -o $@

$(BUILD_DIR)/icon.o: src/icon.png
	@echo "ICON    $<"
	$(Q) $(NWLINK) png-icon-o $< $@

.PRECIOUS: $(BUILD_DIR)
$(BUILD_DIR):
	$(Q) mkdir -p $@/src

.PHONY: clean
clean:
	@echo "CLEAN"
	$(Q) rm -rf $(BUILD_DIR)
