#include <sstream>
#include <iomanip>
#include <algorithm>

#include "pool.h"

const Byte Pool::IsAlive = 0x80;
const Byte Pool::Marked  = 0x40;
const Byte Pool::Marked_R  = 0xbf;

const Addr Pool::nil = 0;

const Byte Pool::NilT  = 0x00;
const Byte Pool::InumT = 0x01;
const Byte Pool::ConsT = 0x02;
const Byte Pool::QueuT = 0x03;
const Byte Pool::VectT = 0x04;
const Byte Pool::SymbT = 0x05;
const Byte Pool::StrnT = 0x06;
const Byte Pool::PackT = 0x07;
const Byte Pool::PiceT = 0x08;
const Byte Pool::ErroT = 0x09;
const Byte Pool::ExtdT = 0x0a;
const Byte Pool::FnumT = 0x0b;


const Addr Pool::ADDR_MASK   = 0x00ffffff;
const Addr Pool::ADDR_MASK_R = 0xff000000;


Pool::Pool ()
	: pool (new Cell[INITIAL_POOL_SIZE])
	, head (1)
	  , size (INITIAL_POOL_SIZE)
	  , t (Pool::nil)
	  , ExtdSubrT (Pool::nil)
	  , ExtdSpfmT (Pool::nil)
	  , ExtdFuncT (Pool::nil)
{
	memset(pool, 0, sizeof(Cell) * INITIAL_POOL_SIZE);
	pool[Pool::nil].a[7] |= Pool::IsAlive;

	t = make_symb("T");
	ExtdSubrT = make_symb("Subr");
	ExtdSpfmT = make_symb("Spfm");
	ExtdFuncT = make_symb("Func");
}

Pool::~Pool ()
{
	delete[] pool;
}

Addr Pool::extend_pool (Addr newsize)
{
	Cell *newpool = new Cell[newsize];
	if (NULL == newpool) { return Pool::nil; }

	memset(newpool, 0, newsize * sizeof(Cell));
	memcpy(newpool, pool, size * sizeof(Cell));
	delete[] pool;
	pool = newpool;
	size = newsize;
	return t;
}

Addr Pool::next (Addr ptr)
{
	Addr nptr = pool[ptr].eax[0] & Pool::ADDR_MASK;
	if (0 == nptr) { return 1 + ptr; }
	return nptr;
}

Addr Pool::seek (Addr vsize)
{
	Addr ptr = head;
	Addr pre_ptr = Pool::nil;
	while (ptr + vsize < size)
	{
		bool found = true;
		for (Addr cnt = 0; cnt < vsize; ++cnt)
		{
			if (next(ptr + cnt) - (ptr + cnt) != 1)
			{
				found = false; break;
			}
		}

		if (found)
		{
			if (isnil(pre_ptr))
			{
				head = next(ptr + vsize - 1);
			}
			else
			{
				pool[pre_ptr].eax[0] = next(ptr + vsize - 1);
			}
			return ptr;
		}
		pre_ptr = ptr;
		ptr = next(ptr);
	}
	return size;
}

Addr Pool::alloc (Byte typ)
{
	if (head > Pool::ADDR_MASK)
	{
		std::cout << "Error: cannot allocate" << std::endl;
		return Pool::nil;
	}
	if (head >= size)
	{
		if (isnil(extend_pool(2 * size + 1)))
		{
			return Pool::nil;
		}
	}
	Addr ptr = head;
	head = next(head);

	memset(pool + ptr, 0, sizeof(Cell));
	pool[ptr].a[7] |= Pool::IsAlive;
	pool[ptr].a[7] |= typ;

	return ptr;
}

Addr Pool::valloc (Addr vsize)
{
	if (0 == vsize) { return Pool::nil; }
	if (head > Pool::ADDR_MASK)
	{
		std::cout << "Error: cannot allocate" << std::endl;
		return Pool::nil;
	}
	if (head >= size)
	{
		if (isnil(extend_pool(2 * size + 1)))
		{
			return Pool::nil;
		}
	}
	Addr vptr = seek(vsize);
	if (vptr >= size)
	{
		if (isnil(extend_pool(size + 2 * vsize))) { return Pool::nil; }
		vptr = seek(vsize);
	}
	

	memset(pool + vptr, 0, sizeof(Cell) * vsize);
	for (Addr itr = vptr; itr < vptr + vsize; ++itr)
	{
		pool[itr].a[7] |= Pool::IsAlive;
	}

	return vptr;
}

void Pool::mfree (Addr ptr)
{
	memset(pool + ptr, 0, sizeof(Cell));
	pool[ptr].eax[0] |= head;
	head = ptr;
}

void Pool::set (Addr ptr, Cell c)
{
	memcpy(pool + ptr, &c, sizeof(Cell));
}

Cell Pool::get (Addr ptr)
{
	return pool[ptr];
}

void Pool::mark (Addr root)
{
	if (pool[root].a[7] & Pool::Marked) { return; }

	pool[root].a[7] |= Pool::Marked;
	switch (type(root))
	{
		case ConsT:
			mark(getcar(root));
			mark(getcdr(root));
			break;
		case QueuT:
			mark(getexit(root));
			break;
		case VectT:
		case SymbT:
		case StrnT:
			for (Addr itr = getvbegin(root); itr < getvend(root)
					; ++itr)
			{
				mark(itr);
			}
			break;
		case PackT:
			for (Addr itr = getvbegin(root)
					; itr < getpsize(root)
					; ++itr)
			{
				mark(itr);
			}
			break;
		case ErroT:
			mark(getestr(root));
			break;
		case ExtdT:
			mark(gettname(root));
			mark(getfields(root));
			break;
	}
}

void Pool::sweep ()
{
	head = size;
	for (Addr itr = size - 1; itr > Pool::nil; --itr)
	{
		if ((pool[itr].a[7] & Pool::IsAlive)
				and (pool[itr].a[7] & Pool::Marked))
		{
			pool[itr].a[7] &= Pool::Marked_R;
		}
		else
		{
			mfree(itr);
		}
	}
}

Addr Pool::make_inum (Fixnum n)
{
	Addr ptr = alloc(Pool::InumT);
	if (isnil(ptr)) { return Pool::nil; }

	pool[ptr].eax[0] |= n;

	return ptr;
}

Addr Pool::make_cons (Addr a, Addr d)
{
	Addr ptr = alloc(Pool::ConsT);
	if (isnil(ptr)) { return Pool::nil; }

	pool[ptr].eax[0] |= a;
	pool[ptr].eax[1] |= d;

	return ptr;
}

Addr Pool::make_queu ()
{
	Addr ptr = alloc(Pool::QueuT);
	if (isnil(ptr)) { return Pool::nil; }

	pool[ptr].eax[0] |= Pool::nil;
	pool[ptr].eax[1] |= Pool::nil;

	return ptr;
}

Addr Pool::make_vect (Fixnum vsize)
{
	Addr ptr = alloc(Pool::VectT);
	Addr vptr = valloc(vsize);
	if (isnil(ptr) or (Pool::nil == vptr and vsize > 0))
	{
		return Pool::nil;
	}

	pool[ptr].eax[0] |= vptr;
	pool[ptr].eax[1] |= vsize;

	return ptr;
}

Addr Pool::make_symb (std::string str)
{
	Addr ssize = str.size();
	Addr ptr = alloc(Pool::SymbT);
	Addr sptr = valloc(ssize);

	if (isnil(ptr) or (Pool::nil == sptr and ssize > 0))
	{
		return Pool::nil;
	}

	for (size_t idx = 0; idx < ssize; ++idx)
	{
		pool[sptr + idx].a[7] |= Pool::InumT;
		pool[sptr + idx].eax[0] |= (Fixnum)str[idx];
	}

	pool[ptr].eax[0] |= sptr;
	pool[ptr].eax[1] |= ssize;

	return ptr;
}

Addr Pool::make_symb (Addr vect)
{
	Addr ssize = getvsize(vect);

	Addr ptr = alloc(Pool::SymbT);
	Addr sptr = valloc(ssize);
	if (isnil(ptr) or (Pool::nil == sptr and ssize > 0))
	{
		return Pool::nil;
	}

	memcpy(pool + sptr, pool + getvbegin(vect)
			, sizeof(Cell) * ssize);

	pool[ptr].eax[0] |= sptr;
	pool[ptr].eax[1] |= ssize;

	return ptr;
}

Addr Pool::make_strn (std::string str)
{
	Addr ssize = str.size();
	Addr ptr = alloc(Pool::StrnT);
	Addr sptr = valloc(ssize);

	if (isnil(ptr) or (Pool::nil == sptr and ssize > 0))
	{
		return Pool::nil;
	}

	for (size_t idx = 0; idx < ssize; ++idx)
	{
		pool[sptr + idx].a[7] |= Pool::InumT;
		pool[sptr + idx].eax[0] |= (Fixnum)str[idx];
	}

	pool[ptr].eax[0] |= sptr;
	pool[ptr].eax[1] |= ssize;

	return ptr;
}

Addr Pool::make_strn (Addr vect)
{
	Addr ssize = getvsize(vect);

	Addr ptr = alloc(Pool::StrnT);
	Addr sptr = valloc(ssize);
	if (isnil(ptr) or (Pool::nil == sptr and ssize > 0))
	{
		return Pool::nil;
	}

	memcpy(pool + sptr, pool + getvbegin(vect)
			, sizeof(Cell) * ssize);

	pool[ptr].eax[0] |= sptr;
	pool[ptr].eax[1] |= ssize;

	return ptr;
}

Addr Pool::make_pack (Addr length, Byte ptype)
{
	Fixnum psize = 1 + length / 7;
	Addr ptr = alloc(Pool::PackT);
	Addr pptr = valloc(psize);
	if (isnil(ptr) or (Pool::nil == pptr and psize > 0))
	{
		return Pool::nil;
	}

	pool[ptr].a[3] |= ptype;
	pool[ptr].eax[0] |= pptr;
	pool[ptr].eax[1] |= length;

	for (Addr itr = pptr; itr < pptr + psize; ++itr)
	{
		pool[itr].a[7] |= Pool::PiceT;
	}

	return ptr;
}

Addr Pool::make_erro (Fixnum ex, Addr obj)
{
	Addr ptr = alloc(Pool::ErroT);
	if (isnil(ptr)) { return Pool::nil; }

	pool[ptr].eax[0] |= ex;
	pool[ptr].eax[1] |= obj;

	return ptr;
}

Addr Pool::make_extd (Addr tname, Addr cons)
{
	Addr ptr = alloc(Pool::ExtdT);
	if (isnil(ptr)) { return Pool::nil; }

	pool[ptr].eax[0] |= tname;
	pool[ptr].eax[1] |= cons;

	return ptr;
}

// TODO
Addr Pool::make_subr (const std::string &name, Fixnum idx)
{
	return make_extd(ExtdSubrT, make_list(make_inum(idx), make_symb(name)));
}

Addr Pool::make_spfm (const std::string &name, Fixnum idx)
{
	return make_extd(ExtdSpfmT, make_list(make_inum(idx), make_symb(name)));
}

Addr Pool::make_func (Addr args, Addr body, Addr env)
{
	return make_extd(ExtdFuncT, make_list(args, body, env));
}

typedef union FBits
{
	float f;
	Addr i;
} FBits;

Addr Pool::make_fnum (float fnum)
{
	Addr ptr = alloc(Pool::FnumT);
	if (isnil(ptr)) { return Pool::nil; }

	pool[ptr].eax[0] |= (*(FBits*)&fnum).i;

	return ptr;
}

Fixnum Pool::getnum (Addr inum)
{
	return pool[inum].eax[0];
}

float Pool::getfnum (Addr fnum)
{
	return (*(FBits*)&pool[fnum].eax[0]).f;
}

Addr Pool::getcar (Addr cons)
{
	return pool[cons].eax[0] & Pool::ADDR_MASK;
}

Addr Pool::getcdr (Addr cons)
{
	return pool[cons].eax[1] & Pool::ADDR_MASK;
}

Addr Pool::getnth (Addr cons, Fixnum idx)
{
	Addr at = cons;
	for (Fixnum i = idx; i > 0; --i)
	{
		at = getcdr(at);
	}
	return getcar(at);
}

Addr Pool::getentr (Addr queu)
{
	return getcar(queu);
}

Addr Pool::getexit (Addr queu)
{
	return getcdr(queu);
}

Addr Pool::getvbegin (Addr vect)
{
	return getcar(vect);
}

Addr Pool::getvend (Addr vect)
{
	return getvbegin(vect) + getvsize(vect);
}

Addr Pool::getvsize (Addr vect)
{
	return getcdr(vect);
}

Addr Pool::getatvect (Addr vect, Addr idx)
{
	return getvbegin(vect) + idx;
}

Byte Pool::getpackedtype (Addr pack)
{
	return pool[pack].a[3];
}

Addr Pool::getpackedsize (Addr pack)
{
	return getcdr(pack);
}

Addr Pool::getpsize (Addr pack)
{
	return 1 + getpackedsize(pack) / 7;
}

Addr Pool::getatpack (Addr pack, Addr idx)
{
	return make_inum((Fixnum)(
				pool[getatvect(pack, idx / 7)].a[idx % 7]));
}

Fixnum Pool::geteid (Addr erro)
{
	return (Fixnum)getcar(erro);
}

Addr Pool::getestr (Addr erro)
{
	return getcdr(erro);
}

Addr Pool::gettname (Addr extd)
{
	return getcar(extd);
}

Addr Pool::getfields (Addr extd)
{
	return getcdr(extd);
}

Addr Pool::getatextd (Addr extd, Addr idx)
{
	return getnth(getfields(extd), idx);
}

void Pool::setcar (Addr cons, Addr ptr)
{
	pool[cons].eax[0] &= Pool::ADDR_MASK_R;
	pool[cons].eax[0] |= ptr;
}

void Pool::setcdr (Addr cons, Addr ptr)
{
	pool[cons].eax[1] &= Pool::ADDR_MASK_R;
	pool[cons].eax[1] |= ptr;
}

void Pool::setnth (Addr cons, Addr idx, Addr ptr)
{
	Addr at = cons;
	for (Addr i = idx; i > 0; --i)
	{
		at = getcdr(at);
	}
	setcar(at, ptr);
}

void Pool::setentr (Addr queu, Addr ptr)
{
	setcar(queu, ptr);
}

void Pool::setexit (Addr queu, Addr ptr)
{
	setcdr(queu, ptr);
}

void Pool::setatvect (Addr vect, Addr idx, Addr ptr)
{
	memcpy(pool + getatvect(vect, idx), pool + ptr, sizeof(Cell));
}

void Pool::setatpack (Addr pack, Addr idx, Addr ptr)
{
	pool[getatvect(pack, idx / 7)].a[idx % 7] = (Byte)getnum(ptr);
}

void Pool::setatextd (Addr extd, Addr idx, Addr ptr)
{
	setnth(getfields(extd), idx, ptr);
}

void Pool::pushcons (Addr &cons, Addr ptr)
{
	cons = make_cons(ptr, cons);
}

Addr Pool::popcons (Addr &cons)
{
	Addr a = getcar(cons);
	cons = getcdr(cons);
	return a;
}

Addr Pool::nconccons (Addr c1, Addr c2)
{
	Addr last = c1;
	if (isnil(last)) { return c2; }
	for (; not isnil(getcdr(last)); last = getcdr(last)) {}

	setcdr(last, c2);
	return c1;
}

Addr Pool::nreversecons (Addr cons)
{
	Addr rcons = Pool::nil;
	for (Addr ch = cons; not isnil(ch);)
	{
		pushcons(rcons, popcons(ch));
	}
	return rcons;
}

Addr Pool::lengthcons (Addr cons)
{
	Addr cnt = 0;
	for (Addr a = cons; ConsT == type(a)
			; a = getcdr(a))
	{
		++cnt;
	}
	return cnt;
}

void Pool::pushqueu (Addr queu, Addr ptr)
{
	Addr entr = getentr(queu);
	Addr c = make_cons(ptr, Pool::nil);
	if (isnil(entr))
	{
		setentr(queu, c);
		setexit(queu, c);
	}
	else
	{
		setcdr(entr, c);
		setentr(queu, c);
	}
}

Addr Pool::popqueu (Addr queu)
{
	Addr exit = getexit(queu);
	if (isnil(exit))
	{
		return Pool::nil;
	}
	else if (exit == getentr(queu))
	{
		setexit(queu, Pool::nil);
		setentr(queu, Pool::nil);
		return popcons(exit);
	}
	else
	{
		Addr a = popcons(exit);
		setexit(queu, exit);
		return a;
	}
}

Addr Pool::popentrqueu (Addr queu)
{
	Addr exit = getexit(queu);
	if (isnil(exit))
	{
		return Pool::nil;
	}
	else if (exit == getentr(queu))
	{
		setexit(queu, Pool::nil);
		setentr(queu, Pool::nil);
		return popcons(exit);
	}
	else
	{
		Addr a = getcar(getentr(queu));
		Addr pre = exit;
		for (Addr rest = exit
				; ConsT == type(getcdr(rest))
				; rest = getcdr(rest))
		{
			pre = rest;
		}
		setcdr(pre, Pool::nil);
		setentr(queu, pre);
		return a;
	}
}

Addr Pool::pack (Addr coll)
{
	Byte typ = type(coll);
	if (ConsT == typ)
	{
		Addr pack = make_pack(lengthcons(coll), ConsT);
		Addr idx = 0;
		for (Addr rest = coll
				; not isnil(rest)
				; rest = getcdr(rest))
		{
			setatpack(pack, idx, getcar(rest));
			++idx;
		}
		return pack;
	}

	if (QueuT == typ)
	{
		Addr pack = make_pack(lengthcons(getexit(coll)), QueuT);
		Addr idx = 0;
		for (Addr rest = getexit(coll)
				; not isnil(rest)
				; rest = getcdr(rest))
		{
			setatpack(pack, idx, getcar(rest));
			++idx;
		}
		return pack;
	}

	if (VectT == typ)
	{
		Addr pack = make_pack(getvsize(coll), VectT);
		for (Addr idx = 0
				; idx < getvsize(coll)
				; ++idx)
		{
			setatpack(pack, idx, getatvect(coll, idx));
		}
		return pack;
	}

	if (StrnT == typ)
	{
		Addr pack = make_pack(getvsize(coll), StrnT);
		for (Addr idx = 0
				; idx < getvsize(coll)
				; ++idx)
		{
			setatpack(pack, idx, getatvect(coll, idx));
		}
		return pack;
	}

	return make_erro(Type
			, make_strn(print(coll) + std::string(" is not packable.")));
}

Addr Pool::unpack (Addr pack)
{
	Byte typ = getpackedtype(pack);
	if (ConsT == typ)
	{
		Addr queu = make_queu();
		for (Addr idx = 0; idx < getpackedsize(pack); ++idx)
		{
			pushqueu(queu, getatpack(pack, idx));
		}
		return getexit(queu);
	}

	if (QueuT == typ)
	{
		Addr queu = make_queu();
		for (Addr idx = 0; idx < getpackedsize(pack); ++idx)
		{
			pushqueu(queu, getatpack(pack, idx));
		}
		return queu;
	}

	if (VectT == typ)
	{
		Addr vect = make_vect(getpackedsize(pack));
		for (Addr idx = 0; idx < getpackedsize(pack); ++idx)
		{
			setatvect(vect, idx, getatpack(pack, idx));
		}
		return vect;
	}

	if (StrnT == typ)
	{
		Addr vect = make_vect(getpackedsize(pack));
		for (Addr idx = 0; idx < getpackedsize(pack); ++idx)
		{
			setatvect(vect, idx, getatpack(pack, idx));
		}
		return make_strn(vect);
	}

	return Pool::nil;
}

Byte Pool::type (Addr ptr)
{
	return pool[ptr].a[7] & 0x0f;
}

void Pool::print_seek (Addr ptr
		, std::vector<Addr> &printed
		, std::vector<Addr> &duped)
{
	if (std::find(printed.begin(), printed.end(), ptr)
			!= printed.end())
	{
		auto ditr = std::find(duped.begin(), duped.end(), ptr);
		if (ditr == duped.end()) { duped.push_back(ptr); }
		return;
	}

	switch (type(ptr))
	{
		case Pool::ConsT:
			printed.push_back(ptr);
			print_seek(getcar(ptr), printed, duped);
			print_seek(getcdr(ptr), printed, duped);
			break;
		case Pool::QueuT:
			printed.push_back(ptr);
			print_seek(getexit(ptr), printed, duped);
			break;
		case Pool::VectT:
			printed.push_back(ptr);
			for (Addr itr = getvbegin(ptr); itr < getvend(ptr)
					; ++itr)
			{
				print_seek(itr, printed, duped);
			}
			break;
		case Pool::ErroT:
			printed.push_back(ptr);
			print_seek(getestr(ptr), printed, duped);
			break;
		case Pool::ExtdT:
			printed.push_back(ptr);
			if (ExtdFuncT == gettname(ptr))
			{
				print_seek(getnth(getfields(ptr), 0), printed, duped);
				print_seek(getnth(getfields(ptr), 1), printed, duped);
			}
			else
			{
				print_seek(getfields(ptr), printed, duped);
			}
			break;
		default:
			break;
	}
	return;
}

std::string Pool::printcons_rec (Addr car, Addr cdr
		, const std::vector<Addr> &duped)
{
	Byte cdrtype = type(cdr);
	if (NilT == cdrtype)
	{
		return std::string("(")
			+ print_rec(car, duped, RECURSIVE_PRINT)
			+ std::string(")");
	}
	else if (ConsT == cdrtype)
	{
		if (std::find(duped.begin(), duped.end(), cdr)
				!= duped.end())
		{
			return std::string("(")
				+ print_rec(car, duped, RECURSIVE_PRINT)
				+ std::string(" . ")
				+ print_rec(cdr, duped, RECURSIVE_PRINT)
				+ std::string(")");
		}
		else
		{
			return std::string("(")
				+ print_rec(car, duped, RECURSIVE_PRINT)
				+ std::string(" ")
				+ print_rec(cdr, duped, RECURSIVE_PRINT).substr(1);
		}
	}
	else
	{
		return std::string("(")
			+ print_rec(car, duped, RECURSIVE_PRINT)
			+ std::string(" . ")
			+ print_rec(cdr, duped, RECURSIVE_PRINT)
			+ std::string(")");
	}
}

std::string Pool::print_rec (Addr ptr
		, const std::vector<Addr> &duped
		, bool recursive)
{
	std::string str;
	auto itr = std::find(duped.begin(), duped.end(), ptr);
	if (recursive and itr != duped.end())
	{
		return std::string("$")
			+ std::to_string(
					std::distance(duped.begin(), itr));
	}
	else
	{
		std::string vstr;
		switch (type(ptr))
		{
			case Pool::NilT:
				return std::string("NIL");
			case Pool::InumT:
				return std::to_string(getnum(ptr));
			case Pool::ConsT:
				return printcons_rec(getcar(ptr)
						, getcdr(ptr), duped);
			case Pool::QueuT:
				return std::string("/")
					+ print_rec(getexit(ptr), duped
							, RECURSIVE_PRINT)
					+ std::string("/");
			case Pool::VectT:
				if (0 == getvsize(ptr)) { return "[]"; }
				vstr = std::string("");
				for (Addr itr = getvbegin(ptr); itr < getvend(ptr)
						; ++itr)
				{
					vstr += (std::string(" ")
							+ print_rec(itr, duped
								, RECURSIVE_PRINT));
				}
				return std::string("[")
					+ vstr.substr(1) + std::string("]");
			case Pool::SymbT:
				//vstr = std::string("\"");
				for (Addr itr = getvbegin(ptr)
						; itr < getvend(ptr)
						; ++itr)
				{
					vstr += (char)getnum(itr);
				}
				//vstr += "\"";
				return vstr;
			case Pool::StrnT:
				vstr = std::string("\"");
				for (Addr itr = getvbegin(ptr)
						; itr < getvend(ptr)
						; ++itr)
				{
					vstr += (char)getnum(itr);
				}
				vstr += "\"";
				return vstr;
			case Pool::PackT:
				if (0 == getvsize(ptr)) { return ""; }
				vstr = std::string("");
				for (Addr idx = 0
						; idx < getpackedsize(ptr)
						; ++idx)
				{
					vstr += (std::string(" ")
							+ print_rec(getatpack(ptr, idx)
								, duped, RECURSIVE_PRINT));
				}
				return std::string("<Pack [")
					+ vstr.substr(1) + std::string("]>");
			case Pool::PiceT:
				return std::string("<Pice ")
					+ std::to_string(get(ptr).rax & 0x00ffffffffffffff)
					+ std::string(" >");
			case Pool::ErroT:
				return std::string("<Erro ")
					+ print_rec(getestr(ptr), duped
							, RECURSIVE_PRINT)
					+ std::string(">");
			case Pool::ExtdT:
				{
					Addr ttyp = gettname(ptr);
					if (ExtdFuncT == ttyp)
					{
						return std::string("<Func ")
							+ print_rec(getatextd(ptr, 0), duped, RECURSIVE_PRINT)
							+ std::string(" ")
							+ print_rec(getatextd(ptr, 1), duped, RECURSIVE_PRINT)
							+ std::string(">");
					}
					else
					{
						return std::string("<")
							+ print_rec(gettname(ptr), duped, RECURSIVE_PRINT)
							+ std::string(" ")
							+ print_rec(getfields(ptr), duped, RECURSIVE_PRINT)
							+ std::string(">");
					}
				}
			case Pool::FnumT:
				return std::to_string(getfnum(ptr));
			default:
				return std::string("<Not printable type>");
		}
	}
}

std::string Pool::print (Addr ptr)
{
	std::vector<Addr> printed;
	std::vector<Addr> duped;
#if RECURSIVE_PRINT
	print_seek(ptr, printed, duped);
	std::string str = print_rec(ptr, duped, true);

	if (duped.size() > 0)
	{
		std::string dstr("");
		for (size_t idx = 0; idx < duped.size(); ++idx)
		{
			dstr += std::string("$") + std::to_string(idx)
				+ std::string(" = ")
				+ print_rec(duped.at(idx), duped, false)
				+ std::string("\n");
		}
		return dstr + str;
	}
	else
	{
		return str;
	}
#else
	return print_rec(ptr, duped, false);
#endif
}

bool Pool::isnil (Addr ptr)
{
	return NilT == type(ptr);
	// return Pool::nil == ptr;
}

bool Pool::consp (Addr ptr)
{
	return ConsT == type(ptr);
}

bool Pool::equal (Addr a, Addr b)
{
	if (a == b) { return true; }

	Byte typ = type(a);
	if (type(b) != typ) { return false; }

	switch (typ)
	{
		case Pool::NilT:
			return true;
		case Pool::InumT:
			return (getnum(a) == getnum(b));
		case Pool::ConsT:
			return (equal(getcar(a), getcar(b))
					and equal(getcdr(a), getcdr(b)));
		case Pool::QueuT:
			return equal(getexit(a), getexit(b));
		case Pool::VectT:
		case Pool::SymbT:
		case Pool::StrnT:
			if (getvsize(a) != getvsize(b)) { return false; }
			for (Addr idx = 0; idx < getvsize(a); ++idx)
			{
				if (not equal(getatvect(a, idx), getatvect(b, idx)))
				{
					return false;
				}
			}
			return true;
		case Pool::PackT:
			if (getpackedsize(a) != getpackedsize(b)) { return false; }
			for (Addr idx = 0; idx < getpackedsize(a); ++idx)
			{
				if (not equal(getatpack(a, idx), getatpack(b, idx)))
				{
					return false;
				}
			}
			return true;
		case Pool::PiceT:
			return (get(a).rax & 0x00ffffffffffffff)
				== (get(b).rax & 0x00ffffffffffffff);
		case Pool::ErroT:
			return (equal(geteid(a), geteid(b))
					and equal(getestr(a), getestr(b)));
		case Pool::ExtdT:
			return equal(gettname(a), gettname(b))
				and equal(getfields(a), getfields(b));
		case Pool::FnumT:
			return (getfnum(a) == getfnum(b));
		default:
			return false;
	}
}

std::string Pool::dumpcell (Addr ptr)
{
	Cell cell = get(ptr);
	std::stringstream ss;
	for (size_t i = 0; i < 8; ++i)
	{
		ss << std::hex << std::setfill('0')
			<< std::setw(2) << (int)cell.a[i] << " ";
	}
	return ss.str();
}

std::string Pool::dump ()
{
	std::stringstream ss;
	ss << "size: " << size << ", head: " << head << std::endl;
	for (size_t idx = 0; idx < size; ++idx)
	{
		ss << std::setfill('0') << std::setw(4)
			<< std::hex << idx
			<< ": "
			<< dumpcell(idx)
			<< std::endl;
	}
	return ss.str();
}

