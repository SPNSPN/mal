#include "derxi.h"
#include "interpreter.h"

#include <iostream>
#include <sstream>

#define GEN_STREAM(code) __gen_stream(code, sizeof(code))
#define CHECK(expr) __check(expr, __LINE__)
#define ITP_CHECK(code, succ) __itp_check(code, succ, __LINE__)

std::stringstream __gen_stream (char *str, size_t size)
{
	std::stringstream ss;
	ss.write(str, size);
	return ss;
}

void __check (bool succ, size_t line)
{
	if (not succ)
	{
		std::cout << std::endl;
		std::cout << "Fail test at: "
			<< __FILE__ << ":" << line << std::endl;
		exit(1);
	}
}

void __itp_check(const std::string &code, const std::string &succ, size_t line)
{
	Interpreter itp;
	std::cout << "CHECK: interpreter eval "
		<< code << " -> " << std::flush;
	std::stringstream src;
	src << code;
	Addr tree = itp.readtop(src);
	tree = itp.eval(tree);
	std::cout << itp.printtop(tree) << std::endl;
	__check(succ == itp.print(tree), line);
	std::cout << "OK" << std::endl;
}

int main (int argc, char **argv)
{
	std::cout << "CHECK: Pool -> " << std::flush;
	Pool pool;

	Addr nil = Pool::nil;
	Addr inum = pool.make_inum(42);
	Addr cons = pool.make_cons(inum, nil);

	Addr queu = pool.make_queu();
	pool.pushqueu(queu, nil);
	pool.pushqueu(queu, inum);
	pool.pushqueu(queu, cons);

	Addr vect = pool.make_vect(4);
	pool.setatvect(vect, 0, nil);
	pool.setatvect(vect, 1, inum);
	pool.setatvect(vect, 2, cons);
	pool.setatvect(vect, 3, queu);

	Addr vstr = pool.make_vect(5);
	pool.setatvect(vstr, 0, pool.make_inum(104));
	pool.setatvect(vstr, 1, pool.make_inum(101));
	pool.setatvect(vstr, 2, pool.make_inum(108));
	pool.setatvect(vstr, 3, pool.make_inum(108));
	pool.setatvect(vstr, 4, pool.make_inum(111));
	Addr strn = pool.make_strn(vstr);
	Addr packc = pool.pack(cons);
	Addr packs = pool.pack(strn);
	Addr estr = pool.make_strn("test-error");
	Addr erro = pool.make_erro(3, estr);
	Addr extd = pool.make_extd(
			pool.make_symb("parson")
			, pool.make_list(
				pool.make_symb("Alice")
				, pool.make_inum(17)
				, pool.make_list(pool.make_inum(79)
					, pool.make_inum(65)
					, pool.make_inum(82))));


	CHECK("NIL" == pool.print(nil));
	CHECK("42" == pool.print(inum));
	CHECK("(42)" == pool.print(cons));
	CHECK("/(NIL 42 (42))/" == pool.print(queu));
	CHECK("[NIL 42 (42) /(NIL 42 (42))/]" == pool.print(vect));
	CHECK("\"hello\"" == pool.print(strn));
	CHECK("<Pack [42]>" == pool.print(packc));
	CHECK("<Pack [104 101 108 108 111]>" == pool.print(packs));
	CHECK("(42)" == pool.print(pool.unpack(packc)));
	CHECK("\"hello\"" == pool.print(pool.unpack(packs)));
	CHECK("<Erro \"test-error\">" == pool.print(erro));
	CHECK("<parson (Alice 17 (79 65 82))>" == pool.print(extd));

	std::cout << "OK" << std::endl;

	std::stringstream in;
	derxi::VM vm;
	std::cout << "CHECK: op_nil -> " << std::flush;
	{
		char code[] = {
			// (do nil nil nil)
			0x00, 
			0x00, 
			0x00, 
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(NIL NIL NIL)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_cnst -> " << std::flush;
	{
		char code[] = {
			// (do 1 2 3)
			0x02, 0x03, 
			0x02, 0x02, 
			0x02, 0x01, 
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(1 2 3)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_cond -> " << std::flush;
	{
		char code[] = {
			// (do (if nil 1)
			// 	   2
			// 	   (if 1 3))
			0x00,       // nil
			0x03, 0x02, // cond 2
			0x02, 0x01, // cnst 1
			0x02, 0x02, // cnst 2
			0x02, 0x01, // cnst 1
			0x03, 0x02, // cond 2
			0x02, 0x03, // cnst 3
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(3 2)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_jump -> " << std::flush;
	{
		char code[] = {
			// (do nil
			// 	   jump 3
			// 	   nil
			// 	   nil
			// 	   nil
			// 	   nil)
			0x00, 
			0x04, 0x03, 
			0x00, 
			0x00, 
			0x00, 
			0x00, 
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(NIL NIL)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_cons -> " << std::flush;
	{
		char code[] = {
			// (do (cons 40 2)
			//     (cons 9 (cons 8 (cons 7 nil))))
			0x02, 0x28, 
			0x02, 0x02, 
			0x08,       // cons
			0x02, 0x09,
			0x02, 0x08,
			0x02, 0x07,
			0x00,
			0x08,       // cons
			0x08,       // cons
			0x08,       // cons
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("((9 8 7) (40 . 2))" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_car op_cdr -> " << std::flush;
	{
		char code[] = {
			// (do (car (cons 40 2))
			//     (cdr (cons 40 2)))
			0x02, 0x28, 
			0x02, 0x02, 
			0x08,       // cons
			0x09,       // car
			0x02, 0x28, 
			0x02, 0x02, 
			0x08,       // cons
			0x0a,       // cdr
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(2 40)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_list -> " << std::flush;
	{
		char code[] = {
			// (do (list 2 3 5 7))
			0x02, 0x02, 
			0x02, 0x03, 
			0x02, 0x05, 
			0x02, 0x07, 
			0x0b, 0x04, // list 4
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("((2 3 5 7))" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_vect -> " << std::flush;
	{
		char code[] = {
			// (do (vect 1 2 4 9))
			0x02, 0x01, 
			0x02, 0x02, 
			0x02, 0x04, 
			0x02, 0x09,
			0x0c, 0x04, // vect 4
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("([1 2 4 9])" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_symb -> " << std::flush;
	{
		char code[] = {
			// (do 'hello)
			0x02, 0x68, 
			0x02, 0x65, 
			0x02, 0x6c, 
			0x02, 0x6c,
			0x02, 0x6f,
			0x0c, 0x05, // vect 5
			0x0d,       // symb
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(hello)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

//	std::cout << "CHECK: op_expr -> " << std::flush;
//	{
//		char code[] = {
//			// (do '3)
//			0x02, 0x03, 
//			0x0e, 0x02, // expr cnst 3
//		};
//		in = GEN_STREAM(code);
//		vm.read(in);
//		vm.gc();
//		vm.eval();
//		vm.gc();
//		CHECK("(<Expr (2 . 3)>)" == vm.print());
//		vm.initilize();
//	}
//	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_erro -> " << std::flush;
	{
		char code[] = {
			// (do (error 55 "error"))
			0x02, 0x65,
			0x02, 0x72,
			0x02, 0x72,
			0x02, 0x6f,
			0x02, 0x72,
			0x0c, 0x05, // vect 5
			0x0e,       // strn
			0x0f, 0x37, // erro 55
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(<Erro \"error\">)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_func -> " << std::flush;
	{
		char code[] = {
			// (do (lambda (arg1)
			//             (do arg1
			//                 arg1)))
			0x05, 0x0b, // func 11
			0x02, 0x02, // cnst 2
			0x02, 0x02, // cnst 2
			0x01,       // push
			0x02, 0x02, // cnst 2
			0x02, 0x02, // cnst 2
			0x01,       // push
			0x07,       // retn
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("((lambda (2 2 2 2 1 2 2 2 2 1 7)))" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_aply -> " << std::flush;
	{
		char code[] = {
			// (do ((lambda (arg1) (do arg1
			//                         arg1))
			//      5))
			0x05, 0x0b, // func 11
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x07,       // retn
			0x02, 0x05, // cnst 5
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x00,       // nil
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(NIL 5 5)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_rcur -> " << std::flush;
	{
		char code[] = {
			// (do (rcur (lambda (arg1) (do arg1
			//                              arg1))
			//      5)
			//     nil)
			0x05, 0x0b, // func 11
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x07,       // retn
			0x02, 0x05, // cnst 5
			0x0b, 0x01, // list 1
			0x10,       // rcur
			0x00,       // nil 
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(5 5)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: dup -> " << std::flush;
	{
		char code[] = {
			// (do ((lambda (arg1) (arg1 42))
			//       (lambda (arg1) (do arg1
			//                          arg1))))
			0x05, 0x0b, // func 11
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x02, 0x2a, // cnst 42
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x07,       // retn

			0x05, 0x0b, // func 11 /* dup */
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x07,       // retn

			0x0b, 0x01, // list 1
			0x06,       // aply
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(42 42)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: swap -> " << std::flush;
	{
		char code[] = {
			// (do ((lambda (arg1) (arg1 58 42))
			//       (lambda (arg1 arg2) (do arg1
			//                               arg2))))
			0x05, 0x0d, // func 13
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x02, 0x3a, // cnst 58
			0x02, 0x2a, // cnst 42
			0x0b, 0x02, // list 2
			0x06,       // aply
			0x07,       // retn

			0x05, 0x0b, // func 11 /* swap */
			0x02, 0x01, // cnst 1
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x02, 0x00, // cnst 0
			0x02, 0x00, // cnst 0
			0x01,       // push
			0x07,       // retn

			0x0b, 0x01, // list 1
			0x06,       // aply
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(58 42)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_addi op_subi op_muli op_divi op_modi -> "
		<< std::flush;
	{
		char code[] = {
			// (do (add  5 37)
			//     (sub 58 16)
			//     (mul  6  7)
			//     (div 126 3)
			//     (mod 125 83))

			0x02, 0x05, // cnst 5
			0x02, 0x25, // cnst 38
			0x12,       // addi
			0x02, 0x3a, // cnst 58
			0x02, 0x10, // cnst 16
			0x13,       // subi
			0x02, 0x06, // cnst 6
			0x02, 0x07, // cnst 7
			0x14,       // muli
			0x02, 0x7e, // cnst 126
			0x02, 0x03, // cnst 3
			0x15,       // divi
			0x02, 0x7d, // cnst 125
			0x02, 0x53, // cnst 83
			0x16        // modi
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(42 42 42 42 42)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_setq op_rpla op_rpld -> " << std::flush;
	{
		char code[] = {
			// (let ((a 1) (b (cons 2 3)))
			//      (let ((c (cons a b)))
			//           (do (setf (cdr b) 4)
			//               (setf (car b) 7)
			//               (setf a 5)
			//               (list c b a))))
			//->
			// ((lambda (a b)
			//          ((lambda (c)
			//                   (do (rplacd b 4)
			//                       (rplaca b 7)
			//                       (setq a 5)
			//                       (list c b a)))
			//            (cons a b)))
			//   1 (cons 2 3))
			0x05, 0x38, // func 56
			0x05, 0x27, // func 39

			0x02, 0x01,
			0x02, 0x01,
			0x01,       // push 1 1 /* b */
			0x02, 0x04, // cnst 4
			0x18,       // rpld

			0x02, 0x01,
			0x02, 0x01,
			0x01,       // push 1 1 /* b */
			0x02, 0x07, // cnst 7
			0x17,       // rpla

			0x02, 0x05,
			0x02, 0x00,
			0x02, 0x01,
			0x11,       // setq 5 0 1 /* set a 5 */

			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* a */
			0x02, 0x01,
			0x02, 0x01,
			0x01,       // push 1 1 /* b */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* c */
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* a */
			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 1 0 /* b */
			0x08,       // cons
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x07,       // retn

			0x02, 0x01, // cnst 1
			0x02, 0x02,
			0x02, 0x03,
			0x08,       // cons 2 3
			0x0b, 0x02, // list 2
			0x06,       // aply

		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("$0 = (7 . 4)\n((1 . $0) $0 5)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: op_def op_ref -> " << std::flush;
	{
		char code[] = {
			// (do (define a 42)
			//     a
			//     ((lambda (b) a) 44))
			0x02, 0x61,
			0x0c, 0x01, // vect 1
			0x0d,       // symb
			0x02, 0x2a, // cnst 42
			0x22,       // def
			0x02, 0x61,
			0x0c, 0x01, // vect 1
			0x0d,       // symb
			0x23,       // ref
			0x05, 0x07, // func 7
			0x02, 0x61,
			0x0c, 0x01, // vect 1
			0x0d,       // symb
			0x23,       // ref
			0x07,       // retn
			0x02, 0x2c, // cnst 44
			0x0b, 0x01, // list 1
			0x06,       // aply
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(42 42)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "CHECK: closure -> " << std::flush;
	{
		char code[] = {
			// (let ((a 0))
			//      (let ((cnt (lambda ()
			//                          (do (setq a (add a 1))
			//                              a))))
			//           (do (cnt)
			//               (cnt)
			//               (cnt)
			//               (cnt)
			//               (cnt))))
			//->
			// (do ((lambda (cnt)
			//              (do (cnt)
			//                  (cnt)
			//                  (cnt)
			//                  (cnt)
			//                  (cnt)))
			//       ((lambda (a)
			//                (lambda ()
			//                        (do (setq a (add a 1))
			//                            a)))
			//                0)))
			0x05, 0x24, // func 36
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* cnt */
			0x00,       // nil
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* cnt */
			0x00,       // nil
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* cnt */
			0x00,       // nil
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* cnt */
			0x00,       // nil
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* cnt */
			0x00,       // nil
			0x06,       // aply
			0x07,       // retn

			0x05, 0x16, // func 22
			0x05, 0x13, // func 19
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* a */
			0x02, 0x01, // cnst 1
			0x12,       // addi
			0x02, 0x00,
			0x02, 0x01,
			0x11,       // setq 0 1 /* a */
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* a */
			0x07,       // retn
			0x07,       // retn

			0x02, 0x00, // cnst 0
			0x0b, 0x01, // list 1
			0x06,       // aply

			0x0b, 0x01, // list 1
			0x06,       // aply
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(5 4 3 2 1)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;
	
	std::cout << "CHECK: fact -> " << std::flush;
	{
		char code[] = {
			// (let ((fact (lambda (n)
			//                     (if (< n 2)
			//                         n
			//                         (* n (fact (- n 1)))))))
			//  (do (fact 1)
			//      (fact 5)
			//      (fact 10)))
			//->
			// ((lambda (fact)
			//          (do (fact 1)
			//              (fact 5)
			//              (fact 10)))
			//  ((lambda (fact)
			//     (do (setq fact (lambda (n)
			//          (if (< n 2)
			//              n
			//              (* n (fact (- n 1))))))
			//         fact))
			//     nil))
			//
			0x05, 0x1f, // func 31
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fact */
			0x02, 0x01, // cnst 1
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fact */
			0x02, 0x05, // cnst 5
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fact */
			0x02, 0x0a, // cnst 10
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x07,       // retn

			0x05, 0x34, // func 52

			0x05, 0x27, // func 39
			0x02, 0x02, // cnst 2
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x21,       // gt
			0x03, 0x06, // cond 6
			// if (< n 2)
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x07,       // retn
			// else
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* fact */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x02, 0x01, // cnst 1
			0x13,       // subi
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x14,       // muli
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* fact */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fact */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply

			0x0b, 0x01, // list 1
			0x06,       // aply

		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("(3628800 120 1)" == vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: fib range map eq equal -> "
		<< std::flush;
	{
		char code[] = {
			// (let ((fib (lambda (n) (if (< n 2) 1
			//                               (+ (fib (- n 2))
			//                                  (fib (- n 1)))))))
			//      (map fib (range 0 14)))
			//->
			// ((lambda (map range)
			//          ((lambda (fib)
			//                   (map fib (range 0 14)))
			//           (lambda (n) (if (< n 2) 1
			//                           (+ (fib (- n 2))
			//                              (fib (- n 1)))))))
			//  ((lambda (map)
			//           (do (setq map
			//                     (lambda
			//                      (fn coll)
			//                      (if coll
			//                          (cons (fn (car coll))
			//                                (map fn
			//                                     (cdr coll)))
			//                          nil)))
			//               map))
			//   nil)
			//  ((lambda (range)
			//           (do (setq range
			//                     (lambda (b e)
			//                             (if (equal b e)
			//                                 (cons e nil)
			//                                 (cons b
			//                                       (range
			//                                       (addi b 1)
			//                                       e)))))
			//               range))
			//   nil))
			0x05, 0x57, // func 87
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* map */

			0x05, 0x3c, // func 60 /* fib gen */
			0x05, 0x2f, // func 47 /* fib */
			0x02, 0x02, // cnst 2
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x21,       // gt
			0x03, 0x03, // cond 3
			// then
			0x02, 0x01, // cnst 1
			0x07,       // retn
			// else
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* fib */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x02, 0x02, // cnst 2
			0x13,       // subi
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* fib */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* n */
			0x02, 0x01, // cnst 1
			0x13,       // subi
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x12,       // addi
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* fib */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fib */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply /* fib gen */

			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 1 0 /* range */
			0x02, 0x00, // cnst 0
			0x02, 0x0e, // cnst 14
			0x0b, 0x02, // list 2
			0x06,       // aply /* range */
			0x0b, 0x02, // list 2
			0x06,       // aply /* map */
			0x07,       // retn

			0x05, 0x3b, // func 59 /* map gen */
			0x05, 0x2e, // func 46 /* map */
			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 1 0 /* coll */
			0x00,       // nil
			0x19,       // eq
			0x03, 0x02, // cond 2
			// then
			0x00,       // nil
			0x07,       // retn
			// else
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fn */
			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 1 0 /* coll */
			0x09,       // car
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* map */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* fn */
			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 1 0 /* coll */
			0x0a,       // cdr
			0x0b, 0x02, // list 2
			0x06,       // aply
			0x08,       // cons
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* map */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* map */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply /* map gen */

			0x05, 0x3e, // func 62 /* range gen */
			0x05, 0x31, // func 49 /* range */

			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* b */
			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 0 0 /* e */
			0x20,       // equal
			0x03, 0x08, // cond 8
			// then
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* b */
			0x00,       // nil
			0x08,       // cons
			0x07,       // retn
			// else
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* b */
			0x02, 0x00,
			0x02, 0x01,
			0x01,       // push 0 1 /* range */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* b */
			0x02, 0x01, // cnst 1
			0x12,       // addi
			0x02, 0x01,
			0x02, 0x00,
			0x01,       // push 1 0 /* e */
			0x0b, 0x02, // list 2
			0x06,       // aply
			0x08,       // cons
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* range */
			0x02, 0x00,
			0x02, 0x00,
			0x01,       // push 0 0 /* range */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply /* range gen */

			0x0b, 0x02, // list 2
			0x06,       // aply
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("((1 1 2 3 5 8 13 21 34 55 89 144 233 377 610))"
				== vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;

	std::cout << "CHECK: push00 push10 push20 push01 push11 push21 -> "
		<< std::flush;
	{
		char code[] = {
			// (let ((fib (lambda (n) (if (< n 2) 1
			//                               (+ (fib (- n 2))
			//                                  (fib (- n 1)))))))
			//      (map fib (range 0 14)))
			//->
			// ((lambda (map range)
			//          ((lambda (fib)
			//                   (map fib (range 0 14)))
			//           (lambda (n) (if (< n 2) 1
			//                           (+ (fib (- n 2))
			//                              (fib (- n 1)))))))
			//  ((lambda (map)
			//           (do (setq map
			//                     (lambda
			//                      (fn coll)
			//                      (if coll
			//                          (cons (fn (car coll))
			//                                (map fn
			//                                     (cdr coll)))
			//                          nil)))
			//               map))
			//   nil)
			//  ((lambda (range)
			//           (do (setq range
			//                     (lambda (b e)
			//                             (if (equal b e)
			//                                 (cons e nil)
			//                                 (cons b
			//                                       (range
			//                                       (addi b 1)
			//                                       e)))))
			//               range))
			//   nil))
			0x05, 0x37, // func 55
			0x24,       // push00 /* map */

			0x05, 0x24, // func 36 /* fib gen */
			0x05, 0x1b, // func 27 /* fib */
			0x02, 0x02, // cnst 2
			0x24,       // push00 /* n */
			0x21,       // gt
			0x03, 0x03, // cond 3
			// then
			0x02, 0x01, // cnst 1
			0x07,       // retn
			// else
			0x27,       // push01 /* fib */
			0x24,       // push00 /* n */
			0x02, 0x02, // cnst 2
			0x13,       // subi
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x27,       // push01 /* fib */
			0x24,       // push00 /* n */
			0x02, 0x01, // cnst 1
			0x13,       // subi
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x12,       // addi
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* fib */
			0x24,       // push00 /* fib */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply /* fib gen */

			0x25,       // push10 /* range */
			0x02, 0x00, // cnst 0
			0x02, 0x0e, // cnst 14
			0x0b, 0x02, // list 2
			0x06,       // aply /* range */
			0x0b, 0x02, // list 2
			0x06,       // aply /* map */
			0x07,       // retn

			0x05, 0x1f, // func 31 /* map gen */
			0x05, 0x16, // func 22 /* map */
			0x25,       // push10 /* coll */
			0x00,       // nil
			0x19,       // eq
			0x03, 0x02, // cond 2
			// then
			0x00,       // nil
			0x07,       // retn
			// else
			0x24,       // push00 /* fn */
			0x25,       // push10 /* coll */
			0x09,       // car
			0x0b, 0x01, // list 1
			0x06,       // aply
			0x27,       // push01 /* map */
			0x24,       // push00 /* fn */
			0x25,       // push10 /* coll */
			0x0a,       // cdr
			0x0b, 0x02, // list 2
			0x06,       // aply
			0x08,       // cons
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* map */
			0x24,       // push00 /* map */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply /* map gen */

			0x05, 0x1e, // func 30 /* range gen */
			0x05, 0x15, // func 21 /* range */

			0x24,       // push00 /* b */
			0x25,       // push10 /* e */
			0x20,       // equal
			0x03, 0x04, // cond 4
			// then
			0x24,       // push00 /* b */
			0x00,       // nil
			0x08,       // cons
			0x07,       // retn
			// else
			0x24,       // push00 /* b */
			0x27,       // push01 /* range */
			0x24,       // push00 /* b */
			0x02, 0x01, // cnst 1
			0x12,       // addi
			0x25,       // push10 /* e */
			0x0b, 0x02, // list 2
			0x06,       // aply
			0x08,       // cons
			0x07,       // retn

			0x02, 0x00,
			0x02, 0x00,
			0x11,       // setq 0 0 /* range */
			0x24,       // push00 /* range */
			0x07,       // retn

			0x00,       // nil
			0x0b, 0x01, // list 1
			0x06,       // aply /* range gen */

			0x0b, 0x02, // list 2
			0x06,       // aply
		};
		in = GEN_STREAM(code);
		vm.read(in);
		vm.gc();
		vm.eval();
		vm.gc();
		std::cout << vm.print() << std::endl;
		CHECK("((1 1 2 3 5 8 13 21 34 55 89 144 233 377 610))"
				== vm.print());
		vm.initilize();
	}
	std::cout << "OK" << std::endl;
	std::cout << std::endl;


	derxi::Compiler cmp;

	std::string code = "(a b c()(d) ( ((e f) (g(h)i ((j)))) k))";
	std::cout << "CHECK: compiler parse "
		<< code << " -> " << std::flush;
	{
		std::stringstream src;
		src << code;
		Addr tree = cmp.parse(src);
		cmp.gc(tree);
		std::cout << cmp.print(tree) << std::endl;
		CHECK("((a b c NIL (d) (((e f) (g (h) i ((j)))) k)))"
				== cmp.print(tree));
		cmp.clean();
	}
	std::cout << "OK" << std::endl;

	Interpreter itp;
	code = "(a b c()(d) ( ((e f) (g(h)i ((j)))) k))";
	std::cout << "CHECK: interpreter read "
		<< code << " -> " << std::flush;
	{
		std::stringstream src;
		src << code;
		Addr tree = itp.read(src);
		std::cout << itp.printtop(tree) << std::endl;
		CHECK("((a b c NIL (d) (((e f) (g (h) i ((j)))) k)))"
				== itp.print(tree));
	}
	std::cout << "OK" << std::endl;

	ITP_CHECK("", "NIL");
	ITP_CHECK("nil", "NIL");
	ITP_CHECK("()", "NIL");
	ITP_CHECK("(cons 1 2)", "(1 . 2)");
	ITP_CHECK("(car (cdr (cons 1 (cons 2 3))))", "2");
	ITP_CHECK("(car nil)", "NIL");
	ITP_CHECK("(cdr nil)", "NIL");
	ITP_CHECK("(atom 1)", "T");
	ITP_CHECK("(atom nil)", "T");
	ITP_CHECK("(atom (cons 1 2))", "NIL");
	ITP_CHECK("(atom [1 2 3])", "T");
	ITP_CHECK("(eq 'a 'a)", "T");
	ITP_CHECK("(eq 'a 'b)", "NIL");
	ITP_CHECK("(eq cons cons)", "T");
	ITP_CHECK("(eq cons 'cons)", "NIL");
	ITP_CHECK("(eq (cons 1 2) (cons 1 2))", "NIL");
	ITP_CHECK("(eq 'a (car (cons 'a 'd)))", "T");
	ITP_CHECK("(equal (cons 1 2) (cons 1 2))", "T");
	ITP_CHECK("(equal (cons 3 2) (cons 1 2))", "NIL");
	ITP_CHECK("(equal 1339 1339)", "T");
	ITP_CHECK("(equal 3 1)", "NIL");
	ITP_CHECK("(equal 1339 (cons nil 44))", "NIL");
	ITP_CHECK("(equal (cons nil 44) nil)", "NIL");
	ITP_CHECK("(list 5 4 3 2 1)", "(5 4 3 2 1)");
	ITP_CHECK("(rplaca (cons nil 44) 34)", "(34 . 44)");
	ITP_CHECK("(rplacd (cons 44 55) (cons 3 nil))", "(44 3)");
	ITP_CHECK("(last (list 3 4 5 6))", "(6)");
	ITP_CHECK("(nconc (list 1 2 3) (list 4 5))", "(1 2 3 4 5)");
	ITP_CHECK("(/ (+ 71 55) (- (* 2 3) 3))", "42");
	ITP_CHECK("(/ 3 2)", "1");
	ITP_CHECK("(/ 3 2.0)", "1.500000");
	ITP_CHECK("(% 9 2)", "1");
	ITP_CHECK("(+ 1 2 (- 10 3 4) 4 (/ 30 2 4) (* 2 2 2))", "21");
	ITP_CHECK("(< 1 2 4)", "T");
	ITP_CHECK("(< 3 2 4)", "NIL");
	ITP_CHECK("(> 3 2 1)", "T");
	ITP_CHECK("(> 3 5 1)", "NIL");
	ITP_CHECK("(if nil 40 (if t 42 41))", "42");
	ITP_CHECK("(if 0 1 2)", "1");
	ITP_CHECK("(if \"\" 1 2)", "1");
	ITP_CHECK("(if () 1 2)", "2");
	ITP_CHECK("(if [] 1 2)", "1");
	ITP_CHECK("(quote sym)", "sym");
	ITP_CHECK("(quote (1 a 2 b))", "(1 a 2 b)");
	ITP_CHECK("(lambda (n) (+ n 1))", "<Func (n) (+ n 1)>");
	ITP_CHECK("((lambda (n) (+ n 1)) 3)", "4");
	ITP_CHECK("(! (lambda (a op b) (list op a b)) 1 + 2)", "3");
	ITP_CHECK("(define foo 42) foo", "42");
	ITP_CHECK("(define bar 32) (setq bar 333) bar", "333");
	ITP_CHECK("(((lambda (fib) (do (setq fib (lambda (n) (if (> 2 n) 1 (+ (fib (- n 1)) (fib (- n 2)))))) fib)) nil) 10)"
			, "89");
	ITP_CHECK("(((lambda (fib) (do (setq fib (lambda (n p1 p2) (if (> 2 n) p1 (fib (- n 1) (+ p1 p2) p1)))) fib)) nil) 45 1 1)"
			, "1836311903");
	ITP_CHECK("(define hello \")[e\\\\\\\\o\\\" Wor)d;\") hello"
			, "\")[e\\\\o\" Wor)d;\"");
	ITP_CHECK("(vect 1 (+ 1 1) 3)", "[1 2 3]");
	ITP_CHECK("[]", "[]");
	ITP_CHECK("[\"abc\" 42 (+ 1 1) () [1 2] 'non]" , "[\"abc\" 42 2 NIL [1 2] non]");
	ITP_CHECK("(queue)", "/NIL/");
	ITP_CHECK("(queue 1 2 3)", "/(1 2 3)/");
	ITP_CHECK("(pushqueue (pushqueue (pushqueue (queue) 5) 1) 0)"
			, "/(5 1 0)/");
	ITP_CHECK("(popqueue (pushqueue (pushqueue (pushqueue (queue) 5) 1) 0))"
			, "5");
	ITP_CHECK("(last (queue 1 2 3))", "(3)");
	ITP_CHECK("(to-list \"hello\")", "(104 101 108 108 111)");
	ITP_CHECK("(to-list 'hello)", "(104 101 108 108 111)");
	ITP_CHECK("(to-list [1 2 3 4])", "(1 2 3 4)");
	ITP_CHECK("(define queu (queue)) (pushqueue queu 1) (pushqueue queu 2) (pushqueue queu 3) (pushqueue queu 4) (to-list queu)"
			, "(1 2 3 4)");
	ITP_CHECK("(to-list nil)", "NIL");
	ITP_CHECK("(to-vect '(1 2 3 4))", "[1 2 3 4]");
	ITP_CHECK("(to-vect \"hello\")", "[104 101 108 108 111]");
	ITP_CHECK("(to-vect 'hello)", "[104 101 108 108 111]");
	ITP_CHECK("(to-vect (pushqueue (pushqueue (pushqueue (queue) 5) 1) 0))"
			, "[5 1 0]");
	ITP_CHECK("(to-vect nil)", "[]");
	ITP_CHECK("(to-queue (list 1 2 3 4))", "/(1 2 3 4)/");
	ITP_CHECK("(to-queue [1 2 3 4])", "/(1 2 3 4)/");
	ITP_CHECK("(to-queue \"hello\")", "/(104 101 108 108 111)/");
	ITP_CHECK("(to-queue 'hello)", "/(104 101 108 108 111)/");
	ITP_CHECK("(to-queue nil)", "/NIL/");
	ITP_CHECK("(symbol '(104 101 108 108 111))", "hello");
	ITP_CHECK("(symbol \"abcd\")", "abcd");
	ITP_CHECK("(symbol [104 101 108 108 111])", "hello");
	ITP_CHECK("(symbol (pushqueue (pushqueue (pushqueue (queue) 97) 98) 99))"
			, "abc");
	ITP_CHECK("(sprint \"a\" 1 (cons 1 2))", "\"a1(1 . 2)\"");
	ITP_CHECK("`(1 2 ,3 ,(+ 2 2) @(if (> 3 1) '(5 6) nil) @(cons 7 `(8 ,(* 3 3))) 10)"
			, "(1 2 3 4 5 6 7 8 9 10)");
	ITP_CHECK("'(1 2 3 . 4)", "(1 2 3 . 4)");
	ITP_CHECK("`',(car '(a . d))", "(quote a)");
	ITP_CHECK("((lambda (head . rest) rest) 1 2 3 4)" , "(2 3 4)");
	ITP_CHECK("((lambda all all) 1 2 3 4)" , "(1 2 3 4)");
	ITP_CHECK("((lambda ((pa (pb pc) pd)) pc) (list 1 (list 2 3) 4))" , "3");
	ITP_CHECK("(load \"mal/matrix.mal\") (matrix::determinant '((3 1 1 2 1) (5 1 3 4 1) (2 0 1 0 3) (1 3 2 1 1) (2 1 5 10 1)))"
			, "-292");
	ITP_CHECK("(throw 1 \"an error occured!\")"
			, "<Erro \"an error occured!\">");
	ITP_CHECK("(do 1 (throw 2 \"an error occured!\") 3)"
			, "<Erro \"an error occured!\">");
	ITP_CHECK("(if (throw 3 \"an error occured!\") 3 4)"
			, "<Erro \"an error occured!\">");
	ITP_CHECK("(if nil (throw 3 \"an error occured!\") 'safe)", "safe");
	ITP_CHECK("(define sym (throw 4 \"an error occured!\"))"
			, "<Erro \"an error occured!\">");
	ITP_CHECK("(and 1 'a '(4))", "(4)");
	ITP_CHECK("(and 1 nil '(4))", "NIL");
	ITP_CHECK("(and 1 nil (throw 1 \"E\"))", "NIL");
	ITP_CHECK("(and)", "T");
	ITP_CHECK("(or nil () '())", "NIL");
	ITP_CHECK("(or nil 'a '(4))", "a");
	ITP_CHECK("(or nil 1 (throw 1 \"E\"))", "1");
	ITP_CHECK("(or)", "NIL");
	ITP_CHECK("ffoo", "<Erro \"ffoo is not defined.\">");
	ITP_CHECK("(", "<Erro \"not found close parenthesis.\">");
	ITP_CHECK(")", "<Erro \"found excess close parenthesis.\">");
	ITP_CHECK("(to-list 3)", "<Erro \"cannot cast 3 to ConsT.\">");
	ITP_CHECK("(load \"not/exist/path.ext\")"
			, "<Erro \"not found file: \"not/exist/path.ext\"\">");
	ITP_CHECK("(load 33)", "<Erro \"cannot apply load to 33\">");
	ITP_CHECK("(to-vect 33)", "<Erro \"cannot cast 33 to VectT.\">");
	ITP_CHECK("(symbol 33)", "<Erro \"cannot cast 33 to SymbT.\">");
	ITP_CHECK("(to-queue 33)", "<Erro \"cannot cast 33 to QueuT.\">");
	ITP_CHECK("(setq ffoo 33)", "<Erro \"ffoo is not defined.\">");
	ITP_CHECK("\"", "<Erro \"not found close double quote.\">");
	ITP_CHECK("((lambda ((a . b)) a) 1)"
			, "<Erro \"cannot bind: ((a . b)) and (1)\">");
	ITP_CHECK("(3 1)", "<Erro \"3 is not callable.\">");
	ITP_CHECK("(+ 3 'a)", "<Erro \"cannot add (3 a)\">");
	ITP_CHECK("(- () 2)", "<Erro \"cannot sub (NIL 2)\">");
	ITP_CHECK("(* [] 2)", "<Erro \"cannot mul ([] 2)\">");
	ITP_CHECK("(/ 3 \"a\")", "<Erro \"cannot div (3 \"a\")\">");
	ITP_CHECK("(% 3 nil)", "<Erro \"cannot mod (3 NIL)\">");
	ITP_CHECK("(empty ())", "T");
	ITP_CHECK("(empty [])", "T");
	ITP_CHECK("(empty (queue))", "T");
	ITP_CHECK("(empty \"\")", "T");
	ITP_CHECK("(empty '(1))", "NIL");
	ITP_CHECK("(empty [1])", "NIL");
	ITP_CHECK("(empty (pushqueue (queue) 1))", "NIL");
	ITP_CHECK("(empty \"1\")", "NIL");
	ITP_CHECK("(catch (lambda (id mess) (list 'trap id mess)) (do (print 1) (print 2) (throw 55 \"fail!\") (print 3) 'end))"
			, "(trap 55 \"fail!\")");
	ITP_CHECK("(type nil)", "<nil>");
	ITP_CHECK("(type 1)", "<inum>");
	ITP_CHECK("(type (cons 1 2))", "<cons>");
	ITP_CHECK("(type (queue))", "<queu>");
	ITP_CHECK("(type [1 2])", "<vect>");
	ITP_CHECK("(type 'foo)", "<symb>");
	ITP_CHECK("(type \"bar\")", "<strn>");
	ITP_CHECK("(type type)", "<subr>");
	ITP_CHECK("(type if)", "<spfm>");
	ITP_CHECK("(type (lambda (baz) baz))", "<func>");
	ITP_CHECK("(type 1.2)", "<fnum>");
	ITP_CHECK("(apply + '(1 4 6 9))", "20");
	ITP_CHECK("(apply (lambda (a b c d) (list d b a c)) '(1 4 6 9))"
			, "(9 4 1 6)");
	ITP_CHECK("(define v [1 2 3]) (getat v 2)", "3");
	ITP_CHECK("(define v [1 2 3]) (setat v 2 44) v", "[1 2 44]");
	ITP_CHECK("(eq (getat [1 nil 3] 1) nil)", "T");
	ITP_CHECK("(if (getat [1 nil 3] 1) 1 2)", "2");
	ITP_CHECK("`[1 2 ,3 ,(+ 2 2) @(if (> 3 1) '(5 6) nil) @(cons 7 `(8 ,(* 3 3))) 10]"
			, "[1 2 3 4 5 6 7 8 9 10]");
	ITP_CHECK("(to-list \"a\\nb\\tc\\0\")", "(97 10 98 9 99 0)");
	ITP_CHECK("(load \"mal/test.mal\")", "NIL");


	return 0;
}

