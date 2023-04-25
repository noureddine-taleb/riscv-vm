#ifndef UART_NS8250_H
#define UART_NS8250_H

#include <types.h>

#include <fifo.h>

#define UART_NS8250_FIFO_SIZE 1

#define IRQ_PENDING 0
#define NO_IRQ_PENDING 1

struct uart_ns8250 {
	/*
	 * IRQ flags
	 */
	u8 irq_enabled_rx;
	u8 irq_enabled_tx;

	struct __fifo tx_fifo;
	u8 tx_fifo_data[UART_NS8250_FIFO_SIZE];

	struct __fifo rx_fifo;
	u8 rx_fifo_data[UART_NS8250_FIFO_SIZE];

	u8 scratch;
	u8 dlab;
	u8 tx_empty_ack;
	u8 lcr; // Line Control Register
	u8 mcr; // Modem Control Register

	pthread_mutex_t lock;
};

void uart_init(struct uart_ns8250 *uart);
int uart_bus_access(struct uart_ns8250 *uart, privilege_level priv_level,
										bus_access_type access_type, uxlen address, void *value,
										u8 len);
u8 uart_update(void *priv);
void uart_add_rx_char(struct uart_ns8250 *uart, u8 x);

#endif /* UART_NS8250_H */
