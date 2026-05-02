.DEFAULT_GOAL := all

.NOTPARALLEL: all

XF_THEME ?= default
CMAKE_ARGS ?=

.PHONY: build prepare all setup

setup:
	@if [ "$$(uname)" != "Darwin" ]; then \
		echo "setup target is macOS only — install cmake, pkg-config, and cairo manually."; \
		exit 1; \
	fi
	@which brew > /dev/null 2>&1 || \
		(echo "Homebrew not found. Install it from https://brew.sh then re-run." && exit 1)
	brew install cmake pkg-config cairo

build:
	+@cmake --build build --config Release

prepare:
	@cmake -S . -B build -DXF_THEME=$(XF_THEME) $(CMAKE_ARGS)

all: clean-build prepare build

clean-%:
	@rm -rf $*

clean: clean-build clean-bin