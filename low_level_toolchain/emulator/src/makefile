C_FILES = $(wildcard src/*.c)
O_FILES = $(C_FILES:src/%.c=build/%.o)

CC_OPT = -std=c99 -Wall -I"./include"

.PHONY: all clean
.DEFAULT: all

all: emulator

emulator: $(O_FILES)
	gcc $(CC_OPT) -o $@ $^

build:
	@mkdir -p build

build/%.o: src/%.c | build
	gcc $(CC_OPT) -c $< -o $@

clean:
	@echo "Cleaning project..."
	@rm -f $(O_FILES) || true
	@rm -f emulator || true
	@rm -rf build || true
	@echo "Done"
