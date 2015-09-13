include common.mk

CFLAGS  += -Iinclude

LIB     := libc11.a
ifeq ($(OSNAME),Windows)
SOURCES := src/win32.c
else
SOURCES := src/pthread.c
endif
OBJECTS := $(SOURCES:%.c=%.o)

all: $(LIB)

clean:
	$(RM) $(LIB) $(OBJECTS)

$(LIB): $(OBJECTS)
	$(QUIET_AR)$(AR) $(ARFLAGS) $@ $^
	$(QUIET_RANLIB)$(RANLIB) $@

%.o: %.c .cflags
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean
