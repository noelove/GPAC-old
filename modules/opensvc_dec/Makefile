include ../../config.mak

vpath %.c $(SRC_PATH)/modules/opensvc_dec

CFLAGS=$(OPTFLAGS) -I"$(SRC_PATH)/include" $(OSVC_CFLAGS)

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
LDFLAGS+=-pg
endif

EXTRALIBS+=-L../../bin/gcc -lgpac $(OSVC_LDFLAGS)

#common obj
OBJS= opensvc_dec.o

SRCS := $(OBJS:.o=.c)

LIB=gm_opensvc_dec.$(DYN_LIB_SUFFIX)


all: $(LIB)


$(LIB): $(OBJS)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/$@ $(OBJS) $(EXTRALIBS) -L../../bin/gcc -lgpac
ifeq ($(STATICBUILD),yes)
	$(CC) $(SHFLAGS) $(LDFLAGS) -o ../../bin/gcc/gm_opensvc_dec-static.$(DYN_LIB_SUFFIX) $(OBJS) $(EXTRALIBS) -L../../bin/gcc -lgpac_static -lz
endif


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -f $(OBJS) ../../bin/gcc/$(LIB)

dep: depend

depend:
	rm -f .depend
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
