DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: mqtt-z1

CONTIKI_WITH_IPV6 = 1

APPS=serial-shell
APPS += powertrace
APPS += config-radio
APPS += mqtt

CONTIKI=../../../..
include $(CONTIKI)/Makefile.include
