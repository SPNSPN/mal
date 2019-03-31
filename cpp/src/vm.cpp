#include <fstream>
#include <iostream>
#include <stdlib.h>

#ifndef DERXIVM
#ifndef JETCEVM
#error preprocessor macro VMTYPE is not defined.
#endif
#endif

#if defined(DERXIVM)
#include "derxi.h"
#elif defined(JETCEVM)
#include "jetce.h"
#endif


int main (int argc, char **argv)
{
	if (argc < 2)
	{
#if defined(DERXIVM)
		printf("Usage: %s <derxi obj file>\n", argv[0]);
#elif defined(JETCEVM)
		printf("Usage: %s <jetce obj file>\n", argv[0]);
#endif
		exit(31);
	}

	std::ifstream fin(argv[1], std::ios::in | std::ios::binary);

#if defined(DERXIVM)
	derxi::VM vm;
#elif defined(JETCEVM)
	jetce::VM vm;
#endif

	vm.read(fin);
	fin.close();
	vm.gc();

	vm.eval();
	vm.gc();

	std::cout << vm.print() << std::endl;
	vm.initilize();

	return 0;
}
