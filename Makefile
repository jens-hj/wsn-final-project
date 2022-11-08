CONTIKI_PROJECT = dct
all: $(CONTIKI_PROJECT)

TARGET_LIBFILES += -lm

CONTIKI = ../..
include $(CONTIKI)/Makefile.include