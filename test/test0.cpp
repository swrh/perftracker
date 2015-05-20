#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <iomanip>

#include "perftracker.hpp"

void	dosomething(uint, uint);

#if 1
#define DOSOMETHING(x, y) do { \
		dosomething(__func__, x, y); \
	} while (0)
#else
#define DOSOMETHING(x, y)
#endif

using namespace perftracker;

tracker tracker::instance;

static unsigned long next = 1;

void
dosomething(const char *func, uint x, uint y)
{
	uint		i;
	int		fd;
	char		filename[MAXPATHLEN];
	uint		__us = x * y;

	std::cerr << " " << std::setw(10) << std::left << func
	    << std::right << " " << std::setw(10)
	    << (static_cast<unsigned int>(__us) / 1000) << std::endl;

	i = ::arc4random_uniform(__us);

	while (i--) {
		snprintf(filename, sizeof(filename), "/tmp/.test%0d", i);
		if ((fd = open(filename, O_RDWR | O_CREAT | O_APPEND)) == -1)
			err(1, "open");

		if (fchmod(fd, S_IRWXU | S_IRWXG | S_IRWXO) == -1)
			err(1, "fchmod");

		write(fd, "test\n", sizeof("test"));
		close(fd);
	}
}

unsigned int
myrand(void) {
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 32) % 16;
}

void
test0(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(0, n);
}

void
test1(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(1, n);
}

void
test2(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(2, n);
}

void
test3(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(3, n);
}

void
test4(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(4, n);
}

void
test5(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(5, n);
}

void
test6(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(6, n);
}

void
test7(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(7, n);
}

void
test8(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(8, n);
}

void
test9(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(9, n);
}

void
test10(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(10, n);
}

void
test11(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(11, n);
}

void
test12(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(12, n);
}

void
test13(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(13, n);
}

void
test14(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(14, n);
}

void
test15(unsigned int n)
{
	PT_TRACKPOINT();
	DOSOMETHING(15, n);
}

int
main(void)
{
	typedef void (*fn)(unsigned int);

	fn functions[16];

	functions[0] = test0;
	functions[1] = test1;
	functions[2] = test2;
	functions[3] = test3;
	functions[4] = test4;
	functions[5] = test5;
	functions[6] = test6;
	functions[7] = test7;
	functions[8] = test8;
	functions[9] = test9;
	functions[10] = test10;
	functions[11] = test11;
	functions[12] = test12;
	functions[13] = test13;
	functions[14] = test14;
	functions[15] = test15;

#if 0
	std::list<time_entry> l;
	//printf("std::list::max_size() = %lu; sizeof() = %lu;\n", l.max_size(), sizeof(unsigned long int));
	printf("std::list::max_size() = %u; sizeof() = %u;\n", l.max_size(), sizeof(unsigned long int));

	for (unsigned int n = 0; n < l.max_size(); n++)
		(functions[0])(0);
#endif

	unsigned int max = 1024;
	for (unsigned int n = 0; n < max; n++) {
		std::cerr << " " << std::setw(10) << std::fixed << std::showpoint << std::setprecision(2) << static_cast<double>(n) / (max - 1) * 100 << "% " << " " << std::setw(10) << n << " ";
		(functions[rand() % 16])(arc4random() % 16);
	}

	PT_DUMP();
	PT_SETFILENAME(NULL);

	return EXIT_SUCCESS;
}
