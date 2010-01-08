
CC	= gcc
CP	= cp -a
CFLAGS  = \
	-Wall -W -O2 -g -Werror -Wshadow -Wbad-function-cast		\
	-Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes	\
	-Wmissing-declarations -Wnested-externs -Wredundant-decls

EXECS = glob nohup range # loadup

all: $(EXECS)
install: $(EXECS)
	$(CP) $(EXECS) ~/bin

clean:
	$(RM) core *~ *.o *.bak $(EXECS) 
	$(RM) -rf *.dSYM
