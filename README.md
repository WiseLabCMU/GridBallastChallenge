# GridBallastChallenge
This is the UART mark/space parity challenge required for the main GridBallast controller to speak over serial to an electronic thermostat. This exercise will get you familiar with the main processor on the GridBallast board (ESP32) and explore your embedded programming background. For more information about GridBallast, please see the [GridBallast website](https://sites.google.com/view/gridballast/home).

# Plot
[APCOM](http://www.apcom-inc.com) has designed an electronic thermostat(ET) for electric water heaters. These ET modules are used in nearly all the major smart electric water heaters (OA Smith etc). The GridBallast team designed and prototyped a control module with many interesting sensing modalities and communications technologies. We would like our controller module to be able to communicate with the ET to safely retrofit a variety of water heaters.

We need a developer capable of interfacing with many peripherals and sensors. Among these peripherals is the ET mentioned above. From a bit of reverse engineering, we have seen that it makes use of a custom RS485 multi-drop protocol using mark/space parity. You may not have seen this before, but do not be intimidated by the names, we will explain in the next section.

# Challenge
For all intents and purposes, RS485 is simply half-duplex UART sent over a differential pair of wires. All devices send and receive in the same collision domain, thus the need for a medium control protocol. In practice, we speak RS485 by sending UART into a transceiver chip which converts the normal TTL RS232 into a differential signal.

In this challenge, we want you to take the first step in interfacing with the ET. You need to dial in the mark/space parity UART (sometimes called 9bit UART) communication protocol used to communicate with the ET. Since the GridBallast Controller uses an ESP32, we will be providing you with an ESP32 Dev Board, which you will use to communicate to a provided test device/program. The test device is a FTDI USB to UART adapter being driven by a provided command line program. The overall mission is simple. Please write an ESP32 program that takes the two identically signed 32-bit integers sent from the command line program running on a computer to the ESP32 over UART, sum them, and send the result back. You will know that you have succeeded when the program confirms the correct transactions.  This will require that you write a 9bit UART driver on the ESP32 and a simple application that sums two values and returns the result.

# Details
* The UART pattern should be 8 data bits, followed by a single mark or space parity bit, followed by 1 stop bit. The first byte of the packet should be marked (parity bit is 1). The remaining bytes of the packet should have a space parity (parity bit is 0). The baud rate is 19200.
* The provided command line program was compiled for GNU/Linux amd64 and will use an FTDI USB serial adapter.
* ESP32 Hardware may not support 9bit communication natively, so you may need to think outside of the box.

If you can get this to work, then you have achieved the first step of getting the GridBallast device to communicate with the ET and are well on your way.

# The `interrogate` Program
The `interrogate` program will interrogate your device's protocol implementation by sending it two random 32bit values over the USB to serial adapter.
It will then wait for the summation response from your device and determine if it has the proper UART mode, parity bit marking, byte count, loose timing, and summation value.
If these are all correct, it will emit a message that looks like the following:
```
# Success - I sent 671974569 and 1462247110 and received 2134221679
```
Note that lines starting with a `#` are used to indicate normal steps of program operation.

If you did not see the above success message, this means something went wrong.
* Lines starting with `Protocol Error - ` indicate that the parity bit was set incorrectly for one or more of the bytes.
* Lines starting with `Error - ` is used to indicate any other response error.
* Any lines that do not match any of the previous rules indicate an `interrogate` program runtime error that my not be related to your implementation.

The `interrogate` command takes one optional argument that specifies the path of the USB to serial adapter, if it is not `/dev/ttyUSB0`.
For example, you may need to run `./interrogate /dev/ttyUSB1` to interrogate the second USB to serial adapter on your machine.
If you do not have permissions to your USB to serial adapters on your machine, you may run the `interrogate` command with `sudo`.
Please see `interrogate --help` for more details.
