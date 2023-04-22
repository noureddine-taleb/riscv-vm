#ifndef _FIFO_H_
#define _FIFO_H_

#include <types.h>

struct __fifo {
	unsigned int in;
	unsigned int out;
	unsigned int mask;
	unsigned int size;
	u8 *data;
};

/**
 * fifo_init - initilizes a fifo structure
 * @fifo: address of the fifo to be used
 */
void fifo_init(struct __fifo *fifo, u8 *data_arr, unsigned int data_arr_size);

/**
 * fifo_resets - resets a fifo structure
 * 	requires a protected and exclusive access 
 * @fifo: address of the fifo to be used
 */
void fifo_reset(struct __fifo *fifo);

/**
 * fifo_size - returns the size of the fifo in elements
 * @fifo: address of the fifo to be used
 */
unsigned int fifo_size(struct __fifo *fifo);

/**
 * fifo_capacity - returns the number free space for new elements
 * @fifo: address of the fifo to be used
 */
unsigned int fifo_free(struct __fifo *fifo);

/**
 * fifo_len - returns the number of used elements in the fifo
 * @fifo: address of the fifo to be used
 */
unsigned int fifo_len(struct __fifo *fifo);

/**
 * kfifo_is_empty - returns true if the fifo is empty
 * @fifo: address of the fifo to be used
 */
int fifo_is_empty(struct __fifo *fifo);

/**
 * kfifo_is_full - returns true if the fifo is full
 * @fifo: address of the fifo to be used
 */
int fifo_is_full(struct __fifo *fifo);

/**
 * fifo_put - put data into the fifo
 * @fifo: address of the fifo to be used
 * @val: the data to be added
 *
 * This macro copies the given value into the fifo.
 * It returns 0 if the fifo was full. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
int fifo_put(struct __fifo *fifo, u8 val);

/**
 * fifo_get - get data from the fifo
 * @fifo: address of the fifo to be used
 * @val: the var where to store the data to be added
 *
 * This macro reads the data from the fifo.
 * It returns 0 if the fifo was empty. Otherwise it returns the number
 * processed elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
int fifo_get(struct __fifo *fifo, u8 *val);

/**
 * fifo_in - put data into the fifo
 * @fifo: address of the fifo to be used
 * @buf: the data to be added
 * @n: number of elements to be added
 *
 * This macro copies the given buffer into the fifo and returns the
 * number of copied elements.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
unsigned int fifo_in(struct __fifo *fifo, const u8 *buf, unsigned long n);

/**
 * kfifo_out - get data from the fifo
 * @fifo: address of the fifo to be used
 * @buf: pointer to the storage buffer
 * @n: max. number of elements to get
 *
 * This macro get some data from the fifo and return the numbers of elements
 * copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these macro.
 */
unsigned int fifo_out(struct __fifo *fifo, u8 *buf, unsigned long n);

#endif // _FIFO_H_
