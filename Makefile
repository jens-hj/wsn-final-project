# CONTIKI_PROJECT = aes
# CONTIKI_PROJECT = client aes server
CONTIKI_PROJECT = client
all: $(CONTIKI_PROJECT)

# Add library flags
# TARGET_LIBFILES += -lm

# Add project modules
include $(CONTIKI)/Makefile.dir-variables
# MODULES += $(CONTIKI_NG_SERVICES_DIR)/simple-energest


# If all flags, full energy will be measured
# If specific time and energy flag, nothing will be measured

# CPPFLAGS += -DSINGLE_MOTE_TESTING
# CPPFLAGS += -DMEASURE_ENERGY_FULL

# CPPFLAGS += -DMEASURE_TIME_SENSOR
# CPPFLAGS += -DMEASURE_ENERGY_SENSOR

# CPPFLAGS += ENCRYPT_WITH_AES
# CPPFLAGS += -DMEASURE_TIME_AES
# CPPFLAGS += -DMEASURE_ENERGY_AES

# CPPFLAGS += -DMEASURE_TIME_TX
# CPPFLAGS += -DMEASURE_ENERGY_TX


CONTIKI = ../..
include $(CONTIKI)/Makefile.include