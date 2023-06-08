#ifndef UART_NS16550_H
#define UART_NS16550_H

#include <types.h>
#include <pthread.h>

// TODO: investigate why linux 6.3 seems to have hard coded irq of 1 for 8250/16550
// because it sets its priority in plic to 0 effectively disabling it while activating
// irq 0x1 instead, this used to be 0xa
#define NS16550_IRQ 0xa

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
