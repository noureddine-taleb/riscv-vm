#ifndef UART_NS8250_H
#define UART_NS8250_H

#include <types.h>

#include <fifo.h>

#define UART_NS8250_NR_REGS 12
#define UART_NS8250_FIFO_SIZE 16

#define IRQ_PENDING 0
#define NO_IRQ_PENDING 1

struct uart_ns8250 {
	u8 dlab;

	/*
	 * IRQ 
	 */
	u8 irq_enabled_rx_data_available;
	u8 irq_enabled_tx_holding_reg_empty;
	u8 irq_enabled_rlsr_change;
	u8 irq_enabled_msr_change;
	u8 irq_enabled_sleep;
	u8 irq_enabled_low_power;

	u8 tx_holding_reg_empty;
	u8 tx_holding_irq_cleared;

	u8 fifo_enabled;

	struct __fifo tx_fifo;
	u8 tx_fifo_data[UART_NS8250_FIFO_SIZE];
	u8 tx_needs_flush;
	u8 tx_stop_triggering;

	struct __fifo rx_fifo;
	u8 rx_fifo_data[UART_NS8250_FIFO_SIZE];
	u8 rx_irq_fifo_level;
	u8 lsr_change;

	u8 curr_iir_id;

	u8 regs[UART_NS8250_NR_REGS];

	pthread_mutex_t lock;

};

void uart_init(struct uart_ns8250 *uart);
int uart_bus_access(struct uart_ns8250 *uart,
		    privilege_level priv_level,
		    bus_access_type access_type, uxlen address,
		    void *value, u8 len);
u8 uart_update(void *priv);
void uart_add_rx_char(struct uart_ns8250 *uart, u8 x);

#endif /* UART_NS8250_H */
