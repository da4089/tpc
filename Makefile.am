bin_PROGRAMS = go

go_SOURCES = \
	pcg.h \
	grammar.h grammar.c \
	component.h component.c \
	production.h production.c \
	parser.h parser.c

pcg.h: grammar.pcg
	@PC@ < $< > $@ 2> /dev/null