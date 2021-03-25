CC      := gcc
CFLAGS  := -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE -std=c99 \
-Werror -pedantic `pkg-config --cflags sdl2 SDL2_mixer SDL2_image SDL2_ttf`
LDFLAGS := `pkg-config --libs sdl2 SDL2_mixer SDL2_image SDL2_ttf`

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)

.PHONY: all clean

all: guitar_amateur

guitar_amateur: $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

%.d: %.c
	gcc -MM $(CFLAGS) $< > $@

-include $(DEPS)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f guitar_amateur
