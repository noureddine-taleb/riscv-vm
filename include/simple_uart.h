#ifndef SIMPLE_UART_H
#define SIMPLE_UART_H

#include <types.h>
#include <pthread.h>

#include <fifo.h>

#include <types.h>

#define SIMPLE_UART_FIFO_SIZE 1

struct simple_uart {
	u8 rx_triggered;
	struct __fifo rx_fifo;
	u8 rx_fifo_data[SIMPLE_UART_FIFO_SIZE];
	u8 rx_irq_enabled;

	u8 tx_triggered;
	struct __fifo tx_fifo;
	u8 tx_fifo_data[SIMPLE_UART_FIFO_SIZE];
	u8 tx_irq_enabled;
	u8 tx_needs_flush;

	pthread_mutex_t lock;

};

void simple_uart_init(struct simple_uart *uart);
int simple_uart_bus_access(struct simple_uart *uart,
			   privilege_level priv_level,
			   bus_access_type access_type,
			   uxlen address, void *value, u8 len);
u8 simple_uart_check_irq(struct simple_uart *uart);
void simple_uart_add_rx_char(struct simple_uart *uart, u8 x);

#endif /* UART_NS8250_H */
