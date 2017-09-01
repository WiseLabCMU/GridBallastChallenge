/**@file interrogate.c
 *
 * @author Craig Hesling
 * @date August 29, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>
#include <stdint.h>

#include "msg.h"

/* Baudrate settings are defined in <asm/termbits.h>, which is included by <termios.h> */
#define BAUDRATE B19200

#define FTDI_SETUP_UDELAY    (100 * 1000)
#define TEST_TIMEOUT_SECONDS 2

/* change this definition for the correct port */
#define DEFAULT_MODEM_DEVICE "/dev/ttyUSB0"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define URANDOM_DEVICE "/dev/urandom"


/**
 * Sets the parity bit to mark for future bytes written
 * @param fd The file descriptor associated wit the UART interface
 * @return 0 on success, -1 on error
 */
int setmark(int fd) {
	int status;
	struct termios tio;
	status = tcgetattr(fd, &tio);
	if (status < 0) {
		return status;
	}
	tio.c_cflag |= CMSPAR;
	tio.c_cflag |= PARODD;
	status = tcsetattr(fd, TCSADRAIN, &tio);
	usleep(FTDI_SETUP_UDELAY);
	return status;
}

/**
 * Sets the parity bit to space for future bytes written
 * @param fd The file descriptor associated wit the UART interface
 * @return 0 on success, -1 on error
 */
int setspace(int fd) {
	int status;
	struct termios tio;
	status = tcgetattr(fd, &tio);
	if (status < 0) {
		return status;
	}
	tio.c_cflag |= CMSPAR;
	tio.c_cflag &= ~PARODD;
	status = tcsetattr(fd, TCSADRAIN, &tio);
	usleep(FTDI_SETUP_UDELAY);
	return status;
}




const struct msg msg_poll_slave = {
		.len = 3,
		.buf = {0x87, 0x00, 0x87}
};

const struct msg msg_slave_ok = {
		.len = 5,
		.buf = {0x07, 0x01, 0x03, 0x04, 0x0F}
};


/**
 * Send a message buffer on the open file descriptor \a fd
 * @param msg The message buffer to send
 * @param fd The open file descriptor to send on
 */
void msg_send(const struct msg *msg, int fd) {
	if (setmark(fd) < 0) {
		perror("Failed to set mark");
		exit(EXIT_FAILURE);
	}

	ssize_t n = write(fd, msg->buf, 1);
	if (n != 1) {
		perror("Failed to write first byte of message to fd");
		exit(EXIT_FAILURE);
	}
	usleep(FTDI_SETUP_UDELAY);

	if (setspace(fd) < 0) {
		perror("Failed to set space");
		exit(EXIT_FAILURE);
	}

	if (msg->len > 1) {
		ssize_t n = write(fd, msg->buf + 1, msg->len-1);
		if (n != (msg->len-1)) {
			perror("Failed to write the remaining bytes of message to fd");
			exit(EXIT_FAILURE);
		}
	}
	usleep(FTDI_SETUP_UDELAY);
}

/**
 * This function is intended to only handle sigalarms
 * @param signo This should be the signal number for SIGALRM
 */
void alarmhandler(int signo) {
	assert(signo == SIGALRM);
	fprintf(stderr, "# Error - You have exceeded the time limit of %u seconds\n", TEST_TIMEOUT_SECONDS);
	exit(EXIT_FAILURE);
}

void printusage() {
	printf("Usage: interrogate [devname]\n");
	printf("This program uses the Linux termios interface to interrogate the devname serial device.\n");
	printf("It's purpose is to determine if the attached serial device can speak a special mark/space serial protocol.\n");
	printf("This is done by sending two random 32 bit value to the device and receiving them summation of those values.\n");
	printf("This program will output a \"# Success -\" message if the device passed the challenge and an \"Error -\" message is the device did not pass.\n");
	printf("\n");
	printf("devname - The path to the serial modem to interrogate. [/dev/ttyUSB0]\n");
}

int main(int argc, char *argv[]) {
	int urandfd, fd;
	struct termios newtio;
	char *devname = DEFAULT_MODEM_DEVICE;
	fd_set fdset;
	int32_t operand1, operand2, sum;
	struct msg sendbuf, currmsg;
	unsigned char buf[sizeof(int32_t)];

	const unsigned char special_esc   = 0xFF;
	const unsigned char special_valid = 0xFF;
	const unsigned char special_err   = 0x00;

	int index;

	// Check all parameters for -h or --help
	for (index = 1; index < argc; index++) {
		if (strcmp(argv[index], "--help")==0 || strcmp(argv[index], "-h")==0) {
			// Print Usage and Exit
			printusage();
			exit(EXIT_SUCCESS);
		}
	}

	if (argc > 2) {
		// Print usage and rage quit if they gave too many args
		printusage();
		exit(EXIT_FAILURE);
	} else if (argc > 1) {
		// Accept their specified device
		devname = argv[1];
	}

	/* Get true-er random values for operand1 and operand2 */
	urandfd = open(URANDOM_DEVICE, O_RDWR);
	if (urandfd < 0) {
		perror("Failed to open urandom device");
		exit(EXIT_FAILURE);
	}
	// Fetch operand1's random value
	if (read(urandfd, &operand1, sizeof operand1) != (sizeof operand1)) {
		perror("Failed to read random value for operand1");
		exit(EXIT_FAILURE);
	}
	// Fetch operand2's random value
	if (read(urandfd, &operand2, sizeof operand2) != (sizeof operand2)) {
		perror("Failed to read random value for operand2");
		exit(EXIT_FAILURE);
	}
	if (close(urandfd) < 0) {
		perror("Failed to close urandom device");
		exit(EXIT_FAILURE);
	}

	// Sum the operands
	sum = operand1 + operand2;

	printf("# Interrogating serial device %s\n", devname);

	/*
	 Open modem device for reading and writing and not as controlling tty
	 because we don't want to get killed if linenoise sends CTRL-C.
	 */
	fd = open(devname, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror("Failed to open serial device");
		exit(EXIT_FAILURE);
	}

	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */


	/* c_cflags - Control Modes
       PARENB Enable parity generation on output and parity checking for
              input.

       PARODD If set, then parity for input and output is odd; otherwise
              even parity is used.

       CMSPAR (not in POSIX) Use "stick" (mark/space) parity (supported on
              certain serial devices): if PARODD is set, the parity bit is
              always 1; if PARODD is not set, then the parity bit is always
              0.  [requires _BSD_SOURCE or _SVID_SOURCE]
	 */
	/*
	 BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	 CRTSCTS : output hardware flow control (only used if the cable has
	 all necessary lines. See sect. 7 of Serial-HOWTO)
	 CS8     : 8n1 (8bit,no parity,1 stopbit)
	 CLOCAL  : local connection, no modem contol (aka ignore modem control lines)
	 CREAD   : enable receiving characters
	 */
//	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_cflag |= PARENB;
//	newtio.c_cflag |= PARODD;
	newtio.c_cflag |= CMSPAR;


	/* c_iflags - Input Modes
	   IGNPAR Ignore framing errors and parity errors.

       PARMRK If this bit is set, input bytes with parity or framing errors
              are marked when passed to the program.  This bit is meaningful
              only when INPCK is set and IGNPAR is not set.  The way
              erroneous bytes are marked is with two preceding bytes, \377
              and \0.  Thus, the program actually reads three bytes for one
              erroneous byte received from the terminal.  If a valid byte
              has the value \377, and ISTRIP (see below) is not set, the
              program might confuse it with the prefix that marks a parity
              error.  Therefore, a valid byte \377 is passed to the program
              as two bytes, \377 \377, in this case.

              If neither IGNPAR nor PARMRK is set, read a character with a
              parity error or framing error as \0.

       INPCK  Enable input parity checking.
	 */
	/*
	 IGNPAR  : ignore bytes with parity errors
	 ICRNL   : map CR to NL (otherwise a CR input on the other computer
	 will not terminate input)
	 otherwise make device raw (no other input processing)
	*/
//	newtio.c_iflag = IGNPAR | ICRNL;
	newtio.c_iflag = 0;
//	newtio.c_iflag |= IGNPAR;
	newtio.c_iflag |= PARMRK;
	newtio.c_iflag |= INPCK;

	/*
	 Raw output.
	 */
	newtio.c_oflag = 0;

	/*
	 ICANON  : enable canonical input
	 disable all echo functionality, and don't send signals to calling program
	 */
//	newtio.c_lflag = ICANON;
	newtio.c_lflag = 0;

	/*
	 now clean the modem line and activate the settings for the port
	 */
//	tcflush(fd, TCIFLUSH);
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSADRAIN, &newtio);
	tcflush(fd, TCIOFLUSH);

	/* Setup the fdset to wait on */
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	/* Set the timeout and Send the two operands to add */
	printf("# Sending operand1(%d) and operand2(%d)\n", operand1, operand2);
	if (signal(SIGALRM, alarmhandler) == SIG_ERR) {
		fprintf(stderr, "\n\nFailed to setup SIGALRM handler\n");
		exit(EXIT_FAILURE);
	}
	alarm(TEST_TIMEOUT_SECONDS);
	msg_setint32(&sendbuf, operand1);
	msg_send(&sendbuf, fd);
	msg_setint32(&sendbuf, operand2);
	msg_send(&sendbuf, fd);


	// Reset current message
	msg_reset(&currmsg);

	bool markedfirstbyte = false;
	bool escaped = false;
	while (msg_len(&currmsg) != sizeof(int32_t)) {
		select(FD_SETSIZE, &fdset, NULL, NULL, NULL);
		ssize_t n = read(fd, buf, sizeof buf);
		if (n < 0) {
			perror("Failed to read from fd");
			exit(EXIT_FAILURE);
		}

		int i;
		for (i = 0; i < n; i++) {
			if (escaped) {
				if (buf[i] == special_err) {
					// Parity error - Start of new message

					/* Check that we have never seen the marked byte before */
					if (markedfirstbyte) {
						fprintf(stderr, "Protocol Error - Received invalid parity for non-first byte\n");
						exit(EXIT_FAILURE);
					}
					markedfirstbyte = true;

					// Next byte is the parity violating byte
					escaped = false;

					continue;
				} else if (buf[i] == special_valid) {
					// Normal 0xFF character - Do nothing
					escaped = false;
				} else {
					fprintf(stderr, "Got bad escape code 0x%2.2X\n", buf[i]);
					exit(EXIT_FAILURE);
				}
			} else if (buf[i] == special_esc) {
				escaped = true;
				// throw away escape character
				continue;
			}

			/* Check that we have seen the marked byte before */
			if (!markedfirstbyte) {
				fprintf(stderr, "Protocol Error - Received invalid parity for first byte\n");
				exit(EXIT_FAILURE);
			}

			msg_appendbyte(&currmsg, buf[i]);

			/* Check that we have only received the */
			if (msg_len(&currmsg) > sizeof(int32_t)) {
				fprintf(stderr, "Error - Too many bytes have been received\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	int32_t givenresult = msg_getint32(&currmsg);
	if (sum != givenresult) {
		fprintf(stderr, "Error - Given summation result(%d = 0x%X) does not match internally calculated result(%d = 0x%X)\n", givenresult, givenresult, sum, sum);
		exit(EXIT_FAILURE);
	}

	printf("# Success - I sent %d and %d and received %d\n", operand1, operand2, givenresult);
	exit(EXIT_SUCCESS);
}

