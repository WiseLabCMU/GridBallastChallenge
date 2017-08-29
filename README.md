# GridBallastChallenge
This is the UART mark/space parity challenge for the GridBallast project.
This challenge will exercise your UART and embedded programming skills.
For more information about GridBallast, please see the [GridBallast website](https://sites.google.com/view/gridballast/home).

# Plot
[APCOM](http://www.apcom-inc.com) has designed an electronic thermostat(ET) for electric water heaters. These ET modules are used in nearly all of the major smart electric water heaters.
On our end, we have designed and prototyped a control module with many interesting sensing modalities and communications technologies.
Interfacing the two would be beneficial for safely retrofitting standard water heaters in the immediate future.

We need a developer capable of interfacing with many peripherals and sensors. Among these peripherals is the ET mentioned above. From a bit of reverse engineering, we have seen that it makes use of a custom RS485 multidrop protocol using mark/space parity. Do not be intimidated by the aforementioned names, we will explain in the next section.

# Challenge
For all intents and purposes, RS485 is simply half-duplex UART sent over a differential pair of wires. All devices send and receive in the same collision domain, thus the need for a medium control protocol. In practice, we speak RS485 by sending UART into a transceiver chip which creates the differential signal for us.

In this challenge, you will take the first step in interfacing with the ET. You need to dial in the mark/space parity UART(somethings called 9bit UART) communication protocol used to communicate with the ET. Since the GridBallast Controller uses an ESP32, we will be providing you with an ESP32 Dev Board, which you will use to communicate to a provided test device/program. The test device is a FTDI USB to UART adapter being driven by a provided command line program.
The overall mission is simple. Please take the two identically signed 32-bit integers sent to you over UART, sum them, and send the result back. You will know that you have succeeded when the program says so.

# Details
* The UART pattern should be 8 data bits, followed by a single mark or space parity bit, followed by 1 stop bit. The first byte of the packet should be marked (parity bit is 1). The remaining bytes of the packet should have a space parity(parity bit is 0). The baud rate is 19200.
* The program was compiled for GNU/Linux amd64

# Extra Info
* [Electronic Thermostat](http://www.searspartsdirect.com/part-number/6911175/0042/159.html?modelNumber=ES40R123-45D&categoryName=Water%20heater,%20Electric&brandName=WHIRLPOOL)
* [Electronic Thermostat Manual](https://www.manualslib.com/manual/194299/Whirlpool-Energy-Smart-188410.html?page=12)
* [GridBallast Controller](https://github.com/WiseLabCMU/gridballast)
