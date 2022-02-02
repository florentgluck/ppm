CC:=gcc
CFLAGS:=-g -Wall -Wextra -std=gnu11 -MMD -fsanitize=address -fsanitize=leak -fsanitize=undefined 
LIBS:=

BIN:=ppm_example
IMG_SRC:=image.ppm
IMG_DST:=output.ppm
SRCS:=$(shell find . -name "*.c")
OBJS:=$(SRCS:.c=.o)
DEPS:=$(OBJS:%.o=%.d)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(OBJS) $(BIN) $(DEPS) $(IMG_DST)

test: $(BIN) $(IMG_SRC)
	@echo "The example program below reads $(IMG_SRC) and creates $(IMG_DST):"
	./$(BIN) $(IMG_SRC) $(IMG_DST)

-include $(DEPS)
