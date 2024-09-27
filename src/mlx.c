#include <sys/mman.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *File;
static void *Program;
static size_t Size;

extern int run(void *, ...);

static void usage(FILE *stream)
{
	fprintf(stream, "usage: mlx ADDRESS OFFSET PROGRAM [ARGUMENTS]\n");
}

static int parseAddress(char *argument, uintptr_t *address)
{
	char *terminator;

	errno = 0;
	*address = strtoumax(argument, &terminator, 0);

	if (errno) {
		perror("strtoumax");
		return -1;
	}

	if (*terminator != 0)
		return -1;
	
	return 0;
}

static int map(void *address)
{
	int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
	int flags = MAP_PRIVATE;

	if (address)
		flags |= MAP_FIXED;

	if (fseek(File, 0, SEEK_END) == -1) {
		perror("fseek");
		return -1;
	}

	Size = ftell(File);

	Program = mmap(address, Size, prot, flags, fileno(File), 0);
	if (Program == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	return 0;
}

static void cleanup(void)
{
	if (Program)
		munmap(Program, Size);

	if (File)
		fclose(File);
}

int main(int argc, char *argv[], char **envp)
{
	uintptr_t address;
	uintptr_t offset;

	if (argc < 2) {
		usage(stderr);
		return EXIT_FAILURE;
	}

	if (parseAddress(argv[1], &address) == -1)
		return EXIT_FAILURE;

	if (parseAddress(argv[2], &offset) == -1)
		return EXIT_FAILURE;

	atexit(cleanup);

	File = fopen(argv[3], "r");
	if (File == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	if (map((void *)address) == -1)
		return EXIT_FAILURE;

	return run(Program + offset, argc - 3, argv + 3, envp);
}
