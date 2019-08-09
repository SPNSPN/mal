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

class vect
{
	$siz;
	$len;
	$mem;

	vect ()
	{
		$this.siz = 10;
		$this.len  = 0;
		$this.mem = @($False) * $this.siz;
	}

	[void] extend ()
	{
		$newsiz = $this.siz * 2;
		$this.mem += @($False) * $this.siz;
		$this.siz = $newsiz;
	}

	[void] push ($val)
	{
		if ($this.siz -le $this.len) { $this.extend(); }
		$this.mem[$this.len] = $val;
		++$this.len;
	}

	[Object] pop ()
	{
		--$this.len;
		return $this.mem[$this.len + 1];
	}

	[Object] getat ($idx)
	{
		return $this.mem[$idx];
	}

	[void] setat ($idx, $val)
	{
		$this.mem[$idx] = $val;
	}
}

class queu
{
	$entr;
	$exit;

	queu ()
	{
		$this.entr = $False;
		$this.exit = $this.entr;
	}

	[queu] push ($val)
	{
		$c = (cons $val $False);
		if (isnil $this.entr)
		{
			$this.entr = $c;
			$this.exit = $c;
		}
		else
		{
			[void](rplacd $this.entr $c);
			$this.entr = $c;
		}
		return $this;
	}

	[object] pop ()
	{
		if (isnil $this.exit) { return $False; }
		$e = car $this.exit;
		if ($this.entr -eq $this.exit)
		{
			$this.entr = $False;
			$this.exit = $False;
		}
		else
		{
			$this.exit = cdr $this.exit;
		}
		return $e;
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

$erroid = @{
	FullMemory = 0;
	UnknownOpcode = 1;
	OutOfEnvironment = 2;
	Type = 3;
	Symbol = 4;
	Syntax = 5;
	UnCallable = 6;
	ArgsUnmatch = 7;
	UnEvaluatable = 8;
	FileNotFound = 9};


function isnil ($o)
{
	if (($o -is [boolean]) -and ($o -eq $nil)) { return $t; }
	return $nil;
}

function isnum ($o)
{
	if (($o -is [int]) -or ($o -is [decimal]) -or ($o -is [byte]) -or ($o -is [double]) -or ($o -is [float])) { return $t; }
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
	if ($a -is [vect])
	{
		if (($b -isnot [vect]) -or (-not (equal $a.mem $b.mem)))
		{
			return $nil;
		}
		return $t;
	}
	if ($a -is [queu])
	{
		if (($b -isnot [queu]) -or (-not (equal $a.exit $b.exit)))
		{
			return  $nil;
		}
		return $t;
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
	if ($c -is [cons])
	{
		$rest = $c;
		for ( ; -not (atom (cdr $rest)); $rest = cdr $rest) { ; }
		return $rest;
	}
	if ($c -is [queu])
	{
		return $c.entr;
	}
	lthrow $erroid["Type"] ("cannot apply last to " + (lprint $c));
}

function assoc ($c, $key)
{
	for ($rest = $c; -not (atom $rest); $rest = (cdr $rest))
	{
		if (equal (car (car $rest)) $key) { return (car $rest); }
	}
	return $nil;
}

function lthrow ($eid, $emess)
{
	throw ("," + $eid + "," + $emess);
}

function empty ($coll)
{
	if ($coll -eq $null) { return $t; }
	if (isnil $coll) { return $t; }
	if (($coll -is [string]) -and ($coll.length -lt 1)) { return $t; }
	if (($coll -is [queu]) -and (isnil $coll.exit)) { return $t; }
	if (($coll -is [symb]) -and ($coll.name.length -lt 1)) { return $t; }
	if ((($coll -is [array]) -or ($coll -is [system.collections.arraylist])) -and ($coll.count -lt 1))
	{
		return $t;
	}
	if (($coll -is [vect]) -and ($coll.len -lt 1)) { return $t; }
	return $nil;
}

function ltype ($o)
{
	if ($o -is [cons]) { return (new-object symb "<cons>"); }
	if ($o -is [func]) { return (new-object symb "<func>"); }
	if ($o -is [spfm]) { return (new-object symb "<spfm>"); }
	if ($o -is [subr]) { return (new-object symb "<subr>"); }
	if ($o -is [symb]) { return (new-object symb "<symb>"); }
	if ($o -is [string]) { return (new-object symb "<strn>"); }
	if (($o -is [int]) -or ($o -is [decimal])) { return (new-object symb "<inum>"); }
	if (($o -is [float]) -or ($o -is [double])) { return (new-object symb "<fnum>"); }
	if (isnil $o) { return (new-object symb "<nil>"); }
	if ($o -is [vect]) { return (new-object symb "<vect>"); }
	if (($o -is [array]) -or ($o -is [system.collections.arraylist])) { return (new-object symb "<array>"); }
	if ($o -is [queu]) { return (new-object symb "<queu>"); }
	return $nil;
}

function getat ($vect, $idx)
{
	if ($vect -is [vect])
	{
		return $vect.getat($idx);
	}

	if (($vect -is [array]) -or ($vect -is [system.collections.arraylist]) -or ($vect -is [string]))
	{
		return $vect[$idx];
	}

	if ($vect -is [symb])
	{
		return $vect.name[$idx];
	}

	lthrow $erroid["Type"] ("cannot apply getat to " + (lprint $vect));
}

function setat ($vect, $idx, $val)
{
	if ($vect -is [vect])
	{
		$vect.setat($idx, $val);
		return $vect;
	}

	if (($vect -is [array]) -or ($vect -is [system.collections.arraylist]))
	{
		$vect[$idx] = $val;
		return $vect;
	}

	if ($vect -is [string])
	{
		$vect = $vect.substring(0, $idx) + $val + $vect.substring($idx + 1);
		return $vect;
	}

	if ($vect -is [symb])
	{
		$vect.name[$idx] = $val;
		return $vect;
	}

	lthrow $erroid["Type"] ("cannot apply setat to " + (lprint $vect));
}

function seekenv ($env, $key)
{
	for ($rest = $env; -not (atom $rest); $rest = (cdr $rest))
	{
		$record = (assoc (car $rest) $key);
		if (-not (isnil $record)) { return $record; }
	}
	lthrow $erroid["Symbol"] ((lprint $key) + " is not defined.");
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
	if ($c -is [cons])
	{
		$len = 0;
		for ($rest = $c; -not (atom $rest); $rest = cdr $rest) { ++$len; }
		return $len;
	}
	if (($c -is [array]) -or ($c -is [system.collections.arraylist]))
	{
		return $c.count;
	}
	if ($c -is [vect])
	{
		return  $c.len;
	}
	if ($c -is [queu])
	{
		return (length $c.exit);
	}

	lthrow $erroid["Type"] ("cannot apply length to " + (lprint $c));
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

function append
{
	$list = $nil;
	for ($idx = $args.count - 1; $idx -gt -1; --$idx)
	{
		[void]($list = append1 $args[$idx] $list);
	}
	return $list;
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

	lthrow $erroid{"Type"} ($proc + " is not appliable.");
}

function vect
{
	[void]($v = new-object vect);
	foreach ($a in $args) { [void]($v.push($a)); }
	return $v;
}

function queu
{
	return (toqueu $args);
}

function toarray ($coll)
{
	if ($coll -is [vect])
	{
		if ($coll.len -lt 1) { return ,@(); }
		return $coll.mem[0..($coll.len - 1)];
	}

	lthrow $erroid["Type"] ("cannot cast " + (lprint $coll) + " to Array.");
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

	if ($coll -is [vect])
	{
		return (tolist (toarray $coll));
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

	if ($coll -is [queu]) { return $coll.exit; }
	if ($coll -is [symb]) { return (tolist $coll.name); }
	if ($coll -is [cons]) { return $coll; }
	if (isnil $coll) { return $coll; }

	lthrow $erroid["Type"] ("cannot cast " + (lprint $coll) + " to ConsT.");
}

function tovect ($coll)
{
	if ($coll -is [cons])
	{
		$vec = new-object vect;
		for ($rest = $coll; -not (atom $rest); $rest = (cdr $rest))
		{
			[void]($vec.push((car $rest)));
		}
		return $vec;
	}
	
	if ($coll -is [string])
	{
		$lstr = [system.text.encoding]::ascii.getbytes($coll);
		$vec = new-object vect;
		for ($idx = 0; $idx -lt $coll.length; ++$idx)
		{
			[void]($vec.push($lstr[$idx]));
		}
		return $vec;
	}

	if (($coll -is [array]) -or ($coll -is [system.collections.arraylist]))
	{
		$vec = new-object vect;
		for ($idx = 0; $idx -lt $coll.length; ++$idx)
		{
			[void]($vec.push($coll[$idx]));
		}
		return $vec;
	}
	if ($coll -is [queu]) { return (tovect $coll.exit); }
	if ($coll -is [symb]) { return (tovect $coll.name); }
	if ($coll -is [vect]) { return $coll; }
	if (isnil $coll) { return (vect); }

	lthrow $erroid["Type"] ("cannot cast " + (lprint $coll) + " to VectT.");
}

function toqueu ($coll)
{
	if ($coll -is [cons])
	{
		$q = new-object queu;
		$q.exit = $coll;
		$q.entr = last $coll;
		return $q;
	}
	if (($coll -is [array]) -or ($coll -is [system.collections.arraylist]))
	{
		$q = new-object queu;
		foreach ($a in $coll) { [void]($q.push($a)); }
		return $q;
	}
	if ($coll -is [string])
	{
		$lstr = [system.text.encoding]::ascii.getbytes($coll);
		$q = new-object queu;
		for ($idx = 0; $idx -lt $coll.length; ++$idx)
		{
			[void]($q.push($lstr[$idx]));
		}
		return $q;
	}
	if ($coll -is [symb]) { return (toqueu $coll.name); }
	if ($coll -is [vect]) { return (toqueu (toarray $coll)); }
	if ($coll -is [queu]) { return $coll; }
	if (isnil $coll) { return (queu); }

	lthrow $erroid["Type"] ("cannot cast " + (lprint $coll) + " to QueuT.");
}

function symbol ($coll)
{
	if ($coll -is [cons])
	{
		$str = "";
		for ($rest = $coll; -not (atom $rest); $rest = (cdr $rest))
		{
			$str += [char](car $rest)
		}
		return new-object symb $str;
	}

	if (($coll -is [array]) -or ($coll -is [system.collections.arraylist]))
	{
		$str = "";
		$len = $coll.count;
		for ($idx = 0; $idx -lt $len; ++$idx)
		{
			$str += [char]($coll[$idx]);
		}
		return new-object symb $str;
	}

	if ($coll -is [string]) { return new-object symb $coll; }
	if ($coll -is [symb]) { return $coll; }
	if ($coll -is [vect]) { return (symbol (toarray $coll)); }
	if ($coll -is [queu]) { return (symbol $coll.exit); }
	if ($coll -eq $nil) { return new-object symb ""; }

	lthrow $erroid["Type"] ("cannot cast " + (lprint $coll) + " to SymbT.");
}

function sprint ($args_)
{
	$str = "";
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		$obj = car $rest;
		if ($obj -is [string])
		{
			$str += $obj;
		}
		else
		{
			$str += lprint $obj;
		}
	};
	return $str;
}

function load ($path)
{
	if ($path -isnot [string])
	{
		lthrow $erroid["Type"] ("cannot apply load to " + (lprint $path));
	}

	if (-not (test-path $path))
	{
		lthrow $erroid["FileNotFound"] ("not found file: " + (lprint $path));
	}

	return (leval (cat -raw $path | readtop_pipe) $genv);
}


function bind_tree ($treea, $treeb)
{
	if (isnil $treea) { return $nil; }
	if (atom $treea) { return list (cons $treea $treeb); }
	if (atom $treeb)
	{
		lthrow $erroid["Syntax"] ("cannot bind: " + (lprint $treea) + " and " + (lprint $treeb));
	}

	try
	{
		return (nconc (bind_tree (car $treea) (car $treeb))`
			(bind_tree (cdr $treea) (cdr $treeb)));
	}
	catch
	{
		lthrow $erroid["Syntax"] ("cannot bind: " + (lprint $treea) + " and " + (lprint $treeb));
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
	lthrow $erroid["Syntax"] "not found close parenthesis.";
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
	lthrow $erroid["Syntax"] "not found close brackets.";
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
	lthrow $erroid["Syntax"] "not found close double quote.";
}

function mapeval ($objs, $env)
{
	$eobjs = $nil;
	for ($rest = reverse $objs; -not (atom $rest); $rest = cdr $rest)
	{
		$eobjs = cons (leval (car $rest) $env) $eobjs;
	}
	return $eobjs;
}

$getc_buf = (cons "" $nil);
function getc ()
{	if (-not (car $getc_buf))
	{
		[void](rplaca $getc_buf (tolist ((read-host) + "`n")));
	}

	[void]($c = car (car $getc_buf));
	[void](rplaca $getc_buf (cdr (car $getc_buf)));
	return $c;
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
		[void](rplaca (last $env) (cons (cons $sym (leval $val $env)) (car (last $env))));
	}
	else
	{
		[void](rplacd $record (leval $val $env));
	}

	return $sym;
}

function setq ($env, $sym, $val)
{
	$record = (seekenv $env $sym);
	if (isnil $record)
	{
		lthrow $erroid["Symbol"] ((lprint $sym) + " is not defined.");
	}
	else
	{
		[void](rplacd $record (leval $val $env));
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

function expand_quasiquote ($expr, $env)
{
	if (atom $expr) { return $expr; }
	if (((car $expr) -is [symb]) -and ("unquote" -eq (car $expr).name))
	{
		return (leval (car (cdr $expr)) $env);
	}

	$eexpr = new-object system.collections.arraylist;
	for ($rest = $expr; -not (atom $rest); $rest = cdr $rest)
	{
		$e = car $rest;
		if ((-not (atom $e)) -and ((car $e) -is [symb]) -and ("splicing" -eq (car $e).name))
		{
			for ($sexpr = (leval (car (cdr $e)) $env); -not (atom $sexpr); $sexpr = cdr $sexpr)
			{
				[void]($eexpr.add((car $sexpr)));
			}
		}
		else
		{
			[void]($eexpr.add((expand_quasiquote $e $env)));
		}
	}
	return (tolist $eexpr);
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
	[void]($vec = new-object vect);
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		[void]($vec.push((car $rest)));
	}
	return $vec;
} "vect";
regist_subr $genv { param($args_);`
	[void]($queu = new-object queu);
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		[void]($queu.push((car $rest)));
	}
	return $queu;
} "queu";
regist_subr $genv { param($args_);
	[void]($q = car $args_);
	[void]$q.push((car (cdr $args_)));
	return $q; } "pushqueu";
regist_subr $genv { param($args_);
	return (car $args_).pop(); } "popqueu";
regist_subr $genv { param($args_);`
	$acc = 0;
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		if (-not (isnum (car $rest)))
		{
			lthrow $erroid["Type"] ("cannot add " + (lprint $args_));
		}
		$acc += (car $rest);
	}
	return $acc;
} "+";
regist_subr $genv { param($args_);`
	if (isnil $args_) { return 0; }
	if (-not (isnum (car $args_)))
	{
		lthrow $erroid["Type"] ("cannot sub " + (lprint $args_));
	}
	$acc = car $args_;
	for ($rest = cdr $args_; -not (atom $rest); $rest = cdr $rest)
	{
		if (-not (isnum (car $rest)))
		{
			lthrow $erroid["Type"] ("cannot sub " + (lprint $args_));
		}
		$acc -= (car $rest);
	}
	return $acc;
} "-";
regist_subr $genv { param($args_);`
	$acc = 1;
	for ($rest = $args_; -not (atom $rest); $rest = cdr $rest)
	{
		if (-not (isnum (car $rest)))
		{
			lthrow $erroid["Type"] ("cannot mul " + (lprint $args_));
		}
		$acc *= (car $rest);
	}
	return $acc;
} "*";
regist_subr $genv { param($args_);`
	if (isnil $args_) { return 0; }
	if (-not (isnum (car $args_)))
	{
		lthrow $erroid["Type"] ("cannot div " + (lprint $args_));
	}
	$acc = car $args_;
	$fflg = $nil;
	for ($rest = cdr $args_; -not (atom $rest); $rest = cdr $rest)
	{
		$n = car $rest;
		if (-not (isnum $n))
		{
			lthrow $erroid["Type"] ("cannot div " + (lprint $args_));
		}
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
regist_subr $genv { param($args_);
	$a = (car $args_);
	$b = (car (cdr $args_));
	if ((-not (isnum $a)) -or (-not (isnum $b)))
	{
		lthrow $erroid["Type"] ("cannot mod " + (lprint $args_));
	}
	return $a % $b} "%";
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
regist_subr $genv { param($args_);
	$n = (car $args_);
	if ("<inum>" -eq (ltype $n).name) { return $n; }
	if ("<fnum>" -eq (ltype $n).name) { return [int]$n; }
	lthrow $erroid["Type"] ("cannot cast " + (lprint $n) + " to InumT.");
} "int";
regist_subr $genv { param($args_);
	$n = (car $args_);
	if ("<fnum>" -eq (ltype $n).name) { return $n; }
	if ("<inum>" -eq (ltype $n).name) { return [double]$n; }
	lthrow $erroid["Type"] ("cannot cast " + (lprint $n) + " to FnumT.");
} "float";
regist_subr $genv { param($args_); return (tolist (car $args_)); } "to-list";
regist_subr $genv { param($args_); return (tovect (car $args_)); } "to-vect";
regist_subr $genv { param($args_); return (toqueu (car $args_)); } "to-queu";
regist_subr $genv { param($args_); return (symbol (car $args_)); } "symbol";
regist_subr $genv { param($args_); return (sprint $args_); } "sprint";
regist_subr $genv { param($args_); return (load (car $args_)); } "load";
regist_subr $genv { param($args_); lthrow (car $args_) (car (cdr $args_)); } "throw";
regist_subr $genv { param($args_); return (empty (car $args_)); } "empty";
regist_subr $genv { param($args_);
	write-host -nonewline (sprint $args_);
	return $nil; } "prin";
regist_subr $genv { param($args_);
	write-host (sprint $args_);
	return $nil; } "print";
regist_subr $genv { param($args_);
	return (ltype (car $args_)); } "type";
regist_subr $genv { param($args_); return (getat (car $args_) (car (cdr $args_))) } "getat";
regist_subr $genv { param($args_);return (setat (car $args_) (car (cdr $args_)) (car (cdr (cdr $args_)))); } "setat";
regist_subr $genv { param($args_); return (getc); } "getc";


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
regist_spfm $genv { param($args_, $env);
	return (expand_quasiquote (car $args_) $env); } "quasiquote";
regist_spfm $genv { param($args_, $env);`
	$expr = car $args_;
	if ($expr -is [symb]) { return iex (car $args_).name; }
	return iex (car $args_);
} "ps";
regist_spfm $genv { param($args_, $env);
	$ret = leval (car $args_) $genv;
	for ($rest = cdr $args_; -not (atom $rest); $rest = cdr $rest)
	{
		write-host "debug: ", (lprint (car $rest));
		[void]($ret = $ret.((car $rest).name));
	}
	return $ret;
} "->";
regist_spfm $genv { param($args_, $env);
	if (isnil $args_) { return $t; }
	$rest = $args_;
	for (; -not (atom (cdr $rest)); $rest = cdr $rest)
	{
		if (isnil (leval (car $rest) $env)) { return $nil; }
	}
	return (leval (car $rest) $env); } "and";
regist_spfm $genv { param($args_, $env);
	if (isnil $args_) { return $nil; }
	$rest = $args_;
	for (; -not (atom (cdr $rest)); $rest = cdr $rest)
	{
		$val = (leval (car $rest) $env);
		if (-not (isnil $val)) { return $val; }
	}
	return (leval (car $rest) $env); } "or";
regist_spfm $genv { param($args_, $env);
	try
	{
		return (leval (car (cdr $args_)) $env);
	}
	catch
	{
		$ex = $_.exception -split ",";
		$eid = [int]($ex[1]);
		return (apply (leval (car $args_) $env) (list $eid ($ex[2..($ex.length - 1)] -join ",")));
	}} "catch";
regist_spfm $genv { param($args_, $env); return $env; } "environment";
# TODO


function readtop_pipe
{
	param([parameter(valuefrompipeline = $True)] $src);
	return (lreadtop $src);
}

function eval_pipe
{
	param($env, [parameter(valuefrompipeline = $True)] $obj);
	return (leval $obj $env);
}

function print_pipe
{
	param([parameter(valuefrompipeline = $True)] $obj);
	return (lprint $obj);
}


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
			[void]($co = find_co_paren $src.substring($idx + 1));
			[void]($tree.add(`
						(wrap_readmacros (lread $src.substring($idx + 1, $co))`
						 $buff[1])));
			[void]($buff[1] = $nil);
			[void]($idx += $co + 1);
		}
		elseif (")" -eq $c)
		{
			lthrow $erroid["Syntax"] "found excess close parenthesis.";
		}
		elseif ("[" -eq $c)
		{
			[void](growth $tree $buff);
			[void]($co = find_co_brackets $src.substring($idx + 1));
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
			lthrow $erroid["Syntax"] "found excess brackets.";
		}
		elseif ("." -eq $c)
		{
			if (-not $buff[0])
			{
				[void]($ltree = array2cons $tree);
				[void](rplacd (last $ltree)`
					(car (lread $src.substring($idx + 1))));
				return $ltree;
			}
			$buff[0] += $c;
		}
		elseif ("`"" -eq $c)
		{
			[void](growth $tree $buff);
			[void]($ret = take_string $src.substring($idx + 1));
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
				return $proc.call((mapeval $args_ $env));
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
				lthrow $erroid["UnCallable"] "$proc is not callable.";
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
	if ($obj -is [vect])
	{
		return (lprint (toarray $obj));
	}
	if ($obj -is [queu])
	{
		return "/" + (lprint $obj.exit) + "/";
	}

	return [string]$obj;
}

function printcons ($a, $d)
{
	$sa = lprint $a;
	if (isnil $d) { return "($sa)"; }

	$sd = lprint $d;

	if ($d -is [cons])
	{
		return "($sa " + $sd.substring(1);
	}

	return "(" + $sa + " . " + $sd + ")";
}

function repl ()
{
	while ($True)
	{
		try
		{
			write-host -nonewline "mal> ";
			$in = (read-host);
			if (-not $in) { return; }
			write-host (lprint (leval (lreadtop $in) $genv));
		}
		catch
		{
			$ex = $_.exception -split ",";
			$eid = [int]($ex[1]);
			write-host ("<Erro " + (lprint ($ex[2..($ex.length - 1)] -join ",")) + ">");
		}
	}
}

