#include <sstream>
#include "pool.h"

#ifndef GC_INTERVAL
#define GC_INTERVAL 100
#endif

class Interpreter;
using Subr = Addr (Interpreter::*)(Addr);
using Spfm = Addr (Interpreter::*)(Addr, Addr);

class Interpreter
{
	private:
		Pool pool;

		std::vector<Addr> root; // Addr root;
		Addr genv;

		std::vector<Subr> subr;
		std::vector<Spfm> spfm;

		Fixnum spfm_if_id;
		Fixnum spfm_do_id;
		Fixnum spfm_syntax_id;

		bool find_co_paren (std::istream &, std::stringstream &);
		bool find_co_brackets (std::istream &, std::stringstream &);
		bool take_string (std::istream &, std::stringstream &);
		Addr read_tok (const std::string &);
		Addr wrap_readmacros (Addr, Addr &);
		void growth (Addr, std::stringstream &, Addr &);

		Addr bind_tree (Addr, Addr);
		Addr zip (Addr, Addr);
		Addr mapeval (Addr, Addr);
		// Addr mapsp (Spfm, Addr, Addr);
		// Addr apply (Addr, Addr, Addr);
		Addr apply_if (Addr, Addr, Addr, Addr);
		Addr apply_do (Addr, Addr);
		Addr apply_syntax (Addr, Addr, Addr);
		Addr apply_subr (Addr, Addr);
		Addr apply_func (Addr, Addr);
		Addr seekenv (Addr, Addr);
		Addr expand_syntax (Addr, Addr);
		Addr expand_quasiquote (Addr, Addr);

		Addr subr_eq (Addr);
		Addr subr_cons (Addr);
		Addr subr_car (Addr);
		Addr subr_cdr (Addr);
		Addr subr_atom (Addr);
		Addr subr_equal (Addr);
		Addr subr_rplaca (Addr);
		Addr subr_rplacd (Addr);
		Addr subr_last (Addr);
		Addr subr_nconc (Addr);
		Addr subr_add (Addr);
		Addr subr_sub (Addr);
		Addr subr_mul (Addr);
		Addr subr_div (Addr);
		Addr subr_mod (Addr);
		Addr subr_gt (Addr);
		Addr subr_lt (Addr);
		Addr subr_int (Addr);
		Addr subr_float (Addr);
		Addr subr_list (Addr);
		Addr subr_print (Addr);
		Addr subr_prin (Addr);
		Addr subr_sprint (Addr);
		Addr subr_tolist (Addr);
		Addr subr_load (Addr);
		Addr subr_vect (Addr);
		Addr subr_tovect (Addr);
		Addr subr_getat (Addr);
		Addr subr_setat (Addr);
		Addr subr_throw (Addr);
		Addr subr_symbol (Addr);
		Addr subr_queu (Addr);
		Addr subr_toqueu (Addr);
		Addr subr_pushqueu (Addr);
		Addr subr_popqueu (Addr);
		Addr subr_empty (Addr);
		Addr subr_type (Addr);
		Addr subr_apply (Addr);
		Addr subr_getc (Addr);

		Addr spfm_if (Addr, Addr);
		Addr spfm_quote (Addr, Addr);
		Addr spfm_lambda (Addr, Addr);
		Addr spfm_syntax (Addr, Addr);
		Addr spfm_define (Addr, Addr);
		Addr spfm_setq (Addr, Addr);
		Addr spfm_do (Addr, Addr);
		Addr spfm_and (Addr, Addr);
		Addr spfm_or (Addr, Addr);
		Addr spfm_quasiquote (Addr, Addr);
		Addr spfm_catch (Addr, Addr);
		Addr spfm_environment (Addr, Addr);

	public:
		Interpreter ();

		void gc ();

		Addr read (std::istream &);
		Addr readtop (std::istream &);
		Addr eval (Addr);
		Addr eval (Addr, Addr);
		std::string print (Addr);
		std::string printtop (Addr);
};

