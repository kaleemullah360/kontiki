all: chmod server

chmod: 
	sudo chmod 777 /dev/ttyUSB*

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CFLAGS += -DUIP_CONF_IPV6_RPL -DRPL_CONF_STATS=1

CONTIKI=/home/user/contiki-2.6

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6=1 -DWITH_UIP6=1
SMALL = 1

CFLAGS += -DWITH_COAP=13
CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
APPS += er-coap-13
APPS += erbium

include $(CONTIKI)/Makefile.include
