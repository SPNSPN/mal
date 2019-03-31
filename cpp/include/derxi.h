#ifndef __INCLUDE__DERXI_H__
#define __INCLUDE__DERXI_H__

#include "pool.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

/* compile option */
#ifndef TYPECHECK
#define TYPECHECK false
#endif

#ifndef AREACHECK
#define AREACHECK false
#endif

#ifndef ARGSCHECK
#define ARGSCHECK false
#endif
/* compile option end */

namespace derxi
{
	class VM
	{
		private:
			Pool pool;

			Addr stak;
			Addr genv;
			Addr lenv;
			Addr code;
			Addr kont;

			Addr LambdaT;
			Addr SyntaxT;

			Addr make_lambda (Addr, Addr);
			Addr make_syntax (Addr, Addr);

			Addr runexpr (Addr &);

			// (vm s e c k)
			// (vm (nil s) e c k)
			Addr op_nil  ();

			// (vm (y x s) e c k)
			// (vm ((at (x y) e) s) e c k)
			Addr op_push ();

			// (vm s e (n c) k)
			// (vm (n s) e c k)
			Addr op_cnst ();

			// (vm (bool s) e (n c) k)
			// if (isnil(bool)) : (vm s e (nth-cdr n c) k)
			Addr op_cond ();

			// (vm s e (n c) k)
			// (vm s e (nthcdr n c) k)
			Addr op_jump ();

			// (vm s e (n e1 e2 .. en c) k)
			// (vm (((e1 e2 .. en) e) s) e c k)
			Addr op_func ();

			// (vm ((fc fe) s) e c k)
			// (vm (s fe fc ((c e) k)))
			Addr op_aply ();

			// (vm s e c ((dc de) k))
			// (vm s de dc k)
			Addr op_retn ();

			// (vm (d a s) e c k)
			// (vm ((a . d) s) e c k)
			Addr op_cons ();

			// (vm ((a . d) s) e c k)
			// (vm (a s) e c k)
			Addr op_car ();

			// (vm ((a . d) s) e c k)
			// (vm (d s) e c k)
			Addr op_cdr ();

			// (vm (v1 v2 .. vn s) e (n c) k)
			// (vm ((v1 v2 .. vn) s) e c k)
			Addr op_list ();

			// (vm (v1 v2 .. vn s) e (n c) k)
			// (vm ([v1 v2 .. vn] s) e c k)
			Addr op_vect ();

			// (vm (vect s) e c k)
			// (vm (symb s) e c k)
			Addr op_symb ();

			// (vm (vect s) e c k)
			// (vm (strn s) e c k)
			Addr op_strn ();

			// (vm (arc opc s) e c k)
			// (vm (<expr (opc . arc)> s) e c k)
//			Addr op_expr ();

			// (vm (emes s) e (n c) k)
			// (vm (<erro (eid . emes)> s) e c k)
			Addr op_erro ();

			// (vm ((fc fe) s) e c k)
			// (vm s fe (fc c) k)
			Addr op_rcur ();

			// (vm (v x y s) e c k)
			// (vm s e c k) with (set (at (x y) e) v)
			Addr op_setq ();

			// (vm (a b s) e c k)
			// (vm ((calc a b) s) e c k)
			Addr op_addi ();
			Addr op_subi ();
			Addr op_muli ();
			Addr op_divi ();
			Addr op_modi ();

			// (vm (c v s) e c k)
			// (vm s e c k) with (set (car c) v)
			Addr op_rpla ();
			Addr op_rpld ();

			Addr op_eq ();
			Addr op_equal ();
			Addr op_gt ();

			Addr op_def ();
			Addr op_ref ();

			Addr op_push00 ();
			Addr op_push10 ();
			Addr op_push20 ();
			Addr op_push01 ();
			Addr op_push11 ();
			Addr op_push21 ();

		public:
			VM();

			void gc ();

			bool read (std::istream &);
			void eval ();
			std::string print ();

			void initilize ();
			std::string dump ();
	};

	class Compiler
	{
		private:
			Pool pool;
			Addr symtab;

			bool find_co_paren (std::istream &
					, std::stringstream &);
			void growth (Addr, std::stringstream &);

			Addr genargscode (Addr, Addr);
			Addr genapplycode (Addr, Addr, Addr);
			Addr genpushcode (Addr, Addr);

		public:
			Compiler ();

			Addr parse (std::istream &);
			Addr gencode (Addr, Addr);
			Addr write (std::ostream &, Addr);

			std::string print (Addr);
			void clean ();
			void gc (Addr);
	};
}

#endif /* __INCLUDE__DERXI_H__ */
