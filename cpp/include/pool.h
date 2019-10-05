#ifndef __INCLUDE__POOL_H__
#define __INCLUDE__POOL_H__

#include <memory.h>
#include <iostream>
#include <string>
#include <vector>

/* compile option */
#ifndef INITIAL_POOL_SIZE
#define INITIAL_POOL_SIZE 100
#endif

#ifndef RECURSIVE_PRINT
#define RECURSIVE_PRINT false
#endif

/* compile option end */

using Byte = uint8_t;
using Wide = uint16_t;
using Addr = uint32_t;
using Full = uint64_t;
using Fixnum = int32_t;

typedef union Cell
{
	Full rax;
	Addr eax[2];
	Wide  ax[4];
	Byte   a[8];
} Cell;

typedef enum ErroId
{
	FullMemory, UnknownOpcode, OutOfEnvironment,
	Type, Symbol, Syntax,
	UnCallable, ArgsUnmatch, UnEvaluatable, FileNotFound,
} ErroId;


class Pool
{
	// private:
	public:
		Cell *pool;
		Addr head;
		Addr size;

		Addr extend_pool (Addr);
		Addr next (Addr);
		Addr seek (Addr);

		Addr alloc (Byte);
		Addr valloc (Addr);
		void mfree (Addr);

		void print_seek (Addr
				, std::vector<Addr> &, std::vector<Addr> &);
		//std::string printcons_raw (Addr, Addr);
		std::string printcons_rec (Addr, Addr
				, const std::vector<Addr> &);

	public:
		Pool ();
		~Pool ();

		void mark (Addr);
		void sweep ();

		void set (Addr, Cell);
		Cell get (Addr);

		Addr make_inum (Fixnum);
		Addr make_cons (Addr, Addr);
		Addr make_queu ();
		Addr make_vect (Fixnum);
		Addr make_symb (Addr);
		Addr make_symb (std::string);
		Addr make_strn (Addr);
		Addr make_strn (std::string);
		Addr make_pack (Addr, Byte);
		Addr make_erro (Fixnum, Addr);
		Addr make_fnum (float);
		Addr make_extd (Addr, Addr);

		Addr make_subr (const std::string &, Fixnum);
		Addr make_spfm (const std::string &, Fixnum);
		Addr make_func (Addr, Addr, Addr);

		inline Addr make_list () { return Pool::nil; }
		template <typename Car, typename... Cdr>
		Addr make_list (Car a, Cdr... d)
		{
			return make_cons(a, make_list(d...));
		}

		Fixnum getnum (Addr);
		float getfnum (Addr);
		Addr getcar (Addr);
		Addr getcdr (Addr);
		Addr getnth (Addr, Fixnum);
		Addr getentr (Addr);
		Addr getexit (Addr);
		Addr getvbegin (Addr);
		Addr getvend (Addr);
		Addr getvsize (Addr);
		Addr getatvect (Addr, Addr);
		Byte getpackedtype (Addr);
		Addr getpackedsize (Addr);
		Addr getpsize (Addr);
		Addr getatpack (Addr, Addr);
		Fixnum geteid (Addr);
		Addr getestr (Addr);
		Addr gettname (Addr);
		Addr getfields (Addr);
		Addr getatextd (Addr, Addr);

		void setcar (Addr, Addr);
		void setcdr (Addr, Addr);
		void setnth (Addr, Addr, Addr);
		void setentr (Addr, Addr);
		void setexit (Addr, Addr);
		void setatvect (Addr, Addr, Addr);
		void setatpack (Addr, Addr, Addr);
		void setatextd (Addr, Addr, Addr);

		void pushcons (Addr &, Addr);
		Addr popcons (Addr &);
		Addr nconccons (Addr, Addr);
		Addr nreversecons (Addr);
		Addr lengthcons (Addr);
		void pushqueu (Addr, Addr);
		Addr popqueu (Addr);
		void concqueu (Addr, Addr);
		Addr popentrqueu (Addr);
		Addr pack (Addr);
		Addr unpack (Addr);

		Byte type (Addr);
		std::string print (Addr);
		//std::string print_raw (Addr);
		std::string print_rec (Addr, const std::vector<Addr> &, bool);
		bool isnil (Addr);
		bool consp (Addr);
		bool equal (Addr, Addr);
		std::string dumpcell (Addr);
		std::string dump ();

		static const Byte IsAlive;
		static const Byte Marked;
		static const Byte Marked_R;

		static const Addr nil;
		Addr t;

		static const Byte NilT;
		static const Byte InumT;
		static const Byte ConsT;
		static const Byte QueuT;
		static const Byte VectT;
		static const Byte SymbT;
		static const Byte StrnT;
		static const Byte PackT;
		static const Byte PiceT;
		static const Byte ErroT;
		static const Byte ExtdT;
		static const Byte FnumT;

		Addr ExtdSubrT;
		Addr ExtdSpfmT;
		Addr ExtdFuncT;

		static const Addr ADDR_MASK;
		static const Addr ADDR_MASK_R;
};

#endif /* __INCLUDE__POOL_H__ */
