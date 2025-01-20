.DEFAULT_GOAL := all

.PHONY: build prepare all

build:
	@cmake --build build --config Release

prepare:
	@cmake -B build

all: prepare build clean-build

clean-%:
	@rm -rf $*

clean: clean-build clean-bin