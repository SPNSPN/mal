. ./interpreter.ps1;

function check ($src, $succ)
{
	$res = (leval (lreadtop $src) $genv);
	$resp = lprint $res;
	write-host "CHECK: $src -> $resp";
	if (-not (equal $succ $res)) { throw "Fail!"; }
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
check "(< 3 2 4)" $nil;
check "(> 3 2 1)" $t;
check "(> 3 5 1)" $nil;
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
check "[]" $null;
check "[`"abc`" 42 (+ 1 1) () [1 2] 'non]"`
	(vect "abc" 42 2 $nil (vect 1 2) (new-object symb "non"));
check "(to-list `"hello`")" (list 104 101 108 108 111);
check "(to-list 'hello)" (list 104 101 108 108 111);
check "(to-list [1 2 3 4])" (list 1 2 3 4);
check "(to-vect '(1 2 3 4))" (vect 1 2 3 4);
check "(to-vect `"hello`")" (vect 104 101 108 108 111);
check "(to-vect 'hello)" (vect 104 101 108 108 111);
check "(to-vect nil)" (vect);
check "(symbol '(104 101 108 108 111))" (new-object symb "hello");
check "(symbol `"abcd`")" (new-object symb "abcd");
check "(symbol [104 101 108 108 111])" (new-object symb "hello");
check "(sprint `"a`" 1 (cons 1 2))" "a1(1 . 2)";
check "``(1 2 ,3 ,(+ 2 2) @(if (> 3 1) '(5 6) nil) @(cons 7 `(8 ,(* 3 3))) 10)" (list 1 2 3 4 5 6 7 8 9 10);
check "'(1 2 3 . 4)" (append (list 1 2) (cons 3 4));
check "``',(car '(a . d))" (list (new-object symb "quote") (new-object symb "a"));
check "((lambda (head . rest) rest) 1 2 3 4)" (list 2 3 4);
check "((lambda all all) 1 2 3 4)" (list 1 2 3 4);
check "((lambda (pa (pb pc) pd) pc) (list 1 (list 2 3) 4))" 3;
check "(load `"lisp/matrix.mal`") (matrix::determinant `((3 1 1 2 1) (5 1 3 4 1) (2 0 1 0 3) (1 3 2 1 1) (2 1 5 10 1)))" -292;


