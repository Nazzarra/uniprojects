C_FILES = $(wildcard src/*.c)
O_FILES = $(C_FILES:src/%.c=build/%.o)

CC_OPT = -std=c99 -Wall -I"./include"

.PHONY: all clean
.DEFAULT: all

all: assembler

assembler: $(O_FILES) parser/src/asm_parser.tab.c parser/src/lex.yy.c
	gcc $(CC_OPT) -o $@ $^

include/asm_parser.tab.h parser/src/asm_parser.tab.c: parser/asm_parser.y
	bison -d parser/asm_parser.y
	mv asm_parser.tab.c parser/src/asm_parser.tab.c
	mv asm_parser.tab.h include/asm_parser.tab.h

parser/src/lex.yy.c: parser/asm_lexer.l include/asm_parser.tab.h
	flex parser/asm_lexer.l
	mv lex.yy.c parser/src/lex.yy.c

build:
	@mkdir -p build

build/%.o: src/%.c | build
	gcc $(CC_OPT) -c $< -o $@

clean:
	@echo "Cleaning project..."
	@rm -f $(O_FILES) || true
	@rm -f assembler || true
	@rm -rf build || true
	@rm include/asm_parser.tab.h &> /dev/null || true
	@rm parser/src/asm_parser.tab.c || true
	@rm parser/src/lex.yy.c || true
	@echo "Done"
