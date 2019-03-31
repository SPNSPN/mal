#ifndef __INCLUDE__JETCE_H__
#define __INCLUDE__JETCE_H__

#include "pool.h"

namespace jetce
{
	class VM
	{
		private:
			Pool pool;

			Addr data;
			Addr genv;
			Addr lenv;
			Addr code;
			Addr kont;

		public:
			VM ();

			void read (std::istream &);
			void eval ();
			void print ();

			std::string dump ();
	};
}

#endif /* __INCLUDE__JETCE_H__ */
