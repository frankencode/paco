%.c: %.png embed
	./embed $< > $@

all: embed \
  filenew.c \
  wwwlookup.c \
  wwwbrowser.c \
  wwwbook.c \
  scriptadd.c \
  scriptstop.c \
  scriptstart.c \
  scriptreset.c \
  scripthalt.c \
  script.c \
  question.c \
  print.c \
  book.c \
  fileverify.c \
  pencil.c \
  idea.c \
  chat.c \
  protect.c \
  piechart.c \
  barchart.c \
  reload.c \
  preferences.c \
  filebrowser.c \
  codebrowser.c

embed: ../embed.c Makefile
	${CC} ../embed.c -o embed

clean:
	rm -rf embed $$(find . -name '*.c' -and -not -name 'embed.c')
