# Copied from projects/make and modified for emscripten
# To run, make sure emsdk is in your path (our sourced via their scripts)
# run `emmake make`

ifndef config
  config=release_32bit
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug_32bit)
  wren_config = debug_32bit
  wren_try_config = debug_32bit

else ifeq ($(config),release_32bit)
  wren_config = release_32bit
  wren_try_config = release_32bit

else
  $(error "invalid configuration $(config)")
endif

PROJECTS := wren wren_try

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

wren:
ifneq (,$(wren_config))
	@echo "==== Building wren ($(wren_config)) ===="
	@${MAKE} --no-print-directory -C . -f wren.make config=$(wren_config)
endif

wren_try: wren
ifneq (,$(wren_try_config))
	@echo "==== Building wren_try ($(wren_try_config)) ===="
	@${MAKE} --no-print-directory -C . -f wren_try.make config=$(wren_try_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f wren.make clean
	@${MAKE} --no-print-directory -C . -f wren_try.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  release_32bit"
	@echo "  debug_32bit"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   wren"
	@echo "   wren_try"
	@echo ""
