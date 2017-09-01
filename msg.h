/**@file msg.h
 *
 * @author Craig Hesling
 * @date August 29, 2017
 */

#ifndef MSG_H_
#define MSG_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


#define MSG_BUFFER_SIZE 256

/**
 * A message buffer
 */
struct msg {
	size_t        len;
	unsigned char buf[MSG_BUFFER_SIZE];
};


void msg_print(const struct msg *msg, FILE *file);
int32_t msg_getint32(const struct msg *msg);
void msg_setint32(struct msg *msg, int32_t value);
void msg_send(const struct msg *msg, int fd);
void msg_reset(struct msg *msg);
size_t msg_asize(const struct msg *msg);
size_t msg_len(const struct msg *msg);
bool msg_append(struct msg *msg, void *buf, size_t count);
bool msg_appendbyte(struct msg *msg, unsigned char byte);
int msg_cmp(const struct msg *msg1, const struct msg *msg2);

unsigned char msg_checksum(const struct msg *msg);
void msg_setpoint(struct msg *msg, unsigned char setpoint);


#endif /* MSG_H_ */
