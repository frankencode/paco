%.c: %.png embed
	./embed $< > $@

all: embed \
	cdup.c \
	cdupNegativ.c \
	popup.c \
	popupNegativ.c \
	plus.c \
	plusNegativ.c \
	wheel.c \
	wheelNegativ.c \
	history.c \
	historyNegativ.c \
	bookmark.c \
	bookmarkNegativ.c \
	scrollLeft.c \
	scrollLeftNegativ.c \
	scrollRight.c \
	scrollRightNegativ.c \
	more.c \
	moreNegativ.c \
	close.c \
	closeNegativ.c \
	magnifier.c \
	magnifierPopup.c \
	toolOpen.c \
	toolOpenNegativ.c \
	toolClosed.c \
	toolClosedNegativ.c \
	gotoLine.c \
	gotoLineNegativ.c \
	reload.c \
	reloadNegativ.c

embed: embed.c Makefile
	${CC} embed.c -o embed

clean:
	rm -rf embed $$(find . -name '*.c' -and -not -name 'embed.c')
