#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "interpreter.h"

void parse_cmd_args (int argc, char **argv
		, bool &readonly
		, std::string &script)
{
	int result;
	while ((result = getopt(argc, argv, "rs:")) != -1)
	{
		switch (result)
		{
			case 'r':
				readonly = true;
				break;
			case 's':
				script = std::string(optarg);
				break;
		}
	}
}

void repl (bool readonly)
{
	Interpreter itp;
	std::string prompt = readonly ? "rdbg> " : "senva> ";

	std::cout << prompt << std::flush;
	for (std::string line; std::getline(std::cin, line); )
	{
		std::stringstream ss;
		ss << line;
		Addr tree = itp.readtop(ss);
		if (not readonly) { tree = itp.eval(tree); }
		std::cout << itp.printtop(tree) << std::endl;
		itp.gc();
		std::cout << prompt << std::flush;
	}
}

void rep_file (const std::string &script, bool readonly)
{
	Interpreter itp;
	std::string prompt = readonly ? "rdbg> " : "senva> ";
	
	std::cout << prompt << std::flush;
	std::fstream fin(script);

	Addr tree = itp.readtop(fin);
	if (not readonly) { tree = itp.eval(tree); }
	std::cout << itp.printtop(tree) << std::endl;
	itp.gc();
}


int main (int argc, char **argv)
{
	bool readonly;
	std::string script;
	parse_cmd_args(argc, argv, readonly, script);


	if (script.empty())
	{
		repl(readonly);
	}
	else
	{
		rep_file(script, readonly);
	}

	return 0;
}
