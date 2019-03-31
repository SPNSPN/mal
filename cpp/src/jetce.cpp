#include "jetce.h"

namespace jetce
{
	VM::VM ()
		: pool()
		  , data (Pool::nil)
		  , genv (Pool::nil)
		  , lenv (Pool::nil)
		  , code (Pool::nil)
		  , kont (Pool::nil)
	{
	}

	void VM::read (std::istream &in)
	{
		// TODO
	}

	void VM::eval ()
	{
		// TODO
	}

	void VM::print ()
	{
		// TODO
	}
}
