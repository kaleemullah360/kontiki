
[Source](https://github.com/contiki-os/contiki/wiki/Change-mac-or-radio-duty-cycling-protocols "Permalink to Change mac or radio duty cycling protocols · contiki-os/contiki Wiki · GitHub")

# Change mac or radio duty cycling protocols · contiki-os/contiki Wiki · GitHub

In low-power networks, the radio transceiver must be switched off as much as possible to save energy. In Contiki, this is done by the Radio Duty Cycling (RDC) layer. Contiki provides a set of RDC mechanisms, with various properties. The default mechanism is [ContikiMAC][1].

The MAC (Medium Access Control) layer sits on top of the RDC layer. The MAC layer is responsible for avoiding collisions at the radio medium and retransmitting packets if there were a collision. Contiki provides two MAC layers: a CSMA (Carrier Sense Multiple Access) mechanism and a NullMAC mechanism, that does not do any MAC-level processing.

This guide shows how to change RDC and MAC layer in Contiki. We assume you have your Contiki project already set up. If not, see the [**Develop your first application][2]** guide.

##  About MAC Drivers

Contiki provides two MAC drivers, CSMA and NullMAC. CSMA is the default mechanism. The MAC layer receives incoming packets from the RDC layer and uses the RDC layer to transmit packets. If the RDC layer or the radio layer detects a radio collision, the MAC layer may retransmit the packet at a later point in time. The CSMA mechanism is currently the only MAC layer that retransmits packets if a collision is detected.

##  About RDC drivers

Contiki has several RDC drivers. The most commonly used are ContikiMAC, X-MAC, CX-MAC, LPP, and NullRDC. ContikiMAC is the default mechanism that provides a very good power efficiency but is somewhat tailored for the 802.15.4 radio and the CC2420 radio transceiver. X-MAC is an older mechanism that does not provide the same power-efficiency as ContikiMAC but has less stringent timing requirements. CX-MAC (Compatibility X-MAC) is an implementation of X-MAC that has more relaxed timing than the default X-MAC and therefore works on a broader set of radios. LPP (Low-Power Probing) as a receiver-initiated RDC protocol. NullRDC is a "null" RDC layer that never switches the radio off and that therefore can be used for testing or for comparison with the other RDC drivers.

RDC drivers keep the radio off as much as possible and periodically check the radio medium for radio activity. When activity is detected, the radio is kept on to receive the packet. The channel check rate is given in Hz, specifying the number of channel checks per second, and the default channel check rate is 8 Hz. Channel check rates are given in powers of two and typical settings are 2, 4, 8, and 16 Hz.

The transmitted packet must generally be repeated or "strobed" until the receiver turns on to detect it. This increases the power usage of the transmitting node as well as adding radio traffic which will interfere with the communication among other nodes. Some RDCs allow for "phase optimization" to delay strobing the tx packet until just before the receiver is expected to wake. This requires a good time sync between the two nodes; if the clocks differ by 1% the rx wake time will shift through the entire tx phase window every 100 cycles, e.g. 12 seconds at 8 Hz. This would make phase optimization useless when there are more than a few seconds between packets, as the transmitter would have to start sending well in advance of the predicted receiver wake. A clock drift correction might fix this. See [RDC Phase Optimization][3] for more details.

The Contiki RDC drivers are called:

     contikimac_driver
     cxmac_driver
     nullrdc_driver

##  Step 1: Add a project-conf.h file to the Makefile

A Contiki project can have an optional per-project configuration file, called '**project-conf.h'**. This file is, however, not enabled by default. To enable it, we need to add the following line to the project's Makefile:

     CFLAGS += -DPROJECT_CONF_H="project-conf.h"

The full Makefile may now look something like this:

     CONTIKI = /home/user/contiki
     CFLAGS += -DPROJECT_CONF_H="project-conf.h"
     include $(CONTIKI)/Makefile.include

##  Step 2: Create the project-conf.h file

We now need to create the project-conf.h file. This file should be called project-conf.h and reside in the project's directory.

The project-conf.h file may override a number of Contiki configuration options. In this example, we will override the radio duty cycle layer driver.

##  Step 3a: Specify a new RDC channel check rate

We first change the RDC channel check rate. This is done by adding a #define to the project-conf.h file that specifies the channel check rate, in Hz:

    #define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 16

##  Step 3b: Specify a new RDC driver

To specify what RDC driver Contiki should use, add another #define to the project-conf.h file:

    #define NETSTACK_CONF_RDC nullrdc_driver

##  Step 3c: Specify a new MAC driver

To specify what MAC driver Contiki should use, add another #define to the project-conf.h file:

    #define NETSTACK_CONF_MAC nullmac_driver

##  Step 4: Recompile

After having specified the project-conf.h file in the Makefile, it is necessary to clean up existing dependencies with the make clean command:

     make TARGET=sky clean

This is only needed the first time, however. Next, just compile as usual:

     make TARGET=sky

##  Conclusions

Radio Duty Cycling (RDC) and Medium Access Control (MAC) protocols are an important part of the Contiki netstack as they ultimately determine the power consumption of the nodes and their behavior when the network is congested. This guide demonstrates how to change RDC and MAC protocols for a Contiki project by adding a project-conf.h file and adding the necessary configuration statements to it.

[1]: /contiki-os/contiki/wiki/Contikimac
[2]: /contiki-os/contiki/wiki/Develop-your-first-application
[3]: /contiki-os/contiki/wiki/RDC-Phase-optimization
  