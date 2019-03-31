$nil = $False;
$t = new-object symb "T";

class symb
{
	[string] $name;

	symb ([string] $name_)
	{
		$this.name = $name_;
	}
}

class cons
{
	$car;
	$cdr;

	cons ($a, $d)
	{
		$this.car = $a;
		$this.cdr = $d;
	}
}

class subr
{
	$script;
	$name;

	subr ($script_, $name_)
	{
		$this.script = $script_;
		$this.name = $name_;
	}

	[Object] call ($args_)
	{
		return (& $this.script $args_);
	}
}

class spfm
{
	$script;
	$name;

	spfm ($script_, $name_)
	{
		$this.script = $script_;
		$this.name = $name_;
	}

	[Object] call ($args_, $env)
	{
		return (& $this.script $args_ $env);
	}
}

class func
{
	$args_;
	$body;
	$env;

	func ($args_, $body_, $env_)
	{
		$this.args_ = $args_;
		$this.body = $body_;
		$this.env = $env_;
	}

	[Object] call ($args_)
	{
		return (leval $this.body (cons (bind_tree $this.args_ $args_) $this.env));
	}
}


function isnil ($o)
{
	if (($o -is [boolean]) -and ($o -eq $nil)) { return $t; }
	return $nil;
}

function cons ($a, $d)
{
	return (new-object cons($a, $d));
}

function car ($c)
{
	if (isnil $c) { return $nil; }
	return $c.car;
}

function cdr ($c)
{
	if (isnil $c) { return $nil; }
	return $c.cdr;
}

function eq ($a, $b)
{
	if ($a -is [symb] -and $b -is [symb])
	{
		if ($a.name -eq $b.name) { return $t; }
		return $nil;
	}

	if (($a.gettype() -eq $b.gettype()) -and ($a -eq $b)) { return $t; }
	return $nil;
}

function equal ($a, $b)
{
	if ($a -is [symb])
	{
		if (($b -isnot [symb]) -or ($a.name -ne $b.name)) { return $nil; }
		return $t;
	}
	if ($a -is [cons])
	{
		if (($b -isnot [cons])`
				-or (-not (equal (car $a) (car $b)))`
				-or (-not (equal (cdr $a) (cdr $b)))) { return $nil; }
		return $t;
	}
	if ($a -is [subr])
	{
		if (($b -isnot [subr]) -or ($a.name -ne $b.name)) { return $nil; }
		return $t;
	}
	if ($a -is [spfm])
	{
		if (($b -isnot [spfm]) -or ($a.name -ne $b.name)) { return $nil; }
		return $t;
	}
	if ($a -is [func])
	{
		if (($b -isnot [func]) -or (-not (equal $a.args_ $b.args_))`
				-or (-not (equal $a.body $b.body)) -or (-not (equal $a.env $b.env)))
		{
			return $nil;
		}
		return $t;
	}
	if (($a -is [system.collections.arraylist]) -or ($a -is [array]))
	{
		if (($b -is [system.collections.arraylist]) -or ($b -is [array]))
		{
			$len = $a.count;
			if ($len -ne $b.count) { return $nil; }
			for ($idx = 0; $idx -lt $len; ++$idx)
			{
				if (-not (equal $a[$idx] $b[$idx])) { return $nil; }
			}
			return $t;
		}
		return $nil;
	}
	if ($a -eq $b) { return $t; };
	return $nil;
}

function atom ($o)
{
	if ($o -is [cons]) { return $nil; }
	return $t;
}

function list
{
	$c = $nil;
	for ($idx = $args.count - 1; $idx -gt -1; --$idx) { $c = cons ($args[$idx]) $c; }
	return $c;
}

function last ($c)
{
	$rest = $c;
	for ( ; -not (atom (cdr $rest)); $rest = cdr $rest) { ; }
	return $rest;
}

function assoc ($c, $key)
{
	for ($rest = $c; -not (atom $rest); $rest = (cdr $rest))
	{
		if (equal (car (car $rest)) $key) { return (car $rest); }
	}
	return $nil;
}

function seekenv ($env, $key)
{
	for ($rest = $env; -not (atom $rest); $rest = (cdr $rest))
	{
		$record = (assoc (car $rest) $key);
		if (-not (isnil $record)) { return $record; }
	}
	$name = lprint $key;
	throw "SymbolError: $name is not defined.";
}

function rplaca ($c, $val)
{
	$c.car = $val;
	return $c;
}

function rplacd ($c, $val)
{
	$c.cdr = $val;
	return $c;
}

function length ($c)
{
	$len = 0;
	for ($rest = $c; -not (atom $rest); $rest = cdr $rest) { ++$len; }
	return $len;
}

function reverse ($coll)
{
	$rev = $nil;
	for ($rest = $coll; -not (atom $rest); $rest = cdr $rest)
	{
		$rev = cons (car $rest) $rev;
	}
	return $rev;
}

function nconc ($a, $b)
{
	[void](rplacd (last $a) $b);
	return $a;
}

function append1 ($a, $b)
{
	$ret = $b;
	for ($rest = reverse $a; -not (atom $rest); $rest = cdr $rest)
	{
		$ret = cons (car $rest) $ret;
	}
	return $ret;
}

function apply ($proc, $args_)
{
	if ($proc -is [subr])
	{
		return $proc.call($args_);
	}

	if ($proc -is [func])
	{
		return $proc.call($args_);
	}

	if ($proc -is [scriptblock])
	{
		return & $proc (cons2array $args_);
	}

	throw "TypeError: $proc is not appliable.";
}

function vect
{
	[void]($l = new-object system.collections.arraylist);
	foreach ($a in $args) { [void]($l.add($a)); }
	return ,$l;
}

function tolist ($coll)
{
	if (($coll -is [array]) -or ($coll -is [system.collections.arraylist]))
	{
		$list = $nil;
		for ($idx = $coll.count - 1; $idx -ge 0; --$idx)
		{
			[void]($list = cons $coll[$idx] $list);
		}
		return $list;
	}

	if ($coll -is [string])
	{
		$list = $nil;
		$lstr = [system.text.encoding]::ascii.getbytes($coll);
		for ($idx = $coll.length - 1; $idx -ge 0; --$idx)
		{
			[void]($list = cons $lstr[$idx] $list);
		}
		return $list;
	}

	if ($coll -is [symb]) { return (tolist $coll.name); }
	if ($coll -is [cons]) { return $coll; }
	if (isnil $coll) { return $coll; }

	$lobj = lprint $coll;
	throw "TypeError: cannot cast ${lobj} to ConsT.";
}

function tovect ($coll)
{
	if ($coll -is [cons])
	{
		$vec = new-object system.collections.arraylist;
		for ($rest = $coll; -not (atom $rest); $rest = (cdr $rest))
		{
			[void]($vec.add((car $rest)));
		}
		return $vec;
	}
	
	if ($coll -is [string])
	{
		$lstr = [system.text.encoding]::ascii.getbytes($coll);
		$vec = new-object system.collections.arraylist;
		for ($idx = 0; $idx -lt $coll.length; ++$idx)
		{
			[void]($vec.add($lstr[$idx]));
		}
		return $vec;
	}

	if ($coll -is [symb]) { return (tovect $coll.name); }
	if ($coll -is [array]) { return $coll; }
	if ($coll -is [system.collections.arraylist]) { return $coll; }
	if ($coll -eq $nil) { return (vect); }

	$lobj = lprint $coll;
	throw "TypeError: cannot cast ${lobj} to VectT.";
}



function bind_tree ($treea, $treeb)
{
	if (isnil $treea) { return $nil; }
	if (atom $treea) { return list (cons $treea $treeb); }
	if (atom $treeb)
	{
		$sa = lprint $treea;
		$sb = lprint $treeb;
		throw "TypeError: cannot bind $sa and $sb";
	}

	try
	{
		return (nconc (bind_tree (car $treea) (car $treeb))`
			(bind_tree (cdr $treea) (cdr $treeb)));
	}
	catch
	{
		$sa = lprint $treea;
		$sb = lprint $treeb;
		throw "TypeError: cannot bind $sa and $sb";
	}
}

function cons2array ($c)
{
	$arr = @();
	for ($rest = $c; -not (atom $rest); $rest = cdr $rest)
	{
		$arr += car $rest;
	}
	return $arr;
}

function array2cons ($a)
{
	$c = $nil;
	for ($idx = $a.count - 1; $idx -gt -1; --$idx) { $c = (cons $a[$idx] $c); }
	return $c;
}

function growth ([system.collections.arraylist]$tree, $buff)
{
	$buf = $buff[0];
	$rmacs = $buff[1];
	if ($buf)
	{
		$buff[0] = "";
		$buff[1] = $nil;
		$num = 0;
		if ([int]::tryparse($buf, [ref]$num))
		{
			$tree.add((wrap_readmacros $num $rmacs));
		}
		elseif ([double]::tryparse($buf, [ref]$num))
		{
			$tree.add((wrap_readmacros $num $rmacs));
		}
		else
		{
			$tree.add((wrap_readmacros (new-object symb $buf) $rmacs));
		}
	}
	return;
}

function wrap_readmacros ($tree, $rmacs)
{
	$wrapped = $tree;
	for ($rest = $rmacs; -not (atom $rest); $rest = cdr $rest)
	{
		$wrapped = list (car $rest) $wrapped;
	}
	return $wrapped;
}

function find_co_paren ($src)
{
	$sflg = $False;
	$layer = 1;
	$len = $src.length;
	for ($idx = 0; $idx -lt $len; ++$idx)
	{
		$c = $src[$idx];
		if ((-not $sflg) -and ("(" -eq $c)) { $layer += 1; }
		elseif ((-not $sflg) -and (")" -eq $c)) { $layer -= 1; }
		elseif ("\" -eq $c) { $idx += 1 }
		elseif ("`"" -eq $c) { $sflg = -not $sflg; }

		if ($layer -lt 1) { return $idx; }
	}
	throw "SyntaxError: not found close parenthesis.";
}

function find_co_brackets ($src)
{
	$sflg = $False;
	$layer = 1;
	$len = $src.length;
	for ($idx = 0; $idx -lt $len; ++$idx)
	{
		$c = $src[$idx];
		if ((-not $sflg) -and ("[" -eq $c)) { $layer += 1; }
		elseif ((-not $sflg) -and ("]" -eq $c)) { $layer -= 1; }
		elseif ("\" -eq $c) { $idx += 1 }
		elseif ("`"" -eq $c) { $sflg = -not $sflg; }

		if ($layer -lt 1) { return $idx; }
	}
	throw "SyntaxError: not found close brackets.";
}

function take_string ($src)
{
	$strn = "";
	[void]($len = $src.length);
	for ($idx = 0; $idx -lt $len; ++$idx)
	{
		[void]($c = $src[$idx]);
		if ("`"" -eq $c) { return @($strn, $idx); }
		if ("\" -eq $c)
		{
			[void](++$idx);
			[void]($c = $src[$idx]);
			if ("a" -eq $c) { $c = "`a"; }
			elseif ("b" -eq $c) { $c = "`b"; }
			elseif ("f" -eq $c) { $c = "`f"; }
			elseif ("n" -eq $c) { $c = "`n"; }
			elseif ("r" -eq $c) { $c = "`r"; }
			elseif ("t" -eq $c) { $c = "`t"; }
			elseif ("v" -eq $c) { $c = "`v"; }
			elseif ("0" -eq $c) { $c = "`0"; }
		}
		[void]($strn += $c);
	}
	throw "SyntaxError: not found close double quote.";
}

function mapeval ($objs, $env)
{
	$lobjs = lprint $objs;
	write-host "debug: mapeval: $lobjs";
	$eobjs = $nil;
	for ($rest = reverse $objs; -not (atom $rest); $rest = cdr $rest)
	{
		write-host "debug: for";
		$eobjs = cons (leval (car $rest) $env) $eobjs;
	}
	$leobjs = lprint $eobjs;
	write-host "debug: mapeval: end: $leobjs";
	return $eobjs;
}



function lif ($env, $pred, $then, $else)
{
	if (isnil (leval $pred $env)) { return (leval $else $env); }
	return (leval $then $env);
}

function quote ($env, $obj)
{
	return $obj;
}

function lambda ($env, $args_, $body)
{
	return new-object func($args_, $body, $env);
}

function define ($env, $sym, $val)
{
	$record = (assoc (car (last $env)) $sym);
	if (isnil $record)
	{
		rplaca (last $env) (cons (cons $sym (leval $val $env)) (car (last $env)));
	}
	else
	{
		rplacd $record (leval $val $env);
	}

	return $sym;
}

function setq ($env, $sym, $val)
{
	$record = (seekenv $env $sym);
	if (isnil $record)
	{
		$name = lprint $sym;
		throw "SymbolError: $name is not defined.";
	}
	else
	{
		rplacd $record (leval $val $env);
	}

	return $sym;
}

function do ($env, $exprs)
{
	for ($rest = $exprs; -not (atom (cdr $rest)); $rest = cdr $rest)
	{
		leval (car $rest) $env;
	}
	return leval (car (last $exprs)) $env;
}

function syntax ($env, $proc, $exprs)
{
	return leval (apply (leval $proc $env) $exprs $env) $env;
}


function regist_subr ($env, $script, $name)
{
	[void](rplaca $env (cons (cons (new-object symb $name)`
				(new-object subr($script, $name))) (car $env)));
}

function regist_spfm ($env, $script, $name)
{
	[void](rplaca $env (cons (cons (new-object symb $name)`
				(new-object spfm($script, $name))) (car $env)));
}

[void]($genv = (cons $nil $nil));
[void](rplaca $genv (cons (cons (new-object symb "nil") $nil) (car $genv)));
[void](rplaca $genv (cons (cons (new-object symb "NIL") $nil) (car $genv)));
[void](rplaca $genv (cons (cons (new-object symb "t") $t) (car $genv)));
[void](rplaca $genv (cons (cons (new-object symb "T") $t) (car $genv)));
regist_subr $genv { param($args_); return (cons (car $args_) (car (cdr $args_)));} "cons";
regist_subr $genv { param($args_); return (car (car $args_)); } "car";
regist_subr $genv { param($args_); return (cdr (car $args_)); } "cdr";
regist_subr $genv { param($args_); return (atom (car $args_)); } "atom";
regist_subr $genv {param($args_); return (eq (car $args_) (car (cdr $args_))); } "eq";
regist_subr $genv { param($args_);`
	return (equal (car $args_) (car (cdr $args_))); } "equal";
regist_subr $genv { param($args_); return $args_; } "list";
regist_subr $genv { param($args_); return (last (car $args_)); } "last";
regist_subr $genv { param($args_);`
	return (assoc (car $args_) (car (cdr $args_))); } "assoc";
regist_subr $genv { param($args_);`
	return (rplaca (car $args_) (car (cdr $args_))); } "rplaca";
regist_subr $genv { param($args_);`
	return (rplacd (car $args_) (car (cdr $args_))); } "rplacd";
regist_subr $genv { param($args_); return (length (car $args_)); } "length";
regist_subr $genv { param($args_); return (reverse (car $args_)); } "reverse";
regist_subr $genv { param($args_);`
	return (nconc (car $args_) (car (cdr $args_))); } "nconc";
regist_subr $genv { param($args_);`
	$ret = $nil;
	for ($rest = reverse (car $args_); -not (atom $rest); $rest = cdr $rest)
	{
		$ret = append1 (car $rest) $ret;
	};
	return $ret; } "append";
regist_subr $genv { param($args_);`
	return (apply (car $args_) (car (cdr $args_))); } "apply";
regist_subr $genv { param($args_);`
	[void]($arr = new-object system.collections.arraylist);
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		[void]($arr.add((car $rest)));
	}
	return ,$arr;
} "vect";
regist_subr $genv { param($args_);`
	$acc = 0;
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		$acc += (car $rest);
	}
	return $acc;
} "+";
regist_subr $genv { param($args_);`
	if (isnil $args_) { return 0; }
	$acc = car $args_;
	for ($rest = cdr $args_; -not (atom $rest); $rest = cdr $rest)
	{
		$acc -= (car $rest);
	}
	return $acc;
} "-";
regist_subr $genv { param($args_);`
	$acc = 1;
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		$acc *= (car $rest);
	}
	return $acc;
} "*";
regist_subr $genv { param($args_);`
	if (isnil $args_) { return 0; }
	$acc = car $args_;
	$fflg = $nil;
	for ($rest = cdr $args_; -not (atom $rest); $rest = cdr $rest)
	{
		$n = car $rest;
		if ($n -is [double]) { $fflg = $t; }
		if (isnil $fflg)
		{
			$acc = [math]::truncate($acc / $n);
		}
		else
		{
			$acc /= $n;
		}
	}
	return $acc;
} "/";
regist_subr $genv { param($args_); return (car $args_) % (car (cdr $args_))} "%";
regist_subr $genv { param($args_);`
	if (isnil $args_) { return $t; }
	for ($rest = $args_; -not (atom (cdr $rest)); $rest = cdr $rest)
	{
		if (-not ((car $rest) -lt (car (cdr $rest)))) { return $nil; }
	}
	return $t;
} "<";
regist_subr $genv { param($args_);`
	if (isnil $args_) { return $t; }
	for ($rest = $args_; -not (atom (cdr $rest)); $rest = cdr $rest)
	{
		if (-not ((car $rest) -gt (car (cdr $rest)))) { return $nil; }
	}
	return $t;
} ">";
regist_subr $genv { param($args_); return (tolist (car $args_)); } "to-list";
regist_subr $genv { param($args_); return (tovect (car $args_)); } "to-vect";

regist_spfm $genv { param($args_, $env);`
	(lif $env (car $args_) (car (cdr $args_)) (car (cdr (cdr $args_))))} "if";
regist_spfm $genv { param($args_, $env);`
	return (quote $env (car $args_)); } "quote";
regist_spfm $genv { param($args_, $env);`
	return (lambda $env (car $args_) (car (cdr $args_))); } "lambda";
regist_spfm $genv { param($args_, $env);`
	return (define $env (car $args_) (car (cdr $args_))); } "define";
regist_spfm $genv { param($args_, $env);`
	return (setq $env (car $args_) (car (cdr $args_))); } "setq";
regist_spfm $genv { param($args_, $env); return (do $env $args_); } "do";
regist_spfm $genv { param($args_, $env);`
	return (syntax $env (car $args_) (cdr $args_)); } "!";
regist_spfm $genv { param($args_, $env);`
	return iex (car $args_); } "ps";
# TODO


function lreadtop ($src)
{
	return (cons (new-object symb "do") (lread $src));
}

function lread ($src)
{
	$tree = new-object system.collections.arraylist;
	$buff = @("", $nil);
	$len = $src.length;
	for ($idx = 0; $idx -lt $len; ++$idx)
	{
		$c = $src[$idx];
		if (";" -eq $c)
		{
			[void](growth $tree $buff);
			for (; $idx -lt $len;  ++$idx) { if ("`n" -eq $src[$idx]) { break; }; }
		}
		elseif (@(" ", "`t", "`n") -contains $c)
		{
			[void](growth $tree $buff);
		}
		elseif ("(" -eq $c)
		{
			[void](growth $tree $buff);
			[void]($co = find_co_paren $src.substring($idx + 1, $len - $idx - 1));
			[void]($tree.add(`
						(wrap_readmacros (lread $src.substring($idx + 1, $co))`
						 $buff[1])));
			[void]($buff[1] = $nil);
			[void]($idx += $co + 1);
		}
		elseif (")" -eq $c)
		{
			throw "SyntaxError: found excess parenthesis.";
		}
		elseif ("[" -eq $c)
		{
			[void](growth $tree $buff);
			[void]($co = find_co_brackets $src.substring($idx + 1, $len - $idx - 1));
			if ($buff[1])
			{
				[void]($tree.add((list (new-object symb "to-vect")`
						(wrap_readmacros (lread $src.substring($idx + 1, $co))`
						 $buff[1]))));
				[void]($buff[1] = $nil);
			}
			else
			{
				[void]($tree.add((cons (new-object symb "vect")`
					(lread $src.substring($idx + 1, $co)))));
			}
			[void]($idx += $co + 1);
		}
		elseif ("]" -eq $c)
		{
			throw "SyntaxError: found excess brackets.";
		}
		elseif ("." -eq $c)
		{
			if (-not $buff[0])
			{
				rplacd $tree[$tree.count - 1]`
				   	(car (lread $src.substring($idx + 1, $len - $idx - 1)));
				return array2cons $tree;
			}
			$buff[0] += $c;
		}
		elseif ("`"" -eq $c)
		{
			[void](growth $tree $buff);
			[void]($ret = take_string $src.substring($idx + 1, $len - $idx - 1));
			[void]($tree.add($ret[0]));
			[void]($idx += $ret[1] + 1);
		}
		elseif ("'" -eq $c)
		{
			[void](growth $tree $buff);
			$buff[1] = cons (new-object symb "quote") $buff[1];
		}
		elseif ("``" -eq $c)
		{
			[void](growth $tree $buff);
			$buff[1] = cons (new-object symb "quasiquote") $buff[1];
		}
		elseif ("," -eq $c)
		{
			[void](growth $tree $buff);
			$buff[1] = cons (new-object symb "unquote") $buff[1];
		}
		elseif ("@" -eq $c)
		{
			[void](growth $tree $buff);
			$buff[1] = cons (new-object symb "splicing") $buff[1];
		}
		else
		{
			$buff[0] += $c;
		}
	}
	[void](growth $tree $buff);
	return array2cons $tree;
}

function leval ($expr, $env)
{
	while ($True)
	{
		if ($expr -is [cons])
		{
			$proc = leval (car $expr) $env;
			$args_ = cdr $expr;

			if ($proc -is [subr])
			{
				$debug = lprint $proc;
				$ag = lprint $args_;
				write-host "debug: call: $debug $ag";
				$ret = $proc.call((mapeval $args_ $env));
				$lret = lprint $ret;
				write-host "debug: ret: $lret";
				return $ret;
# return $proc.call((mapeval $args_ $env));
			}
			elseif ($proc -is [spfm])
			{
				if ("if" -eq $proc.name)
				{
					if (isnil (leval (car $args_) $env))
					{
						$expr = (car (cdr (cdr $args_)));
					}
					else
					{
						$expr = (car (cdr $args_));
					}
				}
				elseif ("do" -eq $proc.name)
				{
					if (-not $args_) { return $nil; }

					$rest = $args_;
					for (; -not (atom (cdr $rest)); $rest = cdr $rest)
					{
						[void](leval (car $rest) $env);
					}
					$expr = car $rest;
				}
				elseif ("!" -eq $proc.name)
				{
					$eproc = leval (car $args_) $env;
					$expr = apply $eproc (cdr $args_);
				}
				else
				{
					return $proc.call($args_, $env);
				}
			}
			elseif ($proc -is [func])
			{
				$expr = $proc.body;
				$env = cons (bind_tree $proc.args_ (mapeval $args_ $env)) $proc.env;
			}
			elseif ($proc -is [scriptblock])
			{
				return & $proc (cons2array (mapeval $args_ $env))
			}
			else
			{
				throw "TypeError: $proc is not callable.";
			}
		}
		elseif ($expr -is [symb])
		{
			return cdr (seekenv $env $expr);
		}
		else
		{
			return $expr;
		}
	}
	return $expr;
}

function lprint ($obj)
{
	if ($obj -is [symb])
	{
		return $obj.name;
	}
	if ($obj -is [cons])
	{
		return (printcons (car $obj) (cdr $obj));
	}
	if ($obj -is [subr])
	{
		return "<Subr: " + $obj.name + ">";
	}
	if ($obj -is [spfm])
	{
		return "<Spfm: " + $obj.name + ">";
	}
	if ($obj -is [func])
	{
		return "<Func: " + (lprint $obj.args_)  + " " + (lprint $obj.body) + ">";
	}
	if (isnil $obj)
	{
		return "NIL";
	}
	if ($obj -is [string])
	{
		return "`"" + $obj + "`"";
	}
	if (($obj -is [system.collections.arraylist]) -or ($obj -is [array]))
	{
		if ($obj.count -lt 1) { return "[]"; }
		$str = "[";
		foreach ($a in $obj) { $str += ("" + (lprint $a) + " "); }
		return $str.substring(0, $str.length - 1) + "]";
	}

	return $obj;
}

function printcons ($a, $d)
{
	$sa = lprint $a;
	if (isnil $d) { return "($sa)"; }

	$sd = lprint $d;

	if ($d -is [cons])
	{
		return "($sa " + $sd.substring(1, $sd.length - 1);
	}

	return "(" + $sa + " . " + $sd + ")";
}

