#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <riscv_helper.h>
#include <soc.h>

// #define UART_DEBUG
#ifdef UART_DEBUG
#define UART_DBG(...)                                                          \
  do {                                                                         \
	printf(__VA_ARGS__);                                                       \
  } while (0)
#else
#define UART_DBG(...)                                                          \
  do {                                                                         \
  } while (0)
#endif

#define REG_RX_TX_DIV_LATCH_LO 0
#define REG_IER_LATCH_HI 1

/*** COMMON REGS ***/
/*
 * READ
 */
#define REG_IIR 2
/*
 * WRITE
 */
#define REG_FCR 2
/*
 * READ/WRITE
 */
#define REG_LCR 3
/*
 * READ/WRITE
 */
#define REG_MCR 4
/*
 * READ
 */
#define REG_LSR 5
/*
 * READ
 */
#define REG_MSR 6
/*
 * READ/WRITE
 */
#define REG_SCRATCH 7

#define UART_FIFO_FAIL 0
#define UART_FIFO_SUCCESS 1

void uart_init(struct uart_ns8250 *uart) {
  memset(uart, 0, sizeof(struct uart_ns8250));

  if (pthread_mutex_init(&uart->lock, NULL) != 0) {
	die("uart mutex init failed\n");
  }

  fifo_init(&uart->tx_fifo, uart->tx_fifo_data, UART_NS8250_FIFO_SIZE);
  fifo_init(&uart->rx_fifo, uart->rx_fifo_data, UART_NS8250_FIFO_SIZE);
}

int uart_bus_access(struct uart_ns8250 *uart, privilege_level priv_level,
					bus_access_type access_type, uxlen address, void *value,
					u8 len) {
  (void)priv_level;
  u8 tmp = 0;
  u8 *outval = value;
  u8 val_u8 = 0;

  if (len != 1)
	die("UART WRITE: Only single byte access allowed!\n");

  pthread_mutex_lock(&uart->lock);

  if (access_type == bus_write_access) {
	val_u8 = *(u8 *)value;
	switch (address) {
	case REG_RX_TX_DIV_LATCH_LO:
	  if (!uart->dlab) {
		fifo_in(&uart->tx_fifo, &val_u8, 1);
		uart->tx_empty_ack = 0;
	  }
	  break;
	case REG_IER_LATCH_HI:
	  if (!uart->dlab) {
		uart->irq_enabled_rx = extract8(val_u8, 0, 1);
		uart->irq_enabled_tx = extract8(val_u8, 1, 1);
	  }
	  break;
	case REG_FCR:
	  /*
	   * DMA mode
	   */
	  if (CHECK_BIT(val_u8, 3))
		die("DMA mode not supported!\n");

	  /*
	   * Clear RX fifo
	   */
	  if (CHECK_BIT(val_u8, 1))
		fifo_reset(&uart->rx_fifo);

	  /*
	   * Clear TX fifo
	   */
	  if (CHECK_BIT(val_u8, 2))
		fifo_reset(&uart->tx_fifo);
	  break;
	case REG_LCR:
	  uart->regs[REG_LCR] = val_u8;
	  uart->dlab = extract8(val_u8, 7, 1);

	  UART_DBG("LCR: " PRINTF_FMT "\n", val_u8);

	  if (uart->dlab)
		UART_DBG("dlab activated\n");
	  else
		UART_DBG("dlab deactivated\n");
	  break;
	case REG_MCR:
	  uart->regs[REG_MCR] = val_u8;
	  break;
	case REG_SCRATCH:
	  uart->scratch = val_u8;
	  break;
	default:
	  die("UART-Write Reg " PRINTF_FMT " not supported yet!\n", address);
	}
  } else {
	switch (address) {
	case REG_RX_TX_DIV_LATCH_LO:
	  if (!uart->dlab) {
		fifo_out(&uart->rx_fifo, &tmp, 1);
		*outval = tmp;
	  }
	  break;
	case REG_IER_LATCH_HI:
	  if (!uart->dlab) {
		*outval = ((uart->irq_enabled_rx << 0) |
				   (uart->irq_enabled_tx << 1));
	  }
	  break;
	case REG_IIR:
	   *outval = 1;
		if (uart->irq_enabled_rx && fifo_len(&uart->rx_fifo)) {
	  		*outval = 0b0100;
		} else if (uart->irq_enabled_tx && fifo_is_empty(&uart->tx_fifo) && !uart->tx_empty_ack) {
	  		*outval = 0b0010;
			uart->tx_empty_ack = 1;
		}
	  break;
	case REG_LSR: {
	  u8 data_avail = (!fifo_is_empty(&uart->rx_fifo)) & 1;
	  u8 overrun_err = 0;
	  u8 parity_err = 0;
	  u8 framing_err = 0;
	  u8 brk_sig = 0;
	  u8 thr_empty = (fifo_is_empty(&uart->tx_fifo)) & 1;
	  u8 thr_empty_and_idle = thr_empty;
	  u8 err_data_fifo = 0;

	  *outval = (data_avail << 0 | overrun_err << 1 | parity_err << 2 |
				 framing_err << 3 | brk_sig << 4 | thr_empty << 5 |
				 thr_empty_and_idle << 6 | err_data_fifo << 7);
	} break;
	case REG_LCR:
	  *outval = uart->regs[REG_LCR];
	  break;
	case REG_MSR:
	  *outval = 0xb0;
	  break;
	case REG_MCR:
	  *outval = 0x8;
	  break;
	case REG_SCRATCH:
	  *outval = uart->scratch;
	  break;
	default:
	  die("UART-Read Reg " PRINTF_FMT " not supported yet!\n", address);
	}
  }

  pthread_mutex_unlock(&uart->lock);
  fprintf(stderr, "uart8250 reg=%lu(dlab=%d at=%s val=%d) priv=%d\n", address,
         extract8(uart->regs[REG_LCR], 7, 1), access_type == bus_read_access ? "read" : "write", *outval,
         priv_level);

  return 0;
}

u8 uart_update(void *priv) {
  struct uart_ns8250 *uart = priv;
  int i = 0;
  u8 tmp_char = 0;
  u8 tmp_fifo_len = 0;
  u8 irq_trigger = 0;

  pthread_mutex_lock(&uart->lock);

  if (fifo_is_full(&uart->tx_fifo)) {
	tmp_fifo_len = fifo_len(&uart->tx_fifo);
	for (i = 0; i < tmp_fifo_len; i++) {
	  fifo_out(&uart->tx_fifo, &tmp_char, 1);
	  putchar(tmp_char);
	}
	fflush(stdout);
  }

  if (uart->irq_enabled_rx && fifo_len(&uart->rx_fifo)) {
	irq_trigger = 1;
	fprintf(stderr, "uart8250 data ready\n");

  } else if (uart->irq_enabled_tx && fifo_is_empty(&uart->tx_fifo) && !uart->tx_empty_ack) {
	irq_trigger = 1;
	fprintf(stderr, "uart8250 fifo empty\n");
  }

  pthread_mutex_unlock(&uart->lock);

  return irq_trigger;
}

void uart_add_rx_char(struct uart_ns8250 *uart, u8 x) {
  pthread_mutex_lock(&uart->lock);

  fifo_in(&uart->rx_fifo, &x, 1);

  pthread_mutex_unlock(&uart->lock);
}
