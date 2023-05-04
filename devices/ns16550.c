#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <riscv_helper.h>
#include <soc.h>

// #define UART_DEBUG
#ifdef UART_DEBUG
#define UART_DBG(...)        \
	do                       \
	{                        \
		printf(__VA_ARGS__); \
	} while (0)
#else
#define UART_DBG(...) \
	do                \
	{                 \
	} while (0)
#endif

#define REG_RX_TX 0
#define REG_IER 1

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

void uart_init(struct ns16550 *uart)
{
	memset(uart, 0, sizeof(struct ns16550));

	if (pthread_mutex_init(&uart->rx_buf.lock, NULL) != 0)
	{
		die("uart mutex init failed\n");
	}

	if (pthread_mutex_init(&uart->tx_buf.lock, NULL) != 0)
	{
		die("uart mutex init failed\n");
	}
}

void uart_put_char(struct ns16550 *uart, u8 c)
{
	pthread_mutex_lock(&uart->tx_buf.lock);
	uart->tx_buf.data = c;
	uart->tx_buf.full = true;
	pthread_mutex_unlock(&uart->tx_buf.lock);
}

u8 uart_get_char(struct ns16550 *uart)
{
	pthread_mutex_lock(&uart->rx_buf.lock);
	u8 data = uart->rx_buf.data;
	uart->rx_buf.full = false;
	pthread_mutex_unlock(&uart->rx_buf.lock);
	return data;
}

bool uart_rx_empty(struct ns16550 *uart)
{
	pthread_mutex_lock(&uart->rx_buf.lock);
	bool b = uart->rx_buf.full;
	pthread_mutex_unlock(&uart->rx_buf.lock);
	return !b;
}

bool uart_tx_empty(struct ns16550 *uart)
{
	pthread_mutex_lock(&uart->tx_buf.lock);
	bool b = uart->tx_buf.full;
	pthread_mutex_unlock(&uart->tx_buf.lock);
	return !b;
}

void uart_insert_char(struct ns16550 *uart, u8 c)
{
	pthread_mutex_lock(&uart->rx_buf.lock);
	uart->rx_buf.data = c;
	uart->rx_buf.full = true;
	pthread_mutex_unlock(&uart->rx_buf.lock);
}

u8 uart_fetch_char(struct ns16550 *uart)
{
	pthread_mutex_lock(&uart->tx_buf.lock);
	u8 data = uart->tx_buf.data;
	uart->tx_buf.full = false;
	pthread_mutex_unlock(&uart->tx_buf.lock);
	return data;
}

void uart_tx_reset(struct ns16550 *uart)
{
	pthread_mutex_lock(&uart->tx_buf.lock);
	uart->tx_buf.full = false;
	pthread_mutex_unlock(&uart->tx_buf.lock);
}

void uart_rx_reset(struct ns16550 *uart)
{
	pthread_mutex_lock(&uart->rx_buf.lock);
	uart->rx_buf.full = false;
	pthread_mutex_unlock(&uart->rx_buf.lock);
}

int uart_bus_access(struct ns16550 *uart, privilege_level priv_level,
					bus_access_type access_type, uxlen address, void *value,
					u8 len)
{
	(void)priv_level;
	u8 *outval = value;
	u8 val_u8 = 0;

	if (len != 1)
		die("UART WRITE: Only single byte access allowed!\n");

	if (access_type == bus_write_access)
	{
		val_u8 = *(u8 *)value;
		switch (address)
		{
		case REG_RX_TX:
			if (!uart->dlab)
			{
				uart_put_char(uart, val_u8);
				uart->tx_empty_ack = 0;
			}
			break;
		case REG_IER:
			if (!uart->dlab)
			{
				uart->irq_enabled_rx = GET_BIT(val_u8, 0);
				uart->irq_enabled_tx = GET_BIT(val_u8, 1);
			}
			break;
		case REG_FCR:
			/*
			 * DMA mode
			 */
			if (GET_BIT(val_u8, 3))
				die("DMA mode not supported!\n");

			/*
			 * Clear RX fifo
			 */
			if (GET_BIT(val_u8, 1))
				uart_rx_reset(uart);

			/*
			 * Clear TX fifo
			 */
			if (GET_BIT(val_u8, 2))
				uart_tx_reset(uart);
			break;
		case REG_LCR:
			uart->lcr = val_u8; // ignored except dlab bit
			uart->dlab = GET_BIT(val_u8, 7);

			UART_DBG("LCR: " PRINTF_FMT "\n", val_u8);

			if (uart->dlab)
				UART_DBG("dlab activated\n");
			else
				UART_DBG("dlab deactivated\n");
			break;
		case REG_MCR:
			uart->mcr = val_u8; // ignored
			break;
		case REG_SCRATCH:
			uart->scratch = val_u8;
			break;
		default:
			die("UART-Write Reg " PRINTF_FMT " not supported yet!\n", address);
		}
	}
	else
	{
		switch (address)
		{
		case REG_RX_TX:
			if (!uart->dlab)
			{
				*outval = uart_get_char(uart);
			}
			break;
		case REG_IER:
			if (!uart->dlab)
			{
				*outval = (uart->irq_enabled_rx << 0) | (uart->irq_enabled_tx << 1);
			}
			break;
		case REG_IIR:
			*outval = 1;
			if (uart->irq_enabled_rx && !uart_rx_empty(uart))
			{
				*outval = 0b0100;
			}
			else if (uart->irq_enabled_tx && uart_tx_empty(uart) && !uart->tx_empty_ack)
			{
				*outval = 0b0010;
				uart->tx_empty_ack = 1;
			}
			break;
		case REG_LSR:
		{
			u8 data_avail = (!uart_rx_empty(uart)) & 1;
			u8 thr_empty = (uart_tx_empty(uart)) & 1;
			u8 thr_empty_and_idle = thr_empty;

			*outval = (data_avail << 0 | thr_empty << 5 |
					   thr_empty_and_idle << 6);
		}
		break;
		case REG_LCR:
			*outval = uart->lcr;
			break;
		case REG_MSR:
			*outval = 0xb0; // Carrier Detect | Data Set Ready | Clear To Send
			break;
		case REG_MCR:
			*outval = 0x8; // set Auxiliary Output 2
			break;
		case REG_SCRATCH:
			*outval = uart->scratch;
			break;
		default:
			die("UART-Read Reg " PRINTF_FMT " not supported yet!\n", address);
		}
	}

	return 0;
}

u8 uart_check_interrupts(void *priv)
{
	struct ns16550 *uart = priv;
	u8 irq_trigger = 0;

	if (!uart_tx_empty(uart))
	{
		putchar(uart_fetch_char(uart));
		fflush(stdout);
	}

	if (uart->irq_enabled_rx && !uart_rx_empty(uart))
	{
		irq_trigger = 1;
	}
	else if (uart->irq_enabled_tx && uart_tx_empty(uart) && !uart->tx_empty_ack)
	{
		irq_trigger = 1;
	}

	return irq_trigger;
}

void uart_add_rx_char(struct ns16550 *uart, u8 x)
{
	uart_insert_char(uart, x);
}
