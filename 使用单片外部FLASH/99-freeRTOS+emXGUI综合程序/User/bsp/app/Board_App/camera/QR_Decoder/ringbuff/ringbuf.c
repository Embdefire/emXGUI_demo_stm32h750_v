
#include "ringbuf.h"

void ringbuf_init(struct RingBuf *rb, unsigned char *buf, int len)
{
	rb->data = buf;
	rb->size = len;
	rb->ridx = 0;
	rb->widx = 0;
}

int ringbuf_write(struct RingBuf *rb, unsigned char c)
{
	rb->widx++;
	if(rb->widx == rb->size)
		rb->widx = 0;
	if(rb->widx != rb->ridx)
	{
		rb->widx--;
		if(rb->widx == -1)
			rb->widx = rb->size - 1;
		rb->data[rb->widx] = c;
		rb->widx++;
		if(rb->widx == rb->size)
			rb->widx = 0;
	}
	else
	{
		rb->widx--;
		if(rb->widx == -1)
			rb->widx = rb->size - 1;
		return 0;
	}
	return 1;
}

int ringbuf_read(struct RingBuf *rb, unsigned char *c)
{
	if(rb->widx == rb->ridx)
		return 0;
	*c = rb->data[rb->ridx];
	rb->ridx++;
	if(rb->ridx == rb->size)
		rb->ridx = 0;
	return 1;
}

int ringbuf_occupy(struct RingBuf *rb)
{
	int valid = rb->widx - rb->ridx;
	if(valid <= 0)
		valid += rb->size;
	return rb->size - valid;
}
