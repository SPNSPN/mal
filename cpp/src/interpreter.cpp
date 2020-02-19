#include <float.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include "interpreter.h"

template <typename T>
size_t findidx (std::vector<T> coll, T val)
{
	return std::distance(coll.begin()
			, std::find(coll.begin(), coll.end(), val));
}

Interpreter::Interpreter ()
	: pool ()
	  , root ()
	  , genv (pool.make_cons(Pool::nil, Pool::nil))
	  , subr ()
	  , spfm ()
	  , spfm_if_id (0)
	  , spfm_do_id (0)
	  , spfm_syntax_id (0)
{
	root.push_back(genv);

	subr.push_back(&Interpreter::subr_eq);
	subr.push_back(&Interpreter::subr_cons);
	subr.push_back(&Interpreter::subr_car);
	subr.push_back(&Interpreter::subr_cdr);
	subr.push_back(&Interpreter::subr_atom);
	subr.push_back(&Interpreter::subr_equal);
	subr.push_back(&Interpreter::subr_rplaca);
	subr.push_back(&Interpreter::subr_rplacd);
	subr.push_back(&Interpreter::subr_last);
	subr.push_back(&Interpreter::subr_nconc);
	subr.push_back(&Interpreter::subr_add);
	subr.push_back(&Interpreter::subr_sub);
	subr.push_back(&Interpreter::subr_mul);
	subr.push_back(&Interpreter::subr_div);
	subr.push_back(&Interpreter::subr_mod);
	subr.push_back(&Interpreter::subr_gt);
	subr.push_back(&Interpreter::subr_lt);
	subr.push_back(&Interpreter::subr_int);
	subr.push_back(&Interpreter::subr_float);
	subr.push_back(&Interpreter::subr_list);
	subr.push_back(&Interpreter::subr_print);
	subr.push_back(&Interpreter::subr_prin);
	subr.push_back(&Interpreter::subr_sprint);
	subr.push_back(&Interpreter::subr_tolist);
	subr.push_back(&Interpreter::subr_load);
	subr.push_back(&Interpreter::subr_vect);
	subr.push_back(&Interpreter::subr_tovect);
	subr.push_back(&Interpreter::subr_getat);
	subr.push_back(&Interpreter::subr_setat);
	subr.push_back(&Interpreter::subr_processor);
	subr.push_back(&Interpreter::subr_throw);
	subr.push_back(&Interpreter::subr_symbol);
	subr.push_back(&Interpreter::subr_queu);
	subr.push_back(&Interpreter::subr_toqueu);
	subr.push_back(&Interpreter::subr_pushqueu);
	subr.push_back(&Interpreter::subr_popqueu);
	subr.push_back(&Interpreter::subr_concqueu);
	subr.push_back(&Interpreter::subr_empty);
	subr.push_back(&Interpreter::subr_type);
	subr.push_back(&Interpreter::subr_apply);
	subr.push_back(&Interpreter::subr_getc);

	spfm.push_back(&Interpreter::spfm_if);
	spfm.push_back(&Interpreter::spfm_quote);
	spfm.push_back(&Interpreter::spfm_lambda);
	spfm.push_back(&Interpreter::spfm_syntax);
	spfm.push_back(&Interpreter::spfm_define);
	spfm.push_back(&Interpreter::spfm_setq);
	spfm.push_back(&Interpreter::spfm_do);
	spfm.push_back(&Interpreter::spfm_and);
	spfm.push_back(&Interpreter::spfm_or);
	spfm.push_back(&Interpreter::spfm_quasiquote);
	spfm.push_back(&Interpreter::spfm_catch);
	spfm.push_back(&Interpreter::spfm_environment);

	pool.setcar(genv, pool.make_cons(
				pool.make_cons(
					pool.make_symb("nil"), Pool::nil)
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(
					pool.make_symb("t"), pool.t)
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(
					pool.make_symb("T"), pool.t)
				, pool.getcar(genv)));

	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("eq")
					, pool.make_subr("eq"
						, findidx<Subr>(subr, &Interpreter::subr_eq)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("cons")
					, pool.make_subr("cons"
						, findidx<Subr>(subr, &Interpreter::subr_cons)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("car")
					, pool.make_subr("car"
						, findidx<Subr>(subr, &Interpreter::subr_car)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("cdr")
					, pool.make_subr("cdr"
						, findidx<Subr>(subr, &Interpreter::subr_cdr)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("atom")
					, pool.make_subr("atom"
						, findidx<Subr>(subr, &Interpreter::subr_atom)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("equal")
					, pool.make_subr("equal"
						, findidx<Subr>(subr, &Interpreter::subr_equal)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("rplaca")
					, pool.make_subr("rplaca"
						, findidx<Subr>(subr, &Interpreter::subr_rplaca)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("rplacd")
					, pool.make_subr("rplacd"
						, findidx<Subr>(subr, &Interpreter::subr_rplacd)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("last")
					, pool.make_subr("last"
						, findidx<Subr>(subr, &Interpreter::subr_last)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("nconc")
					, pool.make_subr("nconc"
						, findidx<Subr>(subr, &Interpreter::subr_nconc)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("+")
					, pool.make_subr("add"
						, findidx<Subr>(subr, &Interpreter::subr_add)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("-")
					, pool.make_subr("sub"
						, findidx<Subr>(subr, &Interpreter::subr_sub)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("*")
					, pool.make_subr("mul"
						, findidx<Subr>(subr, &Interpreter::subr_mul)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("/")
					, pool.make_subr("div"
						, findidx<Subr>(subr, &Interpreter::subr_div)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("%")
					, pool.make_subr("mod"
						, findidx<Subr>(subr, &Interpreter::subr_mod)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb(">")
					, pool.make_subr("gt"
						, findidx<Subr>(subr, &Interpreter::subr_gt)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("<")
					, pool.make_subr("lt"
						, findidx<Subr>(subr, &Interpreter::subr_lt)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("int")
					, pool.make_subr("int"
						, findidx<Subr>(subr, &Interpreter::subr_int)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("float")
					, pool.make_subr("float"
						, findidx<Subr>(subr, &Interpreter::subr_float)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("list")
					, pool.make_subr("list"
						, findidx<Subr>(subr, &Interpreter::subr_list)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("print")
					, pool.make_subr("print"
						, findidx<Subr>(subr, &Interpreter::subr_print)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("prin")
					, pool.make_subr("prin"
						, findidx<Subr>(subr, &Interpreter::subr_prin)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("sprint")
					, pool.make_subr("sprint"
						, findidx<Subr>(subr, &Interpreter::subr_sprint)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("to-list")
					, pool.make_subr("to-list"
						, findidx<Subr>(subr, &Interpreter::subr_tolist)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("load")
					, pool.make_subr("load"
						, findidx<Subr>(subr, &Interpreter::subr_load)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("vect")
					, pool.make_subr("vect"
						, findidx<Subr>(subr, &Interpreter::subr_vect)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("to-vect")
					, pool.make_subr("to-vect"
						, findidx<Subr>(subr, &Interpreter::subr_tovect)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("getat")
					, pool.make_subr("getat"
						, findidx<Subr>(subr, &Interpreter::subr_getat)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("setat")
					, pool.make_subr("setat"
						, findidx<Subr>(subr, &Interpreter::subr_setat)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("processor")
					, pool.make_subr("processor"
						, findidx<Subr>(subr, &Interpreter::subr_processor)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("throw")
					, pool.make_subr("throw"
						, findidx<Subr>(subr, &Interpreter::subr_throw)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("symbol")
					, pool.make_subr("symbol"
						, findidx<Subr>(subr, &Interpreter::subr_symbol)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("queu")
					, pool.make_subr("queu"
						, findidx<Subr>(subr, &Interpreter::subr_queu)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("to-queu")
					, pool.make_subr("to-queu"
						, findidx<Subr>(subr, &Interpreter::subr_toqueu)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("pushqueu")
					, pool.make_subr("pushqueu"
						, findidx<Subr>(subr, &Interpreter::subr_pushqueu)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("popqueu")
					, pool.make_subr("popqueu"
						, findidx<Subr>(subr, &Interpreter::subr_popqueu)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("concqueu")
					, pool.make_subr("concqueu"
						, findidx<Subr>(subr, &Interpreter::subr_concqueu)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("empty")
					, pool.make_subr("empty"
						, findidx<Subr>(subr, &Interpreter::subr_empty)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("type")
					, pool.make_subr("type"
						, findidx<Subr>(subr, &Interpreter::subr_type)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("apply")
					, pool.make_subr("apply"
						, findidx<Subr>(subr, &Interpreter::subr_apply)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("getc")
					, pool.make_subr("getc"
						, findidx<Subr>(subr, &Interpreter::subr_getc)))
				, pool.getcar(genv)));

	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("if")
					, pool.make_spfm("if"
						, findidx<Spfm>(spfm, &Interpreter::spfm_if)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("quote")
					, pool.make_spfm("quote"
						, findidx<Spfm>(spfm, &Interpreter::spfm_quote)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("lambda")
					, pool.make_spfm("lambda"
						, findidx<Spfm>(spfm, &Interpreter::spfm_lambda)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("!")
					, pool.make_spfm("syntax"
						, findidx<Spfm>(spfm, &Interpreter::spfm_syntax)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("define")
					, pool.make_spfm("define"
						, findidx<Spfm>(spfm, &Interpreter::spfm_define)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("setq")
					, pool.make_spfm("setq"
						, findidx<Spfm>(spfm, &Interpreter::spfm_setq)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("do")
					, pool.make_spfm("do"
						, findidx<Spfm>(spfm, &Interpreter::spfm_do)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("and")
					, pool.make_spfm("and"
						, findidx<Spfm>(spfm, &Interpreter::spfm_and)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("or")
					, pool.make_spfm("or"
						, findidx<Spfm>(spfm, &Interpreter::spfm_or)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("quasiquote")
					, pool.make_spfm("quasiquote"
						, findidx<Spfm>(spfm, &Interpreter::spfm_quasiquote)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("catch")
					, pool.make_spfm("catch"
						, findidx<Spfm>(spfm, &Interpreter::spfm_catch)))
				, pool.getcar(genv)));
	pool.setcar(genv, pool.make_cons(
				pool.make_cons(pool.make_symb("environment")
					, pool.make_spfm("environment"
						, findidx<Spfm>(spfm, &Interpreter::spfm_environment)))
				, pool.getcar(genv)));

	spfm_if_id = findidx<Spfm>(spfm, &Interpreter::spfm_if);
	spfm_do_id = findidx<Spfm>(spfm, &Interpreter::spfm_do);
	spfm_syntax_id = findidx<Spfm>(spfm, &Interpreter::spfm_syntax);
}

Addr Interpreter::subr_eq (Addr args)
{
	Addr a = pool.getcar(args);
	Addr b = pool.getcar(pool.getcdr(args));

	if (a == b
			or (Pool::SymbT == pool.type(a)
				and Pool::SymbT == pool.type(b)
				and pool.equal(a, b)))
	{
		return pool.t;
	}

	// A vector cannot have the nil, therefore (eq (getat [nil] 0) nil) -> NIL
	if (pool.isnil(a) and pool.isnil(b))
	{
		return pool.t;
	}

	return Pool::nil;
}

Addr Interpreter::subr_cons (Addr args)
{
	return pool.make_cons(pool.getcar(args)
			, pool.getcar(pool.getcdr(args)));
}

Addr Interpreter::subr_car (Addr args)
{
	// TODO typecheck
	return pool.getcar(pool.getcar(args));
}

Addr Interpreter::subr_cdr (Addr args)
{
	// TODO typecheck
	return pool.getcdr(pool.getcar(args));
}

Addr Interpreter::subr_atom (Addr args)
{
	Byte typ = pool.type(pool.getcar(args));
	if (Pool::ConsT == typ) { return Pool::nil; }

	return pool.t;
}

Addr Interpreter::subr_equal (Addr args)
{
	if (pool.equal(pool.getcar(args), pool.getcar(pool.getcdr(args))))
	{
		return pool.t;
	}
	else
	{
		return Pool::nil;
	}
}

Addr Interpreter::subr_rplaca (Addr args)
{
	// TODO typecheck
	Addr cons = pool.getcar(args);
	pool.setcar(cons, pool.getcar(pool.getcdr(args)));
	return cons;
}

Addr Interpreter::subr_rplacd (Addr args)
{
	// TODO typecheck
	Addr cons = pool.getcar(args);
	pool.setcdr(cons, pool.getcar(pool.getcdr(args)));
	return cons;
}

Addr Interpreter::subr_last (Addr args)
{
	Addr obj = pool.getcar(args);
	Byte typ = pool.type(obj);
	if (Pool::ConsT == typ)
	{
		Addr rest = obj;
		for (; pool.consp(pool.getcdr(rest)); rest = pool.getcdr(rest)) { ; }
		return rest;
	}

	if (Pool::NilT == typ)
	{
		return Pool::nil;
	}

	if (Pool::QueuT == typ)
	{
		return pool.getentr(obj);
	}

	if (Pool::VectT == typ || Pool::StrnT == typ || Pool::SymbT == typ)
	{
		return pool.getatvect(obj, pool.getvsize(obj) - 1);
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot apply subr_last to ") + print(obj)));
}

Addr Interpreter::subr_nconc (Addr args)
{
	// TODO typecheck
	Addr a = pool.getcar(args);
	Addr d = pool.getcdr(args);
	if (pool.isnil(d)) { return a; }

	return pool.nconccons(a, subr_nconc(d));
}

Addr Interpreter::subr_add (Addr args)
{
	Addr acci = 0;
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr n = pool.getcar(rest);
		Byte typ = pool.type(n);
		if (Pool::InumT == typ)
		{
			acci += pool.getnum(n);
		}
		else if (Pool::FnumT == typ)
		{
			double accf = (double)acci;
			for (Addr restf = rest
					; pool.consp(restf)
					; restf = pool.getcdr(restf))
			{
				Addr nf = pool.getcar(restf);
				Byte typf = pool.type(nf);
				if (Pool::InumT == typf)
				{
					accf += pool.getnum(nf);
				}
				else if (Pool::FnumT == typf)
				{
					accf += pool.getfnum(nf);
				}
				else
				{
					return pool.make_erro(Type
							, pool.make_strn(std::string("cannot add ") + print(args)));
				}
			}
			return pool.make_fnum(accf);
		}
		else
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string("cannot add ") + print(args)));
		}
	}
	return pool.make_inum(acci);
}

Addr Interpreter::subr_sub (Addr args)
{
	Addr n = pool.getcar(args);
	Byte typ = pool.type(n);

	Addr na = subr_add(pool.getcdr(args));
	Byte typa = pool.type(na);

	if (Pool::InumT == typ and Pool::InumT == typa)
	{
		return pool.make_inum(pool.getnum(n) - pool.getnum(na));
	}
	else if (Pool::FnumT == typ and Pool::InumT == typa)
	{
		return pool.make_fnum(pool.getfnum(n) - pool.getnum(na));
	}
	else if (Pool::InumT == typ and Pool::FnumT == typa)
	{
		return pool.make_fnum(pool.getnum(n) - pool.getfnum(na));
	}
	else if (Pool::FnumT == typ and Pool::FnumT == typa)
	{
		return pool.make_fnum(pool.getfnum(n) - pool.getfnum(na));
	}
	else
	{
		return pool.make_erro(Type
				, pool.make_strn(std::string("cannot sub ") + print(args)));
	}
}

Addr Interpreter::subr_mul (Addr args)
{
	Addr acci = 1;
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr n = pool.getcar(rest);
		Byte typ = pool.type(n);
		if (Pool::InumT == typ)
		{
			acci *= pool.getnum(n);
		}
		else if (Pool::FnumT == typ)
		{
			double accf = (double)acci;
			for (Addr restf = rest
					; pool.consp(restf)
					; restf = pool.getcdr(restf))
			{
				Addr nf = pool.getcar(restf);
				Byte typf = pool.type(nf);
				if (Pool::InumT == typf)
				{
					accf *= pool.getnum(nf);
				}
				else if (Pool::FnumT == typf)
				{
					accf *= pool.getfnum(nf);
				}
				else
				{
					return pool.make_erro(Type
							, pool.make_strn(std::string("cannot mul ")
								+ print(args)));
				}
			}
			return pool.make_fnum(accf);
		}
		else
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string("cannot mul ") + print(args)));
		}
	}
	return pool.make_inum(acci);
}

Addr Interpreter::subr_div (Addr args)
{
	Addr n = pool.getcar(args);
	Byte typ = pool.type(n);

	Addr na = subr_mul(pool.getcdr(args));
	Byte typa = pool.type(na);

	if (Pool::InumT == typ and Pool::InumT == typa)
	{
		return pool.make_inum(pool.getnum(n) / pool.getnum(na));
	}
	else if (Pool::FnumT == typ and Pool::InumT == typa)
	{
		return pool.make_fnum(pool.getfnum(n) / pool.getnum(na));
	}
	else if (Pool::InumT == typ and Pool::FnumT == typa)
	{
		return pool.make_fnum(pool.getnum(n) / pool.getfnum(na));
	}
	else if (Pool::FnumT == typ and Pool::FnumT == typa)
	{
		return pool.make_fnum(pool.getfnum(n) / pool.getfnum(na));
	}
	else
	{
		return pool.make_erro(Type
				, pool.make_strn(std::string("cannot div ") + print(args)));
	}
}

Addr Interpreter::subr_mod (Addr args)
{
	Byte typa = pool.type(pool.getcar(args));
	Byte typb = pool.type(pool.getcar(pool.getcdr(args)));
	if (not ((Pool::InumT == typa or Pool::FnumT == typa)
			and (Pool::InumT == typb or Pool::FnumT == typb)))
	{
		return pool.make_erro(Type
				, pool.make_strn(std::string("cannot mod ") + print(args)));
	}
	return pool.make_inum(pool.getnum(pool.getcar(args))
			% pool.getnum(pool.getcar(pool.getcdr(args))));
}

Addr Interpreter::subr_gt (Addr args)
{
	float acc = FLT_MAX;
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr a = pool.getcar(rest);
		Byte typ = pool.type(a);
		float n;

		if (Pool::InumT == typ)
		{
			n = pool.getnum(a);
		}
		else if (Pool::FnumT == typ)
		{
			n = pool.getfnum(a);
		}
		else
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string("cannot apply subr_gt to ")
						+ print(args)));
		}

		if (acc > n)
		{
			acc = n;
		}
		else
		{
			return Pool::nil;
		}
	}
	return pool.t;
}

Addr Interpreter::subr_lt (Addr args)
{
	float acc = -FLT_MAX;
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr a = pool.getcar(rest);
		Byte typ = pool.type(a);
		float n;

		if (Pool::InumT == typ)
		{
			n = pool.getnum(a);
		}
		else if (Pool::FnumT == typ)
		{
			n = pool.getfnum(a);
		}
		else
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string("cannot apply subr_lt to ")
						+ print(args)));
		}

		if (acc < n)
		{
			acc = n;
		}
		else
		{
			return Pool::nil;
		}
	}
	return pool.t;
}

Addr Interpreter::subr_int (Addr args)
{
	Addr num = pool.getcar(args);
	Byte typ = pool.type(num);
	if (Pool::InumT == typ) { return num; }
	if (Pool::FnumT == typ)
	{
		return pool.make_inum((Fixnum)(pool.getfnum(num)));
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot cast ") + print(num)
				+ std::string(" to InumT.")));
}

Addr Interpreter::subr_float (Addr args)
{
	Addr num = pool.getcar(args);
	Byte typ = pool.type(num);
	if (Pool::FnumT == typ) { return num; }
	if (Pool::InumT == typ)
	{
		return pool.make_fnum((float)(pool.getnum(num)));
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot cast ") + print(num)
				+ std::string(" to FnumT.")));
}

Addr Interpreter::subr_list (Addr args)
{
	return args;
}

Addr Interpreter::subr_print (Addr args)
{
	subr_prin(args);
	std::cout << std::endl;
	return Pool::nil;
}

Addr Interpreter::subr_prin (Addr args)
{
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr a = pool.getcar(rest);
		if (Pool::StrnT == pool.type(a))
		{
			std::cout
				<< print(a).substr(1, pool.getvsize(a))
				<< std::flush;
		}
		else
		{
			std::cout << print(a) << std::flush;
		}
	}
	return Pool::nil;
}

Addr Interpreter::subr_sprint (Addr args)
{
	std::string str;
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr a = pool.getcar(rest);
		if (Pool::StrnT == pool.type(a))
		{
			str += print(a).substr(1, pool.getvsize(a));
		}
		else
		{
			str += print(a);
		}
	}
	return pool.make_strn(str);
}

Addr Interpreter::subr_tolist (Addr args)
{
	Addr obj = pool.getcar(args);
	Byte typ = pool.type(obj);
	if (Pool::StrnT == typ
			or Pool::VectT == typ
			or Pool::SymbT == typ)
	{
		Addr list = pool.make_queu();
		for (Addr itr = pool.getvbegin(obj)
				; itr < pool.getvend(obj)
				; ++itr)
		{
			pool.pushqueu(list, itr);
		}
		return pool.getexit(list);
	}

	// TODO needs return copy object
	if (Pool::QueuT == typ)
	{
		return pool.getexit(obj);
	}

	if (Pool::ConsT == typ or Pool::NilT == typ)
	{
		return obj;
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot cast ") + print(obj)
				+ std::string(" to ConsT.")));
}

Addr Interpreter::subr_load (Addr args)
{
	Addr path = pool.getcar(args);
	if (Pool::StrnT == pool.type(path))
	{
		std::fstream fin(print(path).substr(1, pool.getvsize(path)));
		if (fin)
		{
			return eval(readtop(fin), genv);
		}
		return pool.make_erro(FileNotFound
				, pool.make_strn(std::string("not found file: ") + print(path)));
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot apply load to ") + print(path)));
}

Addr Interpreter::subr_vect (Addr args)
{
	Addr len = pool.lengthcons(args);
	Addr vect = pool.make_vect(len);
	Addr idx = 0;
	for (Addr rest = args
			; Pool::ConsT == pool.type(rest)
			; rest = pool.getcdr(rest))
	{
		pool.setatvect(vect, idx, pool.getcar(rest));
		++idx;
	}
	return vect;
}

Addr Interpreter::subr_tovect (Addr args)
{
	Addr coll = pool.getcar(args);
	Byte typ = pool.type(coll);
	if (Pool::ConsT == typ or Pool::NilT == typ)
	{
		return subr_vect(coll);
	}

	if (Pool::StrnT == typ or Pool::SymbT == typ)
	{
		return subr_vect(subr_tolist(args));
	}

	if (Pool::QueuT == typ)
	{
		return subr_vect(pool.getexit(coll));
	}

	if (Pool::VectT == typ)
	{
		return coll;
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot cast ") + print(coll)
			+ std::string(" to VectT.")));
}

Addr Interpreter::subr_getat (Addr args)
{
	// TODO areacheck
	Byte typ = pool.type(pool.getcar(args));
	if (Pool::VectT == typ)
	{
		return pool.getatvect(pool.getcar(args)
				, pool.getnum(pool.getcar(pool.getcdr(args))));
	}
	if (Pool::StrnT == typ)
	{
		char c = pool.getnum(pool.getatvect(pool.getcar(args)
				, pool.getnum(pool.getcar(pool.getcdr(args)))));
		return pool.make_strn(std::string(1, c));
	}
	if (Pool::SymbT == typ)
	{
		char c = pool.getnum(pool.getatvect(pool.getcar(args)
				, pool.getnum(pool.getcar(pool.getcdr(args)))));
		return pool.make_symb(std::string(1, c));
	}
	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot apply getat to ")
				+ print(pool.getcar(args))));
}

Addr Interpreter::subr_setat (Addr args)
{
	// TODO typecheck
	// TODO areacheck
	Addr vect = pool.getcar(args);
	Fixnum idx = pool.getnum(pool.getcar(pool.getcdr(args)));
	Addr val = pool.getcar(pool.getcdr(pool.getcdr(args)));
	Byte typ = pool.type(vect);
	Byte vtyp = pool.type(val);
	if (Pool::VectT == typ)
	{
		pool.setatvect(vect, idx, val);
		return vect;
	}
	if (Pool::StrnT == typ or Pool::SymbT == typ)
	{
		if (Pool::InumT == vtyp)
		{
			pool.setatvect(vect, idx, val);
		}
		else if (Pool::StrnT == vtyp or Pool::SymbT == vtyp)
		{
			pool.setatvect(vect, idx, pool.getatvect(val, 0));
		}
		else
		{
			return pool.make_erro(Type
					, pool.make_strn(std::string("cannot setat " + print(val)
							+ " to " + print(vect))));
		}
		return vect;
	}
	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot apply setat to ")
				+ print(vect)));
}

Addr Interpreter::subr_processor (Addr args)
{
	return pool.make_symb("c++");
}

Addr Interpreter::subr_throw (Addr args)
{
	Addr eid = pool.getcar(args);
	Addr emessage = pool.getcar(pool.getcdr(args));
	// TODO typecheck
	return pool.make_erro(pool.getnum(eid), emessage);
}

Addr Interpreter::subr_symbol (Addr args)
{
	Addr name = pool.getcar(args);
	Byte typ = pool.type(name);
	if (Pool::VectT == typ or Pool::StrnT == typ)
	{
		return pool.make_symb(name);
	}

	if (Pool::ConsT == typ or Pool::NilT == typ)
	{
		return pool.make_symb(subr_vect(name));
	}

	if (Pool::QueuT == typ)
	{
		return pool.make_symb(subr_vect(pool.getexit(name)));
	}

	if (Pool::SymbT == typ)
	{
		return name;
	}

	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot cast ") + print(name)
			+ std::string(" to SymbT.")));
}

Addr Interpreter::subr_queu (Addr args)
{
	Addr queu = pool.make_queu();
	for (Addr rest = args
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		pool.pushqueu(queu, pool.getcar(rest));
	}
	return queu;
}

Addr Interpreter::subr_toqueu (Addr args)
{
	Addr coll = pool.getcar(args);
	Byte typ = pool.type(coll);
	if (Pool::ConsT == typ or Pool::NilT == typ)
	{
		Addr queu = pool.make_queu();
		for (Addr rest = coll
				; pool.consp(rest)
				; rest = pool.getcdr(rest))
		{
			pool.pushqueu(queu, pool.getcar(rest));
		}
		return queu;
	}

	if (Pool::VectT == typ
			or Pool::StrnT == typ
			or Pool::SymbT == typ)
	{
		return subr_toqueu(pool.make_cons(subr_tolist(args), Pool::nil));
	}

	if (Pool::QueuT == typ)
	{
		return coll;
	}
	
	return pool.make_erro(Type
			, pool.make_strn(std::string("cannot cast ") + print(coll)
			+ std::string(" to QueuT.")));
}

Addr Interpreter::subr_pushqueu (Addr args)
{
	// typecheck
	Addr queu = pool.getcar(args);
	Addr val = pool.getcar(pool.getcdr(args));
	pool.pushqueu(queu, val);
	return queu;
}

Addr Interpreter::subr_popqueu (Addr args)
{
	// typecheck
	Addr queu = pool.getcar(args);
	return pool.popqueu(queu);
}

Addr Interpreter::subr_concqueu (Addr args)
{
	Addr qa = pool.getcar(args);
	Addr qb = pool.getcar(pool.getcdr(args));
	pool.concqueu(qa, qb);
	return qa;
}

Addr Interpreter::subr_empty (Addr args)
{
	Addr coll = pool.getcar(args);
	Byte typ = pool.type(coll);

	if (Pool::NilT == typ)
	{
		return pool.t;
	}

	if (Pool::VectT == typ
			or Pool::StrnT == typ
			or Pool::SymbT == typ)
	{
		if (pool.getvsize(coll) < 1)
		{
			return pool.t;
		}
	}

	if (Pool::QueuT == typ)
	{
		if (pool.isnil(pool.getexit(coll)) and pool.isnil(pool.getentr(coll)))
		{
			return pool.t;
		}
	}
	return Pool::nil;
}

Addr Interpreter::subr_type (Addr args)
{
	Addr obj = pool.getcar(args);
	Addr typ = pool.type(obj);
	if(Pool::NilT == typ)
	{
		return pool.make_symb("<nil>");
	}
	else if (Pool::InumT == typ)
	{
		return pool.make_symb("<inum>");
	}
	else if (Pool::ConsT == typ)
	{
		return pool.make_symb("<cons>");
	}
	else if (Pool::QueuT == typ)
	{
		return pool.make_symb("<queu>");
	}
	else if (Pool::VectT == typ)
	{
		return pool.make_symb("<vect>");
	}
	else if (Pool::SymbT == typ)
	{
		return pool.make_symb("<symb>");
	}
	else if (Pool::StrnT == typ)
	{
		return pool.make_symb("<strn>");
	}
	else if (Pool::PackT == typ)
	{
		return pool.make_symb("<pack>");
	}
	else if (Pool::PiceT == typ)
	{
		return pool.make_symb("<pice>");
	}
	else if (Pool::ErroT == typ)
	{
		return pool.make_symb("<erro>");
	}
	else if (Pool::FnumT == typ)
	{
		return pool.make_symb("<fnum>");
	}
	else if (Pool::ExtdT == typ)
	{
		Addr ttyp = pool.gettname(obj);
		if (pool.ExtdSubrT == ttyp)
		{
			return pool.make_symb("<subr>");
		}
		else if (pool.ExtdSpfmT == ttyp)
		{
			return pool.make_symb("<spfm>");
		}
		else if (pool.ExtdFuncT == ttyp)
		{
			return pool.make_symb("<func>");
		}
	}
	return Pool::nil;
}

Addr Interpreter::subr_apply (Addr args)
{
	Addr proc = pool.getcar(args);
	if (Pool::ExtdT == pool.type(proc))
	{
		Addr ttyp = pool.gettname(proc);
		if (pool.ExtdSubrT == ttyp)
		{
			return apply_subr(proc, pool.getcar(pool.getcdr(args)));
		}
		if (pool.ExtdFuncT == ttyp)
		{
			return apply_func(proc, pool.getcar(pool.getcdr(args)));
		}
	}

	return pool.make_erro(UnCallable
			, pool.make_strn(print(proc) + std::string(" is not callable.")));
}

Addr Interpreter::subr_getc (Addr args)
{
	return pool.make_inum(getchar());
}

Addr Interpreter::spfm_if (Addr args, Addr env)
{
	Addr boolean = eval(pool.getcar(args), env);
	if (Pool::ErroT == pool.type(boolean))
	{
		return boolean;
	}

	if (pool.isnil(boolean))
	{
		return eval(pool.getcar(pool.getcdr(pool.getcdr(args))), env);
	}
	else
	{
		return eval(pool.getcar(pool.getcdr(args)), env);
	}
}

Addr Interpreter::spfm_quote (Addr args, Addr env)
{
	return pool.getcar(args);
}

Addr Interpreter::spfm_lambda (Addr args, Addr env)
{
	return pool.make_func(pool.getcar(args), pool.getcar(pool.getcdr(args)), env);
}

Addr Interpreter::spfm_syntax (Addr args, Addr env)
{
	// TODO typecheck
	Addr proc = eval(pool.getcar(args), env);
	if (Pool::ErroT == pool.type(proc)) { return proc; }

	return eval(subr_apply(pool.make_list(proc, pool.getcdr(args))), env);
}

Addr Interpreter::spfm_define (Addr args, Addr env)
{
	// TODO typecheck
	Addr sym = pool.getcar(args);
	Addr val = eval(pool.getcar(pool.getcdr(args)), env);
	if (Pool::ErroT == pool.type(val)) { return val; }

	Addr it = seekenv(genv, sym);
	if (Pool::ErroT != pool.type(it))
	{
		// sym has already defined.
		pool.setcdr(it, val);
		return sym;
	}

	pool.setcar(genv, pool.make_cons(pool.make_cons(sym, val), pool.getcar(genv)));
	return sym;
}

Addr Interpreter::spfm_setq (Addr args, Addr env)
{
	// TODO typecheck
	Addr sym = pool.getcar(args);
	Addr val = eval(pool.getcar(pool.getcdr(args)), env);
	if (Pool::ErroT == pool.type(val)) { return val; }

	Addr it = seekenv(env, sym);
	if (Pool::ErroT == pool.type(it))
	{
		return pool.make_erro(Symbol
				, pool.make_strn(print(sym) + std::string(" is not defined.")));
	}

	pool.setcdr(it, val);
	return val;
}

Addr Interpreter::spfm_do (Addr args, Addr env)
{
	Addr ret = Pool::nil;
	for (Addr expr = args
			; pool.consp(expr)
			; expr = pool.getcdr(expr))
	{
		ret = eval(pool.getcar(expr), env);
		if (Pool::ErroT == pool.type(ret)) { return ret; }
	}
	return ret;
}

Addr Interpreter::spfm_and (Addr args, Addr env)
{
	Addr ret = pool.t;
	for (Addr expr = args
			; pool.consp(expr)
			; expr = pool.getcdr(expr))
	{
		ret = eval(pool.getcar(expr), env);
		if (Pool::ErroT == pool.type(ret)) { return ret; }
		if (pool.isnil(ret)) { return Pool::nil; }
	}
	return ret;
}

Addr Interpreter::spfm_or (Addr args, Addr env)
{
	Addr ret = Pool::nil;
	for (Addr expr = args
			; pool.consp(expr)
			; expr = pool.getcdr(expr))
	{
		ret = eval(pool.getcar(expr), env);
		if (not pool.isnil(ret)) { return ret; }
	}
	return Pool::nil;
}



Addr Interpreter::spfm_quasiquote (Addr args, Addr env)
{
	return expand_quasiquote(pool.getcar(args), env);
}

Addr Interpreter::spfm_catch (Addr args, Addr env)
{
	root.push_back(args);
	Addr except = eval(pool.getcar(args), env);
	root.push_back(except);
	Addr body = eval(pool.getcar(pool.getcdr(args)), env);

	// TODO typecheck
	root.pop_back(); // release except
	root.pop_back(); // release args

	if (Pool::ErroT == pool.type(body))
	{
		root.push_back(env);
		root.push_back(except);
		root.push_back(body);

		Addr ebody = eval(pool.getestr(body), env);

		root.pop_back(); // release body
		root.pop_back(); // release except
		root.pop_back(); // release env

		if (Pool::ErroT == pool.type(ebody)) { return ebody; }

		if (Pool::ExtdT == pool.type(except))
		{
			return  subr_apply(pool.make_list(except
						, pool.make_list(pool.make_inum(pool.geteid(body)), ebody)));
		}

		return pool.make_erro(UnCallable
				, pool.make_strn(print(except) + std::string(" is not callable.")));

	}
	return body;
}

Addr Interpreter::spfm_environment (Addr args, Addr env)
{
	return env;
}


void Interpreter::gc ()
{
	pool.mark(pool.t);
	pool.mark(pool.ExtdSubrT);
	pool.mark(pool.ExtdSpfmT);
	pool.mark(pool.ExtdFuncT);
	for (auto itr = root.begin(); itr != root.end(); ++itr)
	{
		pool.mark(*itr);
	}
	pool.sweep();
}

bool Interpreter::find_co_paren (std::istream &in
		, std::stringstream &ss)
{
	size_t layer = 1;
	char c;
	bool str_flag = false;
	for (in.read(&c, sizeof(char))
			; not in.eof()
			; in.read(&c, sizeof(char)))
	{
		if ('\\' == c)
		{
			ss.write(&c, sizeof(char));
			in.read(&c, sizeof(char));
		}
		else if ('"' == c) { str_flag = not str_flag; }
		else if ((not str_flag) and (')' == c)) { --layer; }
		else if ((not str_flag) and ('(' == c)) { ++layer; }

		if (0 == layer) { return true; }

		ss.write(&c, sizeof(char));
	}
	return false;
}

bool Interpreter::find_co_brackets (std::istream &in
		, std::stringstream &ss)
{
	size_t layer = 1;
	char c;
	bool str_flag = false;
	for (in.read(&c, sizeof(char))
			; not in.eof()
			; in.read(&c, sizeof(char)))
	{
		if ('\\' == c)
		{
			ss.write(&c, sizeof(char));
			in.read(&c, sizeof(char));
		}
		else if ('"' == c) { str_flag = not str_flag; }
		else if ((not str_flag) and (']' == c)) { --layer; }
		else if ((not str_flag) and ('[' == c)) { ++layer; }

		if (0 == layer) { return true; }

		ss.write(&c, sizeof(char));
	}
	return false;
}

bool Interpreter::take_string (std::istream &in, std::stringstream &ss)
{
	char c;
	for (in.read(&c, sizeof(char))
			; not in.eof()
			; in.read(&c, sizeof(char)))
	{
		if ('"' == c) { return true; }
		if ('\\' == c)
		{
			in.read(&c, sizeof(char));
			if ('a' == c) { c = '\a'; }
			else if ('b' == c) { c = '\b'; }
			else if ('f' == c) { c = '\f'; }
			else if ('n' == c) { c = '\n'; }
			else if ('r' == c) { c = '\r'; }
			else if ('t' == c) { c = '\t'; }
			else if ('v' == c) { c = '\v'; }
			else if ('0' == c) { c = '\0'; }
		}
		ss.write(&c, sizeof(char));
	}
	return false;
}

Addr Interpreter::read_tok (const std::string &s)
{
	if ("nil" == s or "NIL" == s) { return Pool::nil; }

	if (('-' == s[0] and s.length() > 1) or isdigit(s[0]))
	{
		enum {
			IsInteger, IsFloat, Other
		} flg = IsInteger;
		for (auto c = s.cbegin() + 1; c != s.cend(); ++c)
		{
			if (isdigit(*c)) { continue; }
			if (IsInteger == flg and '.' == *c)
			{
				flg = IsFloat;
				continue;
			}

			flg = Other;
			break;
		}

		if (IsInteger == flg) { return pool.make_inum(stoi(s)); }

		if (IsFloat == flg) { return pool.make_fnum(stof(s)); }
	}

	return pool.make_symb(s);
}

Addr Interpreter::wrap_readmacros (Addr tok, Addr &rmacs)
{
	for (Addr rmac = rmacs
			; Pool::ConsT == pool.type(rmac)
			; rmac = pool.getcdr(rmac))
	{
		tok = pool.make_cons(pool.getcar(rmac)
				, pool.make_cons(tok, Pool::nil));
	}
	rmacs = Pool::nil;
	return tok;
}

void Interpreter::growth (Addr tree
		, std::stringstream &buf, Addr &rmacs)
{
	std::string s = buf.str();
	if (not s.empty())
	{
		pool.pushqueu(tree, wrap_readmacros(
					read_tok(s), rmacs));
		buf.str("");
	}
}

Addr Interpreter::read (std::istream &in)
{
	Addr tree = pool.make_queu();
	std::stringstream buf;
	Addr rmacs = Pool::nil;
	char c;
	for (in.read(&c, sizeof(char))
			; not in.eof()
			; in.read(&c, sizeof(char)))
	{
		std::stringstream pbuf;
		switch (c)
		{
			case ';':
				while ('\n' != c) { in.read(&c, sizeof(char)); }
			case ' ':
			case '\t':
			case '\n':
				growth(tree, buf, rmacs);
				break;
			case '(':
				growth(tree, buf, rmacs);
				if (not find_co_paren(in, pbuf))
				{
					return pool.make_erro(Syntax
							, pool.make_strn("not found close parenthesis."));
				}
				pool.pushqueu(tree
						, wrap_readmacros(
							read(pbuf), rmacs));
				break;
			case ')':
				return pool.make_erro(Syntax
						, pool.make_strn("found excess close parenthesis."));
			case '[':
				growth(tree, buf, rmacs);
				if (not find_co_brackets(in, pbuf))
				{
					return pool.make_erro(Syntax
							, pool.make_strn("not found close brackets."));
				}
				if (pool.isnil(rmacs))
				{
					pool.pushqueu(tree, pool.make_cons(pool.make_symb("vect")
								, read(pbuf)));
				}
				else
				{
					pool.pushqueu(tree
							, pool.make_list(pool.make_symb("to-vect")
								,wrap_readmacros(read(pbuf), rmacs)));
				}
				break;
			case ']':
				return pool.make_erro(Syntax
						, pool.make_strn("found excess close brackets."));
			case '.':
				if (buf.str().empty()) // cons-dot
				{
					Addr entr = pool.getentr(tree);
					pool.setcdr(entr
							, pool.getcar(read(in)));
					return pool.getexit(tree);
				}
				buf << c; // decimal-point
				break;
			case '"':
				growth(tree, buf, rmacs);
				if (not take_string(in, pbuf))
				{
					return pool.make_erro(Syntax
							, pool.make_strn("not found close double quote."));
				}
				pool.pushqueu(tree, pool.make_strn(pbuf.str()));
				break;
			case '\'':
				growth(tree, buf, rmacs);
				pool.pushcons(rmacs
						, pool.make_symb("quote"));
				break;
			case '`':
				growth(tree, buf, rmacs);
				pool.pushcons(rmacs
						, pool.make_symb("quasiquote"));
				break;
			case ',':
				growth(tree, buf, rmacs);
				pool.pushcons(rmacs
						, pool.make_symb("unquote"));
				break;
			case '@':
				growth(tree, buf, rmacs);
				pool.pushcons(rmacs
						, pool.make_symb("splicing"));
				break;
			default:
				buf << c;
				break;
		}
	}
	growth(tree, buf, rmacs);
	return pool.getexit(tree);
}

Addr Interpreter::readtop (std::istream &in)
{
	Addr tree = read(in);
	if (Pool::ErroT == pool.type(tree)) { return tree; }

	return pool.make_cons(pool.make_symb("do"), tree);
}

Addr Interpreter::bind_tree (Addr treea, Addr treeb)
{
	if (Pool::ConsT == pool.type(treea))
	{
		if (Pool::ConsT == pool.type(treeb))
		{
			Addr binda = bind_tree(pool.getcar(treea)
						, pool.getcar(treeb));
			Addr bindd = bind_tree(pool.getcdr(treea)
						, pool.getcdr(treeb));
			if (Pool::ErroT == pool.type(binda)
					or Pool::ErroT == pool.type(bindd))
			{
				return pool.make_erro(Syntax
						, pool.make_strn(std::string("cannot bind: ")
							+ print(treea) + std::string(" and ") + print(treeb)));
			}

			return pool.nconccons(binda, bindd);
		}
	}
	else if (Pool::SymbT == pool.type(treea))
	{
		return pool.make_cons(
				pool.make_cons(treea, treeb), Pool::nil);
	}
	else if (pool.isnil(treea) or pool.isnil(treeb))
	{
		return Pool::nil;
	}

	return pool.make_erro(Syntax
			, pool.make_strn(std::string("cannot bind: ")
				+ print(treea) + std::string(" and ") + print(treeb)));
}

Addr Interpreter::zip (Addr colla, Addr collb)
{
	Addr eargs = pool.make_queu();
	Addr a = colla;
	Addr b = collb;
	while (pool.consp(a) and pool.consp(b))
	{
		pool.pushqueu(eargs
				, pool.make_cons(pool.getcar(a), pool.getcar(b)));
		a = pool.getcdr(a);
		b = pool.getcdr(b);
	}
	return pool.getexit(eargs);
}

Addr Interpreter::mapeval (Addr coll, Addr env)
{
	Addr eargs = pool.make_queu();
	root.push_back(eargs);
	for (Addr rest = coll
			; pool.consp(rest)
			; rest = pool.getcdr(rest))
	{
		Addr a = eval(pool.getcar(rest), env);
		if (Pool::ErroT == pool.type(a))
		{
			root.pop_back(); // release eargs
			return a;
		}
		pool.pushqueu(eargs, a);
	}
	root.pop_back(); // release eargs
	return pool.getexit(eargs);
}

//Addr Interpreter::apply (Addr proc, Addr args, Addr env)
//{
//	if (Pool::ErroT == pool.type(proc)) { return proc; }
//
//	Addr proctyp = pool.gettname(proc);
//	if (SubrT == proctyp)
//	{
//		return (this->*subr[
//				pool.getnum(pool.getatextd(proc, 0))])(
//				mapsp(&Interpreter::eval, args, env));
//	}
//	else if (FuncT == proctyp)
//	{
//		return eval(pool.getatextd(proc, 1)
//				, pool.make_cons(
//					zip(pool.getatextd(proc, 0)
//						, mapsp(&Interpreter::eval, args, env))
//					, pool.getatextd(proc, 2)));
//	}
//	else if (SpfmT == proctyp)
//	{
//		return (this->*spfm[
//				pool.getnum(pool.getatextd(proc, 0))])(args, env);
//	}
//	else
//	{
//		return pool.make_erro(UnCallable
//				, pool.make_strn(
//					pool.print(proc)
//					+ std::string(" is not callable.")));
//	}
//}

Addr Interpreter::seekenv (Addr alist, Addr key)
{
	for (Addr frame = alist
			; pool.consp(frame)
			; frame = pool.getcdr(frame))
	{
		for (Addr seeker = pool.getcar(frame)
				; pool.consp(seeker)
				; seeker = pool.getcdr(seeker))
		{
			if (pool.equal(key, pool.getcar(pool.getcar(seeker))))
			{
				return pool.getcar(seeker);
			}
		}
	}
	return pool.make_erro(Symbol
			, pool.make_strn(print(key) + std::string(" is not defined.")));
}

Addr Interpreter::apply_if (Addr pred, Addr then, Addr els, Addr env)
{
	Addr res = eval(pred, env);
	if (Pool::ErroT == pool.type(res)) { return res; }

	if (pool.isnil(res)) { return els; }
	return then;
}

Addr Interpreter::apply_do (Addr expr, Addr env)
{
	for (; pool.consp(pool.getcdr(expr))
			; expr = pool.getcdr(expr))
	{
		Addr erro = eval(pool.getcar(expr), env);
		if (Pool::ErroT == pool.type(erro))
		{
			return erro;
		}
	}
	return pool.getcar(expr);
}

Addr Interpreter::apply_syntax (Addr proc, Addr args, Addr env)
{
	Addr eproc = eval(proc, env);
	if (Pool::ErroT == pool.type(eproc)) { return eproc; }

	return subr_apply(pool.make_list(eproc, args));
}

Addr Interpreter::apply_subr (Addr proc, Addr args)
{
	return (this->*subr[pool.getnum(pool.getatextd(proc, 0))])(args);
}

Addr Interpreter::apply_func (Addr func, Addr args)
{
	Addr expr = pool.getatextd(func, 1);
	Addr bind = bind_tree(pool.getatextd(func, 0), args);
	if (Pool::ErroT == pool.type(bind)) { return bind; }

	return eval(expr, pool.make_cons(bind, pool.getatextd(func, 2)));
}

Addr Interpreter::expand_syntax (Addr expr, Addr env)
{
	root.push_back(env);
	root.push_back(expr);
	for (size_t counter = 0; Pool::ErroT != pool.type(expr); ++counter)
	{
		// TODO
		if ((GC_INTERVAL - 1) == counter % GC_INTERVAL)
		{
			gc();
		}

		Byte typ = pool.type(expr);
		if (Pool::ConsT == typ)
		{
			Addr args = pool.getcdr(expr);
			Addr proc = eval(pool.getcar(expr), env);
			if (Pool::ErroT == pool.type(proc))
			{
				root.pop_back(); // release expr
				root.pop_back(); // release env
				return proc;
			}

			root.push_back(proc);
			Addr proctyp = pool.gettname(proc);
			if (pool.ExtdSubrT == proctyp)
			{
				args = mapeval(args, env);

				root.pop_back(); // release proc
				root.pop_back(); // release expr
				root.pop_back(); // release env

				if (Pool::ErroT == pool.type(args)) { return args; }
				return (this->*subr[
						pool.getnum(pool.getatextd(proc, 0))])(
						args);
			}
			else if (pool.ExtdFuncT == proctyp)
			{
				args = mapeval(args, env);

				root.pop_back(); // release proc
				root.pop_back(); // release expr
				root.pop_back(); // release env

				if (Pool::ErroT == pool.type(args)) { return args; }
				expr = pool.getatextd(proc, 1);
				Addr bind = bind_tree(
						pool.getatextd(proc, 0)
							, args);
				if (Pool::ErroT == pool.type(bind)) { return bind; }
					env = pool.make_cons(bind	
							, pool.getatextd(proc, 2));
//				env = pool.make_cons(zip(pool.getatextd(proc, 0)
//							, args)
//						, pool.getatextd(proc, 2));

				root.push_back(env);
				root.push_back(expr);
			}
			else if (pool.ExtdSpfmT == proctyp)
			{
				Fixnum spfm_id = pool.getnum(pool.getatextd(proc, 0));
				if (spfm_if_id == spfm_id)
				{
					expr = apply_if(
							pool.getcar(args)
							, pool.getcar(pool.getcdr(args))
							, pool.getcar(pool.getcdr(pool.getcdr(args)))
							, env);

					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.push_back(expr);
				}
				else if (spfm_do_id == spfm_id)
				{
					expr = apply_do(args, env);

					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.push_back(expr);
				}
				else if (spfm_syntax_id == spfm_id)
				{
					expr = apply_syntax(pool.getcar(args), pool.getcdr(args), env);

					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.push_back(expr);
				}
				else
				{
					Addr ret = (this->*spfm[spfm_id])(args, env);
					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.pop_back(); // release env
					return ret;

				}
			}
			else
			{
				root.pop_back(); // release proc
				root.pop_back(); // release expr
				root.pop_back(); // release env
				return pool.make_erro(UnCallable
						, pool.make_strn(pool.print(proc)
							+ std::string(" is not callable.")));
			}
		}
		else if (Pool::SymbT == typ)
		{
			root.pop_back(); // release expr
			root.pop_back(); // release env
			Addr val = seekenv(env, expr);
			if (Pool::ErroT == pool.type(val)) { return val; }
			return pool.getcdr(val);
		}
		else
		{
			root.pop_back(); // release expr
			root.pop_back(); // release env
			return expr;
		}
	}
	root.pop_back(); // release expr
	root.pop_back(); // release env
	return expr;
}

Addr Interpreter::expand_quasiquote (Addr expr, Addr env)
{
	if (pool.consp(expr))
	{
		root.push_back(expr);
		Addr head = pool.getcar(expr);
		if (Pool::SymbT == pool.type(head))
		{
			if (pool.equal(pool.make_symb("unquote"), head))
			{
				root.pop_back(); // release expr
				return eval(pool.getcar(pool.getcdr(expr)), env);
			}
		}
		
		Addr eexpr = pool.make_queu();
		root.push_back(eexpr);
		for (Addr rest = expr
				; pool.consp(rest)
				; rest = pool.getcdr(rest))
		{
			Addr e = pool.getcar(rest);
			if (pool.equal(pool.make_symb("splicing"), pool.getcar(e)))
			{
				Addr spexpr = eval(pool.getcar(pool.getcdr(e)), env);
				if (Pool::ErroT == pool.type(spexpr))
				{
					root.pop_back(); // release eexpr
					root.pop_back(); // release expr
					return spexpr;
				}
				root.push_back(spexpr);
				for (Addr rrest = spexpr
						; pool.consp(rrest)
						; rrest = pool.getcdr(rrest))
				{
					pool.pushqueu(eexpr, pool.getcar(rrest));
				}
				root.pop_back(); // release spexpr
			}
			else
			{
				pool.pushqueu(eexpr, expand_quasiquote(e, env));
			}
		}
		root.pop_back(); // release eexpr
		root.pop_back(); // release expr
		return pool.getexit(eexpr);
	}
	return expr;
}

Addr Interpreter::eval (Addr expr)
{
	return eval(expr, genv);
}

Addr Interpreter::eval (Addr expr, Addr env)
{
	root.push_back(env);
	root.push_back(expr);
	for (size_t counter = 0; Pool::ErroT != pool.type(expr); ++counter)
	{
		// TODO
		if ((GC_INTERVAL - 1) == counter % GC_INTERVAL)
		{
			gc();
		}

		Byte typ = pool.type(expr);
		if (Pool::ConsT == typ)
		{
			Addr args = pool.getcdr(expr);
			Addr proc = eval(pool.getcar(expr), env);
			Addr ptyp = pool.type(proc);
			if (Pool::ErroT == ptyp)
			{
				root.pop_back(); // release expr
				root.pop_back(); // release env
				return proc;
			}

			if (Pool::ExtdT != ptyp)
			{
				root.pop_back(); // release expr
				root.pop_back(); // release env
				return pool.make_erro(UnCallable
						, pool.make_strn(pool.print(proc)
							+ std::string(" is not callable.")));
			}

			root.push_back(proc);
			Addr proctyp = pool.gettname(proc);
			if (pool.ExtdSubrT == proctyp)
			{
				args = mapeval(args, env);

				root.pop_back(); // release proc
				root.pop_back(); // release expr
				root.pop_back(); // release env

				if (Pool::ErroT == pool.type(args)) { return args; }
				return apply_subr(proc, args);
			}
			else if (pool.ExtdFuncT == proctyp)
			{
				args = mapeval(args, env);

				root.pop_back(); // release proc
				root.pop_back(); // release expr
				root.pop_back(); // release env

				if (Pool::ErroT == pool.type(args)) { return args; }
				expr = pool.getatextd(proc, 1);
				Addr bind = bind_tree(pool.getatextd(proc, 0), args);
				if (Pool::ErroT == pool.type(bind)) { return bind; }
				env = pool.make_cons(bind, pool.getatextd(proc, 2));

				root.push_back(env);
				root.push_back(expr);
			}
			else if (pool.ExtdSpfmT == proctyp)
			{
				Fixnum spfm_id = pool.getnum(pool.getatextd(proc, 0));
				if (spfm_if_id == spfm_id)
				{
					expr = apply_if(
							pool.getcar(args)
							, pool.getcar(pool.getcdr(args))
							, pool.getcar(pool.getcdr(pool.getcdr(args)))
							, env);

					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.push_back(expr);
				}
				else if (spfm_do_id == spfm_id)
				{
					expr = apply_do(args, env);

					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.push_back(expr);
				}
				else
				{
					Addr ret = (this->*spfm[spfm_id])(args, env);
					root.pop_back(); // release proc
					root.pop_back(); // release expr
					root.pop_back(); // release env
					return ret;

				}
			}
			else
			{
				root.pop_back(); // release proc
				root.pop_back(); // release expr
				root.pop_back(); // release env
				return pool.make_erro(UnCallable
						, pool.make_strn(pool.print(proc)
							+ std::string(" is not callable.")));
			}
		}
		else if (Pool::SymbT == typ)
		{
			root.pop_back(); // release expr
			root.pop_back(); // release env
			Addr val = seekenv(env, expr);
			if (Pool::ErroT == pool.type(val)) { return val; }
			return pool.getcdr(val);
		}
		else
		{
			root.pop_back(); // release expr
			root.pop_back(); // release env
			return expr;
		}
	}
	root.pop_back(); // release expr
	root.pop_back(); // release env
	return expr;
}

std::string Interpreter::printtop (Addr obj)
{
	return print(obj);
}

std::string Interpreter::print (Addr obj)
{
	return pool.print(obj);
}

