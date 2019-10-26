. ./interpreter.ps1;

function check ($src, $succ)
{
	$res = (leval (lreadtop $src) $genv);
	write-host ("CHECK: $src -> " + (lprint $res));
	if (($succ -ne $null) -and (-not (equal $succ $res)))
	{
		throw ("Fail! except: " + (lprint $succ) + ", but got: " + (lprint $res));
	}
}

function echeck ($src, $eid, $emess)
{
	write-host -nonewline "ECHECK: $src -> ";
	try
	{
		(leval (lreadtop $src) $genv);
	}
	catch
	{
		$ex = $_.exception -split ",";
		$eemess = ($ex[2..($ex.length - 1)] -join ",");
		write-host "<Erro `"$eemess`">";
		if ($eid -ne $ex[1])
		{
			throw "444,expect code: " + $eid + ", but got code: " + $ex[1];
		}
		if ($emess -ne $eemess)
		{
			throw "444,expect mess: " + $emess + ", but got mess: " + $eemess;
		}
		return $nil;
	}
	throw  "4444,not got erro!";
}

check "" $nil;
check "nil"  $nil;
check "()" $nil;
check "(cons 1 2)" (cons 1 2);
check "(car (cdr (cons 1 (cons 2 3))))" 2;
check "(car nil)" $nil;
check "(cdr nil)" $nil;
check "(atom 1)" $t;
check "(atom nil)" $t;
check "(atom (cons 1 2))" $nil;
check "(atom [1 2 3])" $t;
check "(eq 'a 'a)" $t;
check "(eq 'a 'b)" $nil;
check "(eq cons cons)" $t;
check "(eq cons 'cons)" $nil;
check "(eq (cons 1 2) (cons 1 2))" $nil;
check "(eq 'a (car (cons 'a 'b)))" $t;
check "(equal (cons 1 2) (cons 1 2))" $t;
check "(equal (cons 3 2) (cons 1 2))" $nil;
check "(equal 1339 1339)" $t;
check "(equal 3 1)" $nil;
check "(equal 1339 (cons nil 44))" $nil;
check "(equal (cons nil 44) nil)" $nil;
check "(list 5 4 3 2 1)" (list 5 4 3 2 1);
check "(rplaca (cons nil 44) 34)" (cons 34 44);
check "(rplacd (cons 44 55) (cons 3 nil))" (list 44 3);
check "(nconc (list 1 2 3) (list 4 5))" (list 1 2 3 4 5);
check "(/ (+ 71 55) (- (* 2 3) 3))" 42;
check "(/ 3 2)" 1;
check "(/ 3 2.0)" 1.5;
check "(% 9 2)" 1;
check "(+ 1 2 (- 10 3 4) 4 (/ 30 2 4) (* 2 2 2))" 21;
check "(< 1 2 4)" $t;
check "(< 1 2 1)" $nil;
check "(> 3 2 1)" $t;
check "(> 3 2 3)" $nil;
check "(int 2.0)" 2; 
check "(int -555.3)" -555;
check "(int 123)" 123;
check "(float 4)" 4.0;
# check "(float -1555)" -1555.0;
check "(float -15.356)" -15.356;
check "(if nil 40 (if t 42 41))" 42;
check "(if 0 1 2)" 1;
check "(if `"`" 1 2)" 1;
check "(if () 1 2)" 2;
check "(if [] 1 2)" 1;
check "(quote sym)" (new-object symb "sym");
check "(quote (1 a 2 b))" (list 1 (new-object symb "a") 2 (new-object symb "b"));
check "(lambda (n) (+ n 1))" (new-object func((list (new-object symb "n")),`
		(list (new-object symb "+") (new-object symb "n") 1), $genv));
check "((lambda (n) (+ n 1)) 3)" 4;
check "(! (lambda (a op b) (list op a b)) 1 + 2)" 3;
check "(define foo 42) foo" 42;
check "(define bar 32) (setq bar 333) bar" 333;
check "(((lambda (fib) (do (setq fib (lambda (n)`
	(if (> 2 n) 1 (+ (fib (- n 1)) (fib (- n 2)))))) fib)) nil) 10)" 89;
check "(((lambda (fib) (do (setq fib (lambda (n p1 p2)`
	(if (> 2 n) p1 (fib (- n 1) (+ p1 p2) p1)))) fib)) nil) 45 1 1)" 1836311903;
check "(define hello `")[e\\o\`" Wor)d;`") hello" ")[e\o`" Wor)d;";
check "(vect 1 (+ 1 1) 3)" (vect 1 2 3);
check "[]" (vect);
check "[`"abc`" 42 (+ 1 1) () [1 2] 'non]"`
	(vect "abc" 42 2 $nil (vect 1 2) (new-object symb "non"));
check "(queu)" (queu);
check "(queu 1 2 3)" (queu 1 2 3);
check "(pushqueu  (pushqueu (pushqueu (queu) 5) 1) 0)" (queu 5 1 0);
check "(popqueu (pushqueu (pushqueu (pushqueu (queu) 5) 1) 0))" 5;
check "(concqueu (queu 2 5 8) (queu 3 6 9))" (queu 2 5 8 3 6 9);
check "(concqueu (queu 1 4 7) (queu))" (queu 1 4 7);
check "(concqueu (queu) (queu 42))" (queu 42);
check "(last (queu 1 2 3))" (list 3);
check "(to-list `"hello`")" (list 104 101 108 108 111);
check "(to-list 'hello)" (list 104 101 108 108 111);
check "(to-list [1 2 3 4])" (list 1 2 3 4);
check "(define q (queu)) (pushqueu q 1) (pushqueu q 2) (pushqueu q 3) (pushqueu q 4) (to-list q)" (list 1 2 3 4);
check "(to-vect '(1 2 3 4))" (vect 1 2 3 4);
check "(to-vect `"hello`")" (vect 104 101 108 108 111);
check "(to-vect 'hello)" (vect 104 101 108 108 111);
check "(to-vect (pushqueu (pushqueu (queu) 5) 1) 0)" (vect 5 1 0);
check "(to-vect nil)" (vect);
check "(to-queu (list 1 2 3 4))" (queu 1 2 3 4);
check "(to-queu [1 2 3 4])" (queu 1 2 3 4);
check "(to-queu `"hello`")" (queu 104 101 108 108 111);
check "(to-queu 'hello)" (queu 104 101 108 108 111);
check "(to-queu nil)" (queu);
check "(symbol '(104 101 108 108 111))" (new-object symb "hello");
check "(symbol `"abcd`")" (new-object symb "abcd");
check "(symbol [104 101 108 108 111])" (new-object symb "hello");
check "(symbol (pushqueu (pushqueu (pushqueu (queu) 97) 98) 99))" (new-object symb "abc");
check "(symbol nil)" (new-object symb "");
check "(sprint `"a`" 1 (cons 1 2))" "a1(1 . 2)";
check "``(1 2 ,3 ,(+ 2 2) @(if (> 3 1) '(5 6) nil) @(cons 7 ``(8 ,(* 3 3))) 10)" (list 1 2 3 4 5 6 7 8 9 10);
check "'(1 2 3 . 4)" (cons 1 (cons 2 (cons 3 4)));
check "``',(car '(a . d))" (list (new-object symb "quote") (new-object symb "a"));
check "((lambda (head . rest) rest) 1 2 3 4)" (list 2 3 4);
check "((lambda all all) 1 2 3 4)" (list 1 2 3 4);
check "((lambda ((pa (pb pc) pd)) pc) (list 1 (list 2 3) 4))" 3;
check "(load `"mal/matrix.mal`") (matrix::determinant '((3 1 1 2 1) (5 1 3 4 1) (2 0 1 0 3) (1 3 2 1 1) (2 1 5 10 1)))" -292;
echeck "(throw 1 `"an error occured!`")" 1 "an error occured!";
echeck "(do 1 (throw 2 `"an error occured!`") 3)" 2 "an error occured!";
echeck "(if (throw 3 `"an error occured!`") 3 4)" 3 "an error occured!";
check "(if nil (throw 3 `"an error occured!`") 'safe)" (new-object symb "safe");
echeck "(define sym (throw 4 `"an error occured!`"))" 4 "an error occured!";
check "(and 1 'a '(4))" (list 4);
check "(and 1 nil '(4))" $nil;
check "(and 1 nil (throw 1 `"E`"))" $nil;
check "(and)" $t;
check "(or nil () '())" $nil;
check "(or nil 'a '(4))" (new-object symb "a")
check "(or)" $nil;
echeck "ffoo" $erroid["Symbol"] "ffoo is not defined.";
echeck "(" $erroid["Syntax"] "not found close parenthesis.";
echeck ")" $erroid["Syntax"] "found excess close parenthesis.";
echeck "(to-list 3)" $erroid["Type"] "cannot cast 3 to ConsT.";
echeck "(load `"not/exist/path.ext`")" $erroid["FileNotFound"] "not found file: `"not/exist/path.ext`"";
echeck "(load 33)" $erroid["Type"] "cannot apply load to 33";
echeck "(to-vect 33)" $erroid["Type"] "cannot cast 33 to VectT.";
echeck "(symbol 33)" $erroid["Type"] "cannot cast 33 to SymbT.";
echeck "(to-queu 33)" $erroid["Type"] "cannot cast 33 to QueuT.";
echeck "(setq ffoo 33)" $erroid["Symbol"] "ffoo is not defined.";
echeck "`"" $erroid["Syntax"] "not found close double quote.";
echeck "((lambda ((a . b)) a) 1)" $erroid["Syntax"] "cannot bind: ((a . b)) and (1)";
echeck "(3 1)" $erroid["UnCallable"] "3 is not callable.";
echeck "(+ 3 'a)" $erroid["Type"] "cannot add (3 a)";
echeck "(- () 2)" $erroid["Type"] "cannot sub (NIL 2)";
echeck "(* [] 2)" $erroid["Type"] "cannot mul ([] 2)";
echeck "(/ 3 `"a`")" $erroid["Type"] "cannot div (3 `"a`")";
echeck "(% 3 nil)" $erroid["Type"] "cannot mod (3 NIL)";
echeck "(int 'sym)" $erroid["Type"] "cannot cast sym to InumT.";
echeck "(float `"str`")" $erroid["Type"] "cannot cast `"str`" to FnumT.";
check "(empty ())" $t;
check "(empty [])" $t;
check "(empty (queu))" $t;
check "(empty `"`")" $t;
check "(empty '(1))" $nil;
check "(empty [1])" $nil;
check "(empty (pushqueu (queu) 1))" $nil;
check "(empty `"1`")" $nil;
check "(catch (lambda (id mess) (list 'trap id mess)) (do (print 1) (print 2) (throw 55 `"fail!`") (print 3)))" (list (new-object symb "trap") 55 "fail!");
check "(type nil)" (new-object symb "<nil>");
check "(type 1)" (new-object symb "<inum>");
check "(type (cons 1 2))" (new-object symb "<cons>");
check "(type (queu))" (new-object symb "<queu>");
check "(type [1 2])" (new-object symb "<vect>");
check "(type 'foo)" (new-object symb "<symb>");
check "(type `"bar`")" (new-object symb "<strn>");
check "(type type)" (new-object symb "<subr>");
check "(type if)" (new-object symb "<spfm>");
check "(type (lambda (baz) baz))" (new-object symb "<func>");
check "(type 1.2)" (new-object symb "<fnum>");
check "(apply + '(1 4 6 9))" 20;
check "(apply (lambda (a b c d) (list d b a c)) '(1 4 6 9))" (list 9 4 1 6);
check "(define v [1 2 3]) (getat v 2)" 3;
check "(define v [1 2 3]) (setat v 2 44) v" (vect 1 2 44);
check "(eq (getat [1 nil 3] 1) nil)" $t;
check "(if (getat [1 nil 3] 1) 1 2)" 2;
echeck "(getat 5 2)" $erroid["Type"] "cannot apply getat to 5";
echeck "(setat (list 1 2 3) 2 44)" $erroid["Type"] "cannot apply setat to (1 2 3)";
check "``[1 2 ,3 ,(+ 2 2) @(if (> 3 1) '(5 6) nil) @(cons 7 ``(8 ,(* 3 3))) 10]" (vect 1 2 3 4 5 6 7 8 9 10);
check "(to-list `"a\nb\tc\0`")" (list 97 10 98 9 99 0);
check "(load `"mal/test.mal`")" $nil;

check "(reverse (list 1 2 3 4))" (list 4 3 2 1);
check "(append (list 1 2 3 4) (list 5 6 7 8))" (list 1 2 3 4 5 6 7 8);
check "(take (list 1 2 3 4) 2)" (list 1 2);
check "(drop (list 1 2 3 4) 2)" (list 3 4);
check "(ps [math]::pi)" 3.141592;
check "(environment)";

