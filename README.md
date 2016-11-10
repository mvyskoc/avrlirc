README for avrlirc
==================

Avrlirc is Infra-red remote controller receiver for use with
[LIRC](http://www.lirc.org/) software. The receiver is connected to the
computer via common serial port. The comunication with the LIRC provides
host-based daemon *avrlirc2udp* and the default [LIRC UDP
driver](http://www.lirc.org/html/udp.html).  Because of the *Avrlirc* send
information about the length of IR pulses, it can be used with most of the
serial interface (e.g. USB-to-serial convertor).  Unlike the simplest
[Serial port IR receiver](http://www.lirc.org/receivers.html), where the
serial port is used in non-standard way for direct pulse length
measurement.


The idea of this project is taken from Paul Fox,
[foxhapr/avrlirc](https://github.com/foxharp/avrlirc/) with following
differences:

  * The smallest AVR ATTiny25 microcontroller in DIP 8 package is used. In
    the fact any kind of AVR with at least one timer and with crystal
oscillator inputs can be used.
  * The length of IR pulses is measured in multiples of bit rate used for
    serial comunication (i.e. 26 microseconds for used 38400 baud). Foxharp
version uses multiples of 1/16384 s (same what LIRC UDP driver expects).
  * The receiver is powered from DTR, RTS lines of the serial port.
  * To meet previous requirements slightly modified foxharp's *avrlirc2udp*
    daemon is used. Time scaling of IR pulses compatible to LIRC UDP driver
is made in daemon and the DTR, RTS serial lines are set to logic one for power
Avrlirc.

**Remark:**
The Avrlirc implement software UART and because of that can be easily
connected to RS232 serial interface without TTL-RS232 convertor (e.g.
MAX232). But the RS-232 standard for the data transmission defines that
logic one is a positive voltage in the range +3 to +15 volts and the logic
zero is a negative voltage in the range -3 to -15 volts. From that follows
that the range between -3 to +3 volts is not a valid RS-232 level
[[3]](#Links).

Because of simplicity avrlirc use only positive voltage. Fortunately most
of RS-232 accept this voltage levels. 


My story "Why I made avrlirc"
-----------------------------

My reason for making this receiver was the building the HTPC computer for
family of my wife. I wanted to have IR receiver, usb wifi dongle and usb
wireless keyboard/mouse module all hidden in one box and connected to the
internal motherboard headers. Because of that I bought motherboard with
support of serial port.

Firstly I built simple serial IR receiver [1]. Everything looked fine, I
used *irrecord* for the learning remote controller keys. I was even able to
create the configuration file *lircd.conf*. There was a little strange,
that all IR codes were defined only as RAW ! When I wanted to verify learnt
keys, LIRC did not recognise any of them. Than I finally realized, that
something must be wrong with the serial interface. My motherboard has only
one free serial PCIe x1 bus and it has not any paralel PCI bus. So I think
the UART chip on the motherboard is connected to the internal serial bus
PCIe or even USB. But the device on the USB bus cannot immediately notify
CPU about change of the state. It must wait short time until it will be
asked. But because of this delay it is not possible to measure short IR
pulses. I think this problem will be similar in case of the PCIe serial bus
too.

I found [foxharp/avrlirc](https://github.com/foxharp/avrlirc/) project, but
I could not easily build because of absence ATTiny2313 in my country.
Firstly I modified for ATMega8A, it was working but too big. Than I thought
how to do smaller and this is the result.
   
Circuit Diagram and Working Explanation
---------------------------------------
<a href="circuit/avrlirc-01-sch.png"></a>

Links
-----
  1. Simple Home-brew serial port IR receiver, http://www.lirc.org/receivers.html
  2. AVR-based IR interface for lircd (@foxhapr/avrlirc), https://github.com/foxharp/avrlirc/ 
  3. RS-232 (Wikipedia), https://en.wikipedia.org/wiki/RS-232 





