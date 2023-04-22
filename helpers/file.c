#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <file.h>
#include <riscv_helper.h>

size_t get_file_size(char *file)
{
	struct stat s;
	if (stat(file, &s) < 0)
		die("stat");

	return s.st_size;
}

void load_file(char *file, u8 *memory)
{
	int fd = open(file, O_RDONLY);
	if (fd < 0)
		die("open");

	ssize_t size = get_file_size(file);
	ssize_t result = read(fd, memory, size);
	if (result != size)
		die("error reading file = '%s'", file);
	close(fd);
}
