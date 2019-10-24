
#ifndef _RINGBUF_H_
#define _RINGBUF_H_

typedef struct RingBuf
{
	unsigned char *data;
	int ridx;
	int widx;
	int size;
}RingBuf;

void ringbuf_init(RingBuf *rb, unsigned char *buf, int len);
int ringbuf_write(RingBuf *rb, unsigned char c);
int ringbuf_read(RingBuf *rb, unsigned char *c);
int ringbuf_occupy(struct RingBuf *rb);

#endif//_RINGBUF_H_
