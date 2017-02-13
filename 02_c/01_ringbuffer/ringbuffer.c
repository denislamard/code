#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uint8_t;
typedef uint8_t bool;

#define	true		1
#define false		0

#define BUFFER_SIZE		8

typedef struct
{
	uint8_t buf[BUFFER_SIZE];
	uint8_t head;
	uint8_t tail;
	uint8_t count;
} ringbuffer_t;


void ring_init(ringbuffer_t* buf);
bool ring_empty(ringbuffer_t* buf);
bool ring_full(ringbuffer_t* buf);
uint8_t ring_size(ringbuffer_t* buf);
void ring_put(ringbuffer_t* buf, uint8_t value);
uint8_t ring_get(ringbuffer_t* buf);
void ringbuffer_flush(ringbuffer_t* buf, const bool clearbuffer);


void ring_init(ringbuffer_t* buf)
{
	memset(buf->buf, 0, sizeof(*buf));
}

bool ring_empty(ringbuffer_t* buf)
{
	return (0 == buf->count);
}

bool ring_full(ringbuffer_t* buf)
{
	return buf->count >= BUFFER_SIZE;
}

uint8_t ring_size(ringbuffer_t* buf)
{
	return buf->count;
}

void ring_put(ringbuffer_t* buf, uint8_t value)
{
	if (buf->count < BUFFER_SIZE)
	{
		buf->buf[buf->head] = value;
		buf->head++;
		if (buf->head >= BUFFER_SIZE)
			buf->head = 0;
		++buf->count;
	}
}

uint8_t ring_get(ringbuffer_t* buf)
{
	uint8_t c;

	if (buf->count > 0)
	{
		c = buf->buf[buf->tail];
		buf->buf[buf->tail] = 0;
		buf->tail++;
		if (buf->tail >= BUFFER_SIZE)
			buf->tail = 0;
		--buf->count;
	}
	return c;
}

void ringbuffer_flush(ringbuffer_t* buf, const bool clearbuffer)
{
	if (clearbuffer == true)
		memset(buf, 0, sizeof(ringbuffer_t));
	else
	{
		buf->count = 0;
		buf->head = 0;
		buf->tail = 0;
	}
}

ringbuffer_t ring;

void show_ring(ringbuffer_t* buf)
{
	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		printf("%d\t", buf->buf[i]);
	}
	printf("\r\n");
}

int main(void)
{
	show_ring(&ring);
	ring_put(&ring, 100);
	show_ring(&ring);
	ring_put(&ring, 101);
	show_ring(&ring);
	ring_put(&ring, 102);
	show_ring(&ring);
	ring_put(&ring, 103);
	show_ring(&ring);
	ring_put(&ring, 104);
	show_ring(&ring);
	ring_put(&ring, 105);
	show_ring(&ring);
	ring_put(&ring, 106);
	show_ring(&ring);
	ring_put(&ring, 107);
	show_ring(&ring);
	ring_put(&ring, 108);
	show_ring(&ring);

	ring_get(&ring);
	show_ring(&ring);

	ring_put(&ring, 109);
	show_ring(&ring);

	ring_get(&ring);
	show_ring(&ring);
	ring_get(&ring);
	show_ring(&ring);

	printf("%d\r\n", ring_size(&ring));


	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
