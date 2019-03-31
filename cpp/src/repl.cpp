#include <iostream>
#include "interpreter.h"

int main (int argc, char **argv)
{
	bool readdbg = false;
	if (argc > 1)
	{
		if (0 == strcmp("-r", argv[1])) { readdbg = true; }
	}
	Interpreter itp;
	std::cout << "mal> " << std::flush;
	for (std::string line; std::getline(std::cin, line); )
	{
		std::stringstream ss;
		ss << line;
		Addr tree = itp.readtop(ss);
		if (not readdbg) { tree = itp.eval(tree); }
		std::cout << itp.printtop(tree) << std::endl;
		itp.gc();
		std::cout << "mal> " << std::flush;
	}
	return 0;
}
