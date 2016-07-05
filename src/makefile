cc = gcc
CFLAGS = -std=c99 -Wall -Wpointer-arith -Wsign-compare -Wno-unused-result \
         -fno-omit-frame-pointer -funroll-loops \
         -fPIC -pipe -g3 -O3 \
         -D_GNU_SOURCE -D_BSD_SOURCE -D_POSIX_SOURCE -D_REENTRANT -DOPEN_PRINT \
         -I.
LIBS = -lpthread
srcs = $(wildcard *.c)
cobjs = $(srcs:.c=.o)
objdir = .objs
objs = $(addprefix $(objdir)/, $(cobjs))

$(objdir):
	@mkdir -p $@

target = lmvs

all: $(target)

$(target): $(objs)
	$(cc) $(CFLAGS) -o $@ $^ $(LIBS)

$(objdir)/%.o: %.c | $(objdir)
	$(cc) $(CFLAGS) -c $< -o $@

install:
	cp $(target) ../bin

.PHONY: clean
clean:
	rm -rf $(objdir)
	rm -f $(target)
