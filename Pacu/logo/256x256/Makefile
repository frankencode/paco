%.c: %.png embed
	./embed $< > $@

all: embed \
	pacu.c

embed: ../embed.c Makefile
	${CC} ../embed.c -o embed

clean:
	rm -rf embed $$(find . -name '*.c' -and -not -name 'embed.c')
