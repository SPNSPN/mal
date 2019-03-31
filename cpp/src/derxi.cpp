#include "derxi.h"

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <utility>

namespace derxi
{
	VM::VM ()
		: pool()
		  , stak (Pool::nil)
		  , genv (Pool::nil)
		  , lenv (Pool::nil)
		  , code (Pool::nil)
		  , kont (Pool::nil)
		  , LambdaT (Pool::nil)
		  , SyntaxT (Pool::nil)
	{
		initilize();
	}

	void VM::gc ()
	{
		pool.mark(stak);
		pool.mark(genv);
		pool.mark(lenv);
		pool.mark(code);
		pool.mark(kont);
		pool.sweep();
	}

	bool VM::read (std::istream &in)
	{
		if (not in) { return false; }
		Byte opc;
		code = pool.make_cons(Pool::nil, Pool::nil);
		Addr codehead = code;

		while (not in.eof())
		{
			in.read((char *)&opc, sizeof(opc));
			if (in.eof()) { break; }

			pool.setcdr(codehead, pool.make_cons(
						pool.make_inum((Fixnum)opc), Pool::nil));
			codehead = pool.getcdr(codehead);
		}

		pool.popcons(code);
		return true;
	}

	void VM::eval ()
	{
		while (not pool.isnil(code))
		{
			Addr erro = runexpr(code);
			if (not pool.isnil(erro))
			{
				pool.pushcons(stak, erro);
				return;
			}
		}
	}

	std::string VM::print ()
	{
		return pool.print(stak);
	}

	void VM::initilize ()
	{
		stak = Pool::nil;
		genv = Pool::nil;
		lenv = Pool::nil;
		code = Pool::nil;
		kont = Pool::nil;
		pool.mark(Pool::nil);
		pool.sweep();

		LambdaT = pool.make_symb("lambda");
		SyntaxT = pool.make_symb("syntax");
		pool.pushcons(genv, pool.make_cons(LambdaT, LambdaT));
		pool.pushcons(genv, pool.make_cons(SyntaxT, SyntaxT));

		// (define if)
		Addr lcode = pool.make_queu(); // TODO
		// push00
		pool.pushqueu(lcode, pool.make_inum(0x24));
		// nil
		pool.pushqueu(lcode, pool.make_inum(0x00));
		// aply
		pool.pushqueu(lcode, pool.make_inum(0x06));
		// cond 8
		pool.pushqueu(lcode, pool.make_inum(0x03));
		pool.pushqueu(lcode, pool.make_inum(0x08));
		// push10
		pool.pushqueu(lcode, pool.make_inum(0x25));
		// nil
		pool.pushqueu(lcode, pool.make_inum(0x00));
		// aply
		pool.pushqueu(lcode, pool.make_inum(0x06));
		// retn
		pool.pushqueu(lcode, pool.make_inum(0x07));
		// push20
		pool.pushqueu(lcode, pool.make_inum(0x26));
		// nil
		pool.pushqueu(lcode, pool.make_inum(0x00));
		// aply
		pool.pushqueu(lcode, pool.make_inum(0x06));
		// retn
		pool.pushqueu(lcode, pool.make_inum(0x07));

		pool.pushcons(genv
				, pool.make_cons(pool.make_symb("if")
					, make_syntax(pool.getexit(lcode), lenv)));

		// (define quote) TODO
		lcode = pool.make_queu();

		pool.pushcons(genv
				, pool.make_cons(pool.make_symb("quote")
					, make_syntax(pool.getexit(lcode), lenv)));
		pool.pushcons(genv
				, pool.make_cons(pool.make_symb("define")
					, make_syntax(pool.getexit(lcode), lenv)));
		pool.pushcons(genv
				, pool.make_cons(pool.make_symb("lambda")
					, make_syntax(pool.getexit(lcode), lenv)));
	}

	std::string VM::dump ()
	{
		return std::string("\npool: \n")
			+ pool.dump()
			+ std::string("\nstak: ")
			+ pool.print(stak)
			+ std::string("\ngenv: ")
			+ pool.print(genv)
			+ std::string("\nlenv: ")
			+ pool.print(lenv)
			+ std::string("\ncode: ")
			+ pool.print(code)
			+ std::string("\nkont: ")
			+ pool.print(kont);
	}

	Addr VM::make_lambda (Addr body, Addr env)
	{
		return pool.make_cons(LambdaT
				, pool.make_cons(body, env));
	}

	Addr VM::make_syntax (Addr body, Addr env)
	{
		return pool.make_cons(SyntaxT
				, pool.make_cons(body, env));
	}

	Addr VM::runexpr (Addr &code)
	{
		Fixnum op = pool.getnum(pool.popcons(code));

//		std::cout << "debug: op: " << op << std::endl;
//		std::cout << "stak: " << pool.print(stak) << std::endl;
//		std::cout << "genv: " << pool.print(genv) << std::endl;
//		std::cout << "lenv: " << pool.print(lenv) << std::endl;
//		std::cout << "code: " << pool.print(code) << std::endl;

		switch (op)
		{
			case 0x00:
				return op_nil();
			case 0x01:
				return op_push();
			case 0x02:
				return op_cnst();
			case 0x03:
				return op_cond();
			case 0x04:
				return op_jump();
			case 0x05:
				return op_func();
			case 0x06:
				return op_aply();
			case 0x07:
				return op_retn();
			case 0x08:
				return op_cons();
			case 0x09:
				return op_car();
			case 0x0a:
				return op_cdr();
			case 0x0b:
				return op_list();
			case 0x0c:
				return op_vect();
			case 0x0d:
				return op_symb();
			case 0x0e:
				return op_strn();
			case 0x0f:
				return op_erro();
			case 0x10:
				return op_rcur();
			case 0x11:
				return op_setq();
			case 0x12:
				return op_addi();
			case 0x13:
				return op_subi();
			case 0x14:
				return op_muli();
			case 0x15:
				return op_divi();
			case 0x16:
				return op_modi();
			case 0x17:
				return op_rpla();
			case 0x18:
				return op_rpld();
			case 0x19:
				return op_eq();
			case 0x20: // TODO -> 1a
				return op_equal();
			case 0x21: // TODO -> 1b
				return op_gt();
			case 0x22:
				return op_def();
			case 0x23:
				return op_ref();
			case 0x24:
				return op_push00();
			case 0x25:
				return op_push10();
			case 0x26:
				return op_push20();
			case 0x27:
				return op_push01();
			case 0x28:
				return op_push11();
			case 0x29:
				return op_push21();
			default:
				return pool.make_erro(ErroId::UnknownOpcode
						, pool.make_strn(pool.print(op)
						+ std::string(" is undefined opcode.")));
		}
	}

	Addr VM::op_nil ()
	{
		pool.pushcons(stak, Pool::nil);
		return Pool::nil;
	}

	Addr VM::op_push ()
	{
		Addr y = pool.popcons(stak);
		Addr x = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(x)
				or Pool::InumT != pool.type(y))
		{
			return pool.make_erro(ErroId::Type
					, pool.make_strn(
						std::string(
							"op_push required (InumT InumT) but got: ")
					+ pool.print(x)
					+ std::string(" ")
					+ pool.print(y)));
		}
#endif
#if AREACHECK
		if ((Addr)pool.getnum(y) >= pool.lengthcons(lenv)
				or (Addr)pool.getnum(x)
				>= pool.lengthcons(
					pool.getnth(lenv, pool.getnum(y))))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(")
						+ pool.print(x)
						+ std::string(" . ")
						+ pool.print(y)
						+ std::string(") to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak, pool.getnth(
					pool.getnth(lenv, pool.getnum(y))
					, pool.getnum(x)));
		return Pool::nil;
	}

	Addr VM::op_cnst ()
	{
		pool.pushcons(stak, pool.popcons(code));
		return Pool::nil;
	}

	Addr VM::op_cond ()
	{
		Addr boolean = pool.popcons(stak);
		Fixnum n = pool.getnum(pool.popcons(code));
		if (pool.isnil(boolean))
		{
			for (Fixnum idx = n; idx > 0; --idx)
			{
				pool.popcons(code);
			}
		}
		return Pool::nil;
	}

	Addr VM::op_jump ()
	{
		for (Fixnum i = pool.getnum(pool.popcons(code)); i > 0
				; --i)
		{
			pool.popcons(code);
		}
		return Pool::nil;
	}

	Addr VM::op_func ()
	{
		Addr fnq = pool.make_queu();
		for (Addr idx = pool.getnum(pool.popcons(code))
				; idx > 0; --idx)
		{
			pool.pushqueu(fnq, pool.popcons(code));
		}
		pool.pushcons(stak
				, make_lambda(pool.getexit(fnq), lenv));
		return Pool::nil;
	}

	Addr VM::op_aply ()
	{
		Addr args = pool.popcons(stak);
		Addr lambda = pool.popcons(stak);
#if TYPECHECK
		if (Pool::ConsT != pool.type(lambda)
				or (Pool::ConsT != pool.type(args)
					and not pool.isnil(args)))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_aply required (ConsT ConsT) but got: ")
					+ pool.print(lambda) + std::string(" ") + pool.print(args)));
		}
#endif
		pool.pushcons(kont, pool.make_cons(code, lenv));
		code = pool.getcar(pool.getcdr(lambda));
		lenv = pool.make_cons(args
				, pool.getcdr(pool.getcdr(lambda)));
		return Pool::nil;
	}

	Addr VM::op_retn ()
	{
		Addr cc = pool.popcons(kont);
		code = pool.getcar(cc);
		lenv = pool.getcdr(cc);
		return Pool::nil;
	}

	Addr VM::op_cons ()
	{
		Addr d = pool.popcons(stak);
		Addr a = pool.popcons(stak);
		pool.pushcons(stak, pool.make_cons(a, d));
		return Pool::nil;
	}

	Addr VM::op_car ()
	{
		Addr cons = pool.popcons(stak);
#if TYPECHECK
		if (Pool::ConsT != pool.type(cons))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_car required (ConsT) but got: ")
					+ pool.print(cons)));
		}
#endif
		pool.pushcons(stak, pool.getcar(cons));
		return Pool::nil;
	}

	Addr VM::op_cdr ()
	{
		Addr cons = pool.popcons(stak);
#if TYPECHECK
		if (Pool::ConsT != pool.type(cons))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_cdr required (ConsT) but got: ")
					+ pool.print(cons)));
		}
#endif
		pool.pushcons(stak, pool.getcdr(cons));
		return Pool::nil;
	}

	Addr VM::op_list ()
	{
		Addr c = Pool::nil;
		for (Fixnum idx = pool.getnum(pool.popcons(code))
				; idx > 0; --idx)
		{
			pool.pushcons(c, pool.popcons(stak));
		}
		pool.pushcons(stak, c);
		return Pool::nil;
	}

	Addr VM::op_vect ()
	{
		Fixnum vsize = pool.getnum(pool.popcons(code));
		Addr v = pool.make_vect(vsize);
		for (Fixnum idx = vsize - 1; idx >= 0; --idx)
		{
			pool.setatvect(v, idx, pool.popcons(stak));
		}
		pool.pushcons(stak, v);
		return Pool::nil;
	}

	Addr VM::op_symb ()
	{
		Addr vstr = pool.popcons(stak);
#if TYPECHECK
		if (Pool::VectT != pool.type(vstr))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_symb required (VectT) but got: ")
					+ pool.print(vstr)));
		}
#endif
		pool.pushcons(stak, pool.make_symb(vstr));
		return Pool::nil;
	}

	Addr VM::op_strn ()
	{
		Addr vstr = pool.popcons(stak);
#if TYPECHECK
		if (Pool::VectT != pool.type(vstr))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_strn required (VectT) but got: ")
					+ pool.print(vstr)));
		}
#endif
		pool.pushcons(stak, pool.make_strn(vstr));
		return Pool::nil;
	}

	Addr VM::op_erro ()
	{
		Addr estr = pool.popcons(stak);
#if TYPECHECK
		if (Pool::StrnT != pool.type(estr))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_erro required (StrnT) but got: ")
					+ pool.print(estr)));
		}
#endif
		pool.pushcons(stak, pool.make_erro(pool.getnum(pool.popcons(code)), estr));
		return Pool::nil;
	}

	Addr VM::op_rcur ()
	{
		Addr args = pool.popcons(stak);
		Addr lambda = pool.popcons(stak);
#if TYPECHECK
		if (Pool::ConsT != pool.type(lambda)
				or (Pool::ConsT != pool.type(args)
					and not pool.isnil(args)))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_rcur required (ConsT ConsT) but got: ")
					+ pool.print(lambda) + std::string(" ") + pool.print(args)));
		}
#endif
		code = pool.getcar(pool.getcdr(lambda));
		lenv = pool.make_cons(args
				, pool.getcdr(pool.getcdr(lambda)));
		return Pool::nil;
	}

	Addr VM::op_setq ()
	{
		Addr y = pool.popcons(stak);
		Addr x = pool.popcons(stak);
		Addr v = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(x)
				or Pool::InumT != pool.type(y))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_setq required (Any InumT InumT) but got: ")
					+ pool.print(v) + std::string(" ")
					+ pool.print(x) + std::string(" ")
					+ pool.print(y)));
		}
#endif

		pool.setnth(
				pool.getnth(
					lenv, pool.getnum(y))
				, pool.getnum(x)
				, v);

		return Pool::nil;
	}

	Addr VM::op_addi ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(a)
				or Pool::InumT != pool.type(b))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_addi required (InumT InumT) but got: ")
					+ pool.print(a) + std::string(" ") + pool.print(b)));
		}
#endif
		pool.pushcons(stak, pool.make_inum(
					pool.getnum(a) + pool.getnum(b)));
		return Pool::nil;
	}
	
	Addr VM::op_subi ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(a)
				or Pool::InumT != pool.type(b))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_subi required (InumT InumT) but got: ")
					+ pool.print(a) + std::string(" ") + pool.print(b)));
		}
#endif
		pool.pushcons(stak, pool.make_inum(
					pool.getnum(a) - pool.getnum(b)));
		return Pool::nil;
	}

	Addr VM::op_muli ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(a)
				or Pool::InumT != pool.type(b))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_muli required (InumT InumT) but got: ")
					+ pool.print(a) + std::string(" ") + pool.print(b)));
		}
#endif
		pool.pushcons(stak, pool.make_inum(
					pool.getnum(a) * pool.getnum(b)));
		return Pool::nil;
	}

	Addr VM::op_divi ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(a)
				or Pool::InumT != pool.type(b))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_divi required (InumT InumT) but got: ")
					+ pool.print(a) + std::string(" ") + pool.print(b)));
		}
#endif
		pool.pushcons(stak, pool.make_inum(
					pool.getnum(a) / pool.getnum(b)));
		return Pool::nil;
	}

	Addr VM::op_modi ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(a)
				or Pool::InumT != pool.type(b))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_modi required (InumT InumT) but got: ")
					+ pool.print(a) + std::string(" ") + pool.print(b)));
		}
#endif
		pool.pushcons(stak, pool.make_inum(
					pool.getnum(a) % pool.getnum(b)));
		return Pool::nil;
	}

	Addr VM::op_rpla ()
	{
		Addr val = pool.popcons(stak);
		Addr cons = pool.popcons(stak);
#if TYPECHECK
		if (Pool::ConsT != pool.type(cons))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_rpla required (ConsT Any) but got: ")
						+ pool.print(cons) + std::string(" ") + pool.print(val)));
		}
#endif
		pool.setcar(cons, val);
		return Pool::nil;
	}

	Addr VM::op_rpld ()
	{
		Addr val = pool.popcons(stak);
		Addr cons = pool.popcons(stak);
#if TYPECHECK
		if (Pool::ConsT != pool.type(cons))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_rpld required (ConsT Any) but got: ")
						+ pool.print(cons) + std::string(" ") + pool.print(val)));
		}
#endif
		pool.setcdr(cons, val);
		return Pool::nil;
	}

	Addr VM::op_eq ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);

		if (a == b)
		{
			pool.pushcons(stak, pool.make_inum(1));
		}
		else
		{
			pool.pushcons(stak, Pool::nil);
		}

		return Pool::nil;
	}

	Addr VM::op_equal ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);

		if (pool.equal(a, b))
		{
			pool.pushcons(stak, pool.make_inum(1));
		}
		else
		{
			pool.pushcons(stak, Pool::nil);
		}

		return Pool::nil;
	}

	Addr VM::op_gt ()
	{
		Addr b = pool.popcons(stak);
		Addr a = pool.popcons(stak);
#if TYPECHECK
		if (Pool::InumT != pool.type(a)
				or Pool::InumT != pool.type(b))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_gt required (InumT InumT) but got: ")
					+ pool.print(a) + std::string(" ") + pool.print(b)));
		}
#endif
		if (pool.getnum(a) > pool.getnum(b))
		{
			pool.pushcons(stak, pool.make_inum(1));
		}
		else
		{
			pool.pushcons(stak, Pool::nil);
		}

		return Pool::nil;
	}

	Addr VM::op_def ()
	{
		Addr val = pool.popcons(stak);
		Addr sym = pool.popcons(stak);
#if TYPECHECK
		if (Pool::SymbT != pool.type(sym))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_def required (SymbT Any) but got: ")
						+ pool.print(sym) + std::string(" ") + pool.print(val)));
		}
#endif

		Addr seeker = genv;
		for (; not pool.isnil(seeker)
				; seeker = pool.getcdr(seeker))
		{
			if (pool.equal(sym, pool.getcar(pool.getcar(seeker))))
			{
				val = pool.getcdr(pool.getcar(seeker));
				break;
			}
		}
		
		if (pool.isnil(seeker))
		{
			pool.pushcons(genv, pool.make_cons(sym, val));
		}
		else
		{
			pool.setcdr(pool.getcar(seeker), val);
		}

		return Pool::nil;
	}

	Addr VM::op_ref ()
	{
		Addr sym = pool.popcons(stak);
#if TYPECHECK
		if (Pool::SymbT != pool.type(sym))
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string(
							"op_ref required (SymbT) but got: ") + pool.print(sym)));
		}
#endif
		Addr val = Pool::nil;
		for (Addr seeker = genv
				; not pool.isnil(seeker)
				; seeker = pool.getcdr(seeker))
		{
			if (pool.equal(sym, pool.getcar(pool.getcar(seeker))))
			{
				val = pool.getcdr(pool.getcar(seeker));
				break;
			}
		}

		if (pool.isnil(val))
		{
			return pool.make_erro(Symbol
					, pool.make_strn(
						pool.print(sym) + std::string(" is not defined.")));
		}
		else
		{
			pool.pushcons(stak, val);
		}

		return Pool::nil;
	}

	Addr VM::op_push00 ()
	{
#if AREACHECK
		if (0 >= pool.lengthcons(lenv)
				or 0 >= pool.lengthcons(pool.getnth(lenv, 0)))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(0 . 0) to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak
				, pool.getnth(pool.getnth(lenv, 0), 0));
		return Pool::nil;
	}

	Addr VM::op_push10 ()
	{
#if AREACHECK
		if (0 >= pool.lengthcons(lenv)
				or 1 >= pool.lengthcons(pool.getnth(lenv, 0)))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(1 . 0) to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak
				, pool.getnth(pool.getnth(lenv, 0), 1));
		return Pool::nil;
	}

	Addr VM::op_push20 ()
	{
#if AREACHECK
		if (0 >= pool.lengthcons(lenv)
				or 2 >= pool.lengthcons(pool.getnth(lenv, 0)))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(2 . 0) to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak
				, pool.getnth(pool.getnth(lenv, 0), 2));
		return Pool::nil;
	}

	Addr VM::op_push01 ()
	{
#if AREACHECK
		if (1 >= pool.lengthcons(lenv)
				or 0 >= pool.lengthcons(pool.getnth(lenv, 1)))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(0 . 1) to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak
				, pool.getnth(pool.getnth(lenv, 1), 0));
		return Pool::nil;
	}

	Addr VM::op_push11 ()
	{
#if AREACHECK
		if (1 >= pool.lengthcons(lenv)
				or 1 >= pool.lengthcons(pool.getnth(lenv, 1)))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(1 . 1) to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak
				, pool.getnth(pool.getnth(lenv, 1), 1));
		return Pool::nil;
	}

	Addr VM::op_push21 ()
	{
#if AREACHECK
		if (1 >= pool.lengthcons(lenv)
				or 2 >= pool.lengthcons(pool.getnth(lenv, 1)))
		{
			return pool.make_erro(ErroId::OutOfEnvironment
					, pool.make_strn(
						std::string("op_push got ")
						+ std::string("(2 . 1) to env: ")
						+ pool.print(lenv)));
		}
#endif
		pool.pushcons(stak
				, pool.getnth(pool.getnth(lenv, 1), 2));
		return Pool::nil;
	}

	Compiler::Compiler()
		: pool()
	{
	}

	bool Compiler::find_co_paren (std::istream &in
			, std::stringstream &ss)
	{
		size_t layer = 1;
		char c;
		for (in.read(&c, sizeof(char))
				; not in.eof()
				; in.read(&c, sizeof(char)))
		{
			if (')' == c) { --layer; }
			else if ('(' == c) { ++layer; }

			if (layer == 0) { return true; }

			ss.write(&c, sizeof(char));
		}
		return false;
	}

	void Compiler::growth (Addr tree, std::stringstream &buf)
	{
		if (not buf.str().empty())
		{
			pool.pushqueu(tree
					, pool.make_symb(buf.str()));
			buf.str("");
		}
	}

	Addr Compiler::parse (std::istream &in)
	{
		Addr tree = pool.make_queu();
		std::stringstream buf;
		char c;
		for (in.read(&c, sizeof(char))
				; not in.eof()
				; in.read(&c, sizeof(char)))
		{
			std::stringstream pbuf;
			switch (c)
			{
				case ';':
					while ('\n' != c)
					{
						in.read(&c, sizeof(char));
					}
				case ' ':
				case '\t':
				case '\n':
					growth(tree, buf);
					break;
				case '(':
					growth(tree, buf);
					if (not find_co_paren(in, pbuf))
					{
						return pool.make_erro(Syntax
								, pool.make_strn(
									"not found close parenthesis."));
					}
					pool.pushqueu(tree, parse(pbuf));
					break;
				case ')':
					return pool.make_erro(Syntax
							, pool.make_strn("find excess close parenthesis."));
				default:
					buf << c;
					break;
			}
		}
		growth(tree, buf);
		return pool.getexit(tree);
	}

//	Addr Compiler::geneq (Addr args, Addr symtab)
//	{
//#if ARGSCHECK
//		if (2 != pool.lengthcons(args))
//		{
//			return pool.make_erro(ArgsUnmatch
//					, pool.make_strn(
//						std::string("eq require 2 args but got: ")
//						+ std::to_string(
//							pool.lengthcons(args))
//						+ std::string(" args.")));
//		}
//#endif
//		Addr code = genargscode(args, symtab);
//		pool.pushqueu(code, pool.make_inum(0x19));
//		return code;
//	}

	Addr Compiler::genargscode (Addr args, Addr symtab)
	{
		// TODO
		return args;
	}

	Addr Compiler::genapplycode (Addr fn, Addr args, Addr symtab)
	{
		// TODO
		if (Pool::ConsT == pool.type(fn))
		{
			// TODO
		}

		std::string fname = pool.print(fn);
		if ("if" == fname)
		{
		}
		else if ("quote" == fname)
		{
		}
		else if ("define" == fname)
		{
		}
		else if ("lambda" == fname)
		{
		}

		return Pool::nil;
	}

	Addr Compiler::genpushcode (Addr sym, Addr symtab)
	{
		Addr lp = Pool::nil;
		size_t y = 0;
		for (Addr seekery = symtab
				; not pool.isnil(seekery)
				; seekery = pool.getcdr(seekery))
		{
			size_t x = 0;
			for (Addr seekerx = pool.getcar(seekery)
					; not pool.isnil(seekerx)
					; seekerx = pool.getcdr(seekerx))
			{
				if (pool.equal(sym, pool.getcar(seekerx)))
				{
					lp = pool.make_cons(
							pool.make_inum(x)
							, pool.make_inum(y));
					break;
				}
				++x;
			}
			if (not pool.isnil(lp)) { break; }
			++y;
		}
		
		Addr code = pool.make_queu();
		if (pool.isnil(lp))
		{
			// cnst c
			std::string symstr = pool.print(sym);
			for (size_t idx = 0
					; idx < symstr.size()
					; ++idx)
			{
				pool.pushqueu(code, pool.make_inum(symstr[idx]));
			}
			// vect symbsize
			pool.pushqueu(code, pool.make_inum(0x0c));
			pool.pushqueu(code, pool.make_inum(symstr.size()));
			// symb
			pool.pushqueu(code, pool.make_inum(0x0d));
			// ref
			pool.pushqueu(code, pool.make_inum(0x23));
		}
		else
		{
			// cnst x
			pool.pushqueu(code, pool.make_inum(0x02));
			pool.pushqueu(code, pool.make_inum(pool.getcar(lp)));
			// cnst y
			pool.pushqueu(code, pool.make_inum(0x02));
			pool.pushqueu(code, pool.make_inum(pool.getcdr(lp)));
			// push
			pool.pushqueu(code, pool.make_inum(0x01));
		}

		return code;
	}

	Addr Compiler::gencode (Addr tree, Addr symtab)
	{
		Byte typ = pool.type(tree);
		if (Pool::NilT == typ)
		{
			return Pool::nil;
		}
		else if (Pool::ConsT == typ)
		{
			return genapplycode(
					pool.getcar(tree)
					, pool.getcdr(tree), symtab);
		}
		else if (Pool::SymbT == typ)
		{
			return genpushcode(tree, symtab);
		}
		else
		{
			return pool.make_erro(UnEvaluatable
					, pool.make_strn(
						pool.print(tree)
						+ std::string(" is not evaluatable.")));
		}
	}

	Addr Compiler::write (std::ostream &out, Addr code)
	{
		for (Addr itr = code; Pool::ConsT == pool.type(itr)
				; itr = pool.getcdr(itr))
		{
			Byte b = (Byte)pool.getnum(itr);
			out.write((char *)&b, sizeof(Byte));
		}
		return 0;
	}

	std::string Compiler::print (Addr ptr)
	{
		return pool.print(ptr);
	}

	void Compiler::clean ()
	{
		pool.mark(Pool::nil);
		pool.sweep();

	}

	void Compiler::gc (Addr root)
	{
		pool.mark(root);
		pool.sweep();
	}
}

