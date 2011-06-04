#include <err.h>
#include <stdlib.h>

#include <iostream>

#include <perftracker.hpp>

using namespace perftracker;

tracker tracker::instance;

void
test0(unsigned int nsecs)
{
	PT_TRACKPOINT();

	std::cerr << "sleep(" << nsecs << ");" << std::endl;
	usleep(nsecs * 100000);
}

int
main(int argc, char *argv[])
{
	if (argc != 1)
		errx(EXIT_FAILURE, "invalid number of arguments");

	test0(0);
	test0(1);
	test0(2);
	test0(3);
	test0(4);
	test0(5);
	test0(6);

	return EXIT_SUCCESS;
}
