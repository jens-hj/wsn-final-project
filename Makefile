# CONTIKI_PROJECT = aes
# CONTIKI_PROJECT = client aes server
CONTIKI_PROJECT = client
all: $(CONTIKI_PROJECT)

# Add library flags
# TARGET_LIBFILES += -lm

# Add project modules
include $(CONTIKI)/Makefile.dir-variables
# MODULES += $(CONTIKI_NG_SERVICES_DIR)/simple-energest

CONTIKI = ../..
include $(CONTIKI)/Makefile.include