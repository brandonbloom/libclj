HEADERS = clj.h
SRC = clj.c tool.c
OBJ = $(patsubst %.c,%.o,$(SRC))

all: $(OBJ) cljtool

test: all
	./cljtool

cljtool: $(OBJ)
	clang $^ -o $@

%.o: %.c $(HEADERS)
	clang -c $< -o $@
