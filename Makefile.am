bin_PROGRAMS = tpc

tpc_SOURCES = \
	pcg.h \
	component.h component.c \
	production.h production.c \
	grammar.h grammar.c \
	kernel.h kernel.c \
	parser.h parser.c \
	getopt.h getopt.c getopt1.c \
	main.c

pcg.h: grammar.pcg
	@PC@ < $< > $@ 2> /dev/null