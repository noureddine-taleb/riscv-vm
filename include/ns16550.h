#ifndef UART_NS16550_H
#define UART_NS16550_H

#include <types.h>

#include <stdbool.h>
#include <pthread.h>

#define UART_NS16550_FIFO_SIZE 1

#define IRQ_PENDING 0
#define NO_IRQ_PENDING 1

struct uart_buffer
{
	bool full;
	u8 data;

	pthread_mutex_t lock;
};

struct ns16550
{
	/*
	 * IRQ flags
	 */
	u8 irq_enabled_rx;
	u8 irq_enabled_tx;

	struct uart_buffer rx_buf;
	struct uart_buffer tx_buf;

	u8 scratch;
	u8 dlab;
	u8 tx_empty_ack;
	u8 lcr; // Line Control Register
	u8 mcr; // Modem Control Register
};

void uart_init(struct ns16550 *uart);
int uart_bus_access(struct ns16550 *uart, privilege_level priv_level,
					bus_access_type access_type, uxlen address, void *value,
					u8 len);
u8 uart_check_interrupts(void *priv);
void uart_add_rx_char(struct ns16550 *uart, u8 x);

#endif /* UART_NS16550_H */
