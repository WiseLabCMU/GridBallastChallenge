/**@file msg.c
 *
 * @author Craig Hesling
 * @date August 29, 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "msg.h"


/* -------------------------------- MSG -------------------------------- */

/**
 * Print a message buffer in hex
 * @param msg The message buffer to send
 * @param file The output file to write the the hex to
 */
void msg_print(const struct msg *msg, FILE *file) {
	size_t index = 0;
	for (; index < msg->len; index++) {
		fprintf(file, "%2.2X ", msg->buf[index]);
	}
}

/**
 * Extract an int32_t from a message buffer
 * @param msg The message buffer
 * @return The int32 in the message buffer or -1 if the
 */
int32_t msg_getint32(const struct msg *msg) {
	assert(msg);
	int32_t result = 0;
	if (msg->len != 4) {
		return -1;
	}
	result |= (int32_t)(((uint32_t)msg->buf[3]) << 3*8);
	result |= (int32_t)(((uint32_t)msg->buf[2]) << 2*8);
	result |= (int32_t)(((uint32_t)msg->buf[1]) << 1*8);
	result |= (int32_t)(((uint32_t)msg->buf[0]) << 0*8);
	return result;
}

/**
 * Set a message buffer to the given int32
 * @param msg The message buffer
 * @param value The int32 value to set the message buffer to
 */
void msg_setint32(struct msg *msg, int32_t value) {
	assert(msg);
	msg->buf[0] = (unsigned char)((value>>0*8) & 0xFF);
	msg->buf[1] = (unsigned char)((value>>1*8) & 0xFF);
	msg->buf[2] = (unsigned char)((value>>2*8) & 0xFF);
	msg->buf[3] = (unsigned char)((value>>3*8) & 0xFF);
	msg->len = 4;

	assert(value == msg_getint32(msg));
}

/**
 * Reset the msg buffer. This has the effect of emptying it.
 * @param msg The message buffer
 */
void msg_reset(struct msg *msg) {
	assert(msg);
	msg->len = 0;
}

/**
 * Return the allocated size of the msg data buffer
 * @param msg The message buffer
 * @return The size of the message buffer
 */
size_t msg_asize(const struct msg *msg) {
	assert(msg);
	return sizeof msg->buf;
}

/**
 * Return the used length of the msg data buffer
 * @param msg The message buffer
 * @return The number of bytes being used in the buffer
 */
size_t msg_len(const struct msg *msg) {
	assert(msg);
	return msg->len;
}

/**
 * Append \a count bytes from \a buf into the \a msg
 * @param msg The message buffer
 * @param buf The buffer to copy from
 * @param count The number of bytes to copy
 * @return true if sufficient space is available, false otherwise
 */
bool msg_append(struct msg *msg, void *buf, size_t count) {
	assert(msg);
	assert(msg_len(msg) <= msg_asize(msg));

	if ((msg_asize(msg) - msg_len(msg)) < count) {
		return false;
	}

	memcpy(msg->buf + msg->len, buf, count);
	msg->len += count;
	return true;
}

/**
 * Append \a byte to \a msg
 * @param msg The message buffer
 * @param byte The byte to append
 * @return true if sufficient space is available, false otherwise
 */
bool msg_appendbyte(struct msg *msg, unsigned char byte) {
	assert(msg);
	assert(msg_len(msg) <= msg_asize(msg));

	if ((msg_asize(msg) - msg_len(msg)) < 1) {
		return false;
	}

	msg->buf[msg->len++] = byte;
	return true;
}

int msg_cmp(const struct msg *msg1, const struct msg *msg2) {
	assert(msg1);
	assert(msg2);
	if((msg1->len-msg2->len) != 0) {
		return msg1->len-msg2->len;
	}
	return memcmp(msg1->buf, msg2->buf, msg1->len);
}

unsigned char msg_checksum(const struct msg *msg) {
	assert(msg);
	unsigned char sum = 0;
	size_t index = 0;
	for (; (msg->len>0) && (index < (msg->len-1)); index++) {
		sum += msg->buf[index];
	}
	return sum;
}

void msg_setpoint(struct msg *msg, unsigned char setpoint) {
	assert(msg);
	// 87 09 03 57 57 41
	unsigned char buffer[6] = {
			0x87, 0x09, 0x03,
			setpoint, setpoint,
			0x00 // checksum
	};
	msg->len = 6;
	memcpy(msg->buf, buffer, msg->len);
	msg->buf[5] = msg_checksum(msg);
}
