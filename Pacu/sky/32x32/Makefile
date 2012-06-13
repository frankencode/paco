%.c: %.png embed
	./embed $< > $@

all: embed \
  filenew.c \
  wwwbook.c \
  book.c \
  idea.c \
  chat.c \
  print.c \
  fileverify.c \
  wwwlookup.c \
  protect.c \
  script.c \
  scripthalt.c \
  scriptstop.c \
  scriptstart.c \
  scriptreset.c \
  piechart.c \
  barchart.c \
  filedown.c \
  fileup.c \
  folderdown.c \
  folderup.c \
  critical.c \
  pencil.c \
  filebrowser.c \
  codebrowser.c \
  information.c \
  preferences.c \
  question.c \
  reload.c \
  search.c \
  terminal.c \
  wwwbrowser.c

embed: ../embed.c Makefile
	${CC} ../embed.c -o embed

clean:
	rm -rf embed $$(find . -name '*.c' -and -not -name 'embed.c')
