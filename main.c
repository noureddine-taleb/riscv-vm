#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <unistd.h>

/*
 * for uart RX thread
 */
#include <pthread.h>
#include <termios.h>

#include <riscv_helper.h>
#include <soc.h>

struct soc soc;

void init_terminal() {
	struct termios old = {0};

	if (tcgetattr(0, &old) < 0)
		die("tcsetattr");

	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;

	if (tcsetattr(0, TCSANOW, &old) < 0)
		die("tcsetattr ICANON");
}

int getchar() {
	char buf;
	if (read(0, &buf, 1) < 0)
		die("getchar: read");

	return (buf);
}

void *uart_rx_thread(void *p) {
	struct soc *soc = p;
	char x = 0;

	printf("Uart RX Thread running...\n");

	while (1) {
		x = getchar();

		uart_add_rx_char(&soc->ns16550, x);
	}
}

void start_uart_rx_thread(void *p) {
	pthread_t uart_rx_th_id;
	init_terminal();
	pthread_create(&uart_rx_th_id, NULL, uart_rx_thread, p);
}

int main(int argc, char *argv[]) {
	if (argc != 3)
		die("usage: %s <fdt> <kernel>", argv[0]);
	char *fdt = argv[1];
	char *kernel = argv[2];

	soc_init(&soc, fdt, kernel);

#ifndef DEBUG
	start_uart_rx_thread(&soc);
#endif

	soc_run(&soc);
}
