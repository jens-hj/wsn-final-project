CONTIKI_PROJECT = client
all: $(CONTIKI_PROJECT)

# Add library flags
# TARGET_LIBFILES += -lm

# Add project modules
# MODULES += os/services/simple-energest

WERROR=0

CONTIKI = ../..
include $(CONTIKI)/Makefile.include