#! /usr/bin/env python3
import sys
import importlib

class NIL:
	def __bool__ (self):
		return False

	def __str__ (self):
		return "NIL"

class Symb:
	def __init__ (self, name):
		self.name = name

	def __eq__ (self, a):
		return isinstance(a, Symb) and self.name == a.name

	def __str__ (self):
		return self.name

class Cons:
	def __init__ (self, a, d):
		self.car = a
		self.cdr = d
	
	def __eq__ (self, a):
		return isinstance(a, Cons) and self.car == a.car and self.cdr == a.cdr

	def __str__ (self):
		return "({0} . {1})".format(lprint(self.car), lprint(self.cdr))

class Queu:
	def __init__ (self):
		self.mem = []

	def push (self, val):
		self.mem.append(val)
		return self

	def pop (self):
		v = self.mem[0]
		self.mem.pop(0)
		return v
	
	def __eq__ (self, a):
		return isinstance(a, Queu) and self.mem == a.mem

	def __str__ (self):
		return "/({0})/".format(" ".join([lprint(e) for e in self.mem]))

class Spfm:
	def __init__ (self, proc, name):
		self.proc = proc
		self.name = name

	def __eq__ (self, a):
		return isinstance(a, Spfm) and self.name  == a.name

	def __call__ (self, args, env):
		return self.proc(env, args)

	def __str__ (self):
		return "<Spfm {0}>".format(self.name)

class Func:
	def __init__ (self, args, body, env):
		self.args = args
		self.body = body
		self.env = env

	def __eq__ (self, a):
		return isinstance(a, Func) and self.args == self.a and self.body == a.body and self.env == a.env
	
	def __call__ (self, args):
		return leval(self.body, cons(bind_tree(self.args, args), self.env))

	def __str__ (self):
		return "<Func {0} {1}>".format(lprint(self.args), lprint(self.body))

class Erro (Exception):
	def __init__ (self, eid, message):
		self.eid = eid
		self.message = message

	def __str__ (self):
		return "<Erro \"{0}\">".format(self.message)

class Void:
	pass

ErroId = Void()

ErroId.__dict__["FullMemory"]       = 0
ErroId.__dict__["UnknownOpcode"]    = 1
ErroId.__dict__["OutOfEnvironment"] = 2
ErroId.__dict__["Type"]             = 3
ErroId.__dict__["Symbol"]           = 4
ErroId.__dict__["Syntax"]           = 5
ErroId.__dict__["UnCallable"]       = 6
ErroId.__dict__["ArgsUnmatch"]      = 7
ErroId.__dict__["UnEvaluatable"]    = 8
ErroId.__dict__["FileNotFound"]     = 9

nil = NIL()
t = Symb("T")

def cons (a, d):
	return Cons(a, d)

def car (c):
	if c is nil:
		return nil
	return c.car

def cdr (c):
	if c is nil:
		return nil
	return c.cdr

def eq (a, b):
	if a is b:
		return t
	else:
		if isinstance(a, Symb) and a == b:
			return t
		return nil

def equal (a, b):
	if a == b:
		return t
	else:
		return nil

def atom (o):
	if Symb("<cons>") == ltype(o):
		return nil
	return t

def add (*nums):
	acc = 0
	for n in nums:
		if not (isinstance(n, int) or isinstance(n, float)):
			raise Erro(ErroId.Type, "cannot add " + lprint(array2cons(nums)))
		acc += n
	return acc

def sub (head, *nums):
	if not (isinstance(head, int) or isinstance(head, float)):
		raise Erro(ErroId.Type, "cannot sub " + lprint(cons(head, array2cons(nums))))
	acc = head
	for n in nums:
		if not (isinstance(n, int) or isinstance(n, float)):
			raise Erro(ErroId.Type, "cannot sub " + lprint(cons(head, array2cons(nums))))
		acc -= n
	return acc

def mul (*nums):
	acc = 1
	for n in nums:
		if not (isinstance(n, int) or isinstance(n, float)):
			raise Erro(ErroId.Type, "cannot mul " + lprint(array2cons(nums)))
		acc *= n
	return acc

def div (head, *nums):
	def fdiv ():
		acc = head
		for n in nums:
			if not (isinstance(n, int) or isinstance(n, float)):
				raise Erro(ErroId.Type, "cannot div "
						+ lprint(cons(head, array2cons(nums))))
			acc /= n
		return acc

	if isinstance(head, float):
		return fdiv()
	if not isinstance(head, int):
		raise Erro(ErroId.Type, "cannot div " + lprint(cons(head, array2cons(nums))))
	acc = head
	for n in nums:
		if not (isinstance(n, int) or isinstance(n, float)):
			raise Erro(ErroId.Type, "cannot div " + lprint(cons(head, array2cons(nums))))
		if isinstance(n, float):
			return fdiv()
		acc //= n
	return acc

def mod (a, b):
	if not ((isinstance(a, int) or isinstance(a, float))\
		   	and (isinstance(b, int) or isinstance(b, float))):
		raise Erro(ErroId.Type, "cannot mod " + lprint(l(a, b)))
	return a % b

def gt (head, *nums):
	a = head
	for n in nums:
		if a > n:
			a = n
		else:
			return nil
	return t

def lt (head, *nums):
	a = head
	for n in nums:
		if a < n:
			a = n
		else:
			return nil
	return t

def ge (head, *nums):
	a = head
	for n in nums:
		if a >= n:
			a = n
		else:
			return nil
	return t

def le (head, *nums):
	a = head
	for n in nums:
		if a <= n:
			a = n
		else:
			return nil
	return t

def ltype (o):
	if isinstance(o, Cons):
		return Symb("<cons>")
	if isinstance(o, Func):
		return Symb("<func>")
	if isinstance(o, Spfm):
		return Symb("<spfm>")
	if callable(o):
		return Symb("<subr>")
	if isinstance(o, Symb):
		return Symb("<symb>")
	if isinstance(o, str):
		return Symb("<strn>")
	if isinstance(o, int):
		return Symb("<inum>")
	if isinstance(o, float):
		return Symb("<fnum>")
	if isinstance(o, NIL):
		return Symb("<nil>")
	if isinstance(o, list):
		return Symb("<vect>")
	if isinstance(o, Queu):
		return Symb("<queu>")
	return nil

genv = cons(nil, nil)

def lif (env, args):
	if leval(car(args), env) is nil:
		return leval(car(cdr(cdr(args))), env)
	else:
		return leval(car(cdr(args)), env)

def ldefine (env, args):
	sym = car(args)
	idx = associdx(car(genv), sym)
	if idx is None:
		rplaca(genv, cons(cons(sym, leval(car(cdr(args)), env)), car(genv)))
	else:
		rplaca(genv, cons(cons(sym, leval(car(cdr(args)), env))
				, append(take(car(genv), idx), drop(car(genv), idx + 1))))	
	return sym

def lsetq (env, args):
	sym = car(args)
	rest = env
	while not atom(rest):
		idx = associdx(car(rest), sym)
		if not idx is None:
			rplaca(rest, cons(cons(sym, leval(car(cdr(args)), env))
						, append(take(car(rest), idx), drop(car(rest), idx + 1))))	
			return sym
		rest = cdr(rest)
	raise Erro(ErroId.Symbol, lprint(sym) + " is not defined.")

def land (env, args):
	ret = t
	for a in cons2array(args):
		ret = leval(a, env)
		if ret is nil:
			return nil
	return ret

def lor (env, args):
	ret = nil
	for a in cons2array(args):
		ret = leval(a, env)
		if not ret is nil:
			return ret
	return nil

def expand_quasiquote (expr, env):
	if atom(expr):
		return expr
	if Symb("unquote") == car(expr):
		return leval(car(cdr(expr)), env)

	eexpr = Queu()
	rest = expr
	while not atom(rest):
		if not atom(car(rest)) and Symb("splicing") == car(car(rest)):
			sexpr = leval(car(cdr(car(rest))), env)
			while not atom(sexpr):
				eexpr.push(car(sexpr))
				sexpr = cdr(sexpr)
		else:
			eexpr.push(expand_quasiquote(car(rest), env))
		rest = cdr(rest)
	return to_list(eexpr)

def attr (env, args):
	ret = car(args)
	rest = cdr(args)
	while not atom(rest):
		ret = ret.__dict__[car(rest).name]
		rest = cdr(rest)
	return ret


def ldo (env, args):
	rest = args
	while not atom(rest) and not atom(cdr(rest)):
		leval(car(rest), env)
		rest = cdr(rest)
	return leval(car(rest), env)
	
def lread (code):
	tree = nil
	buff = ["", nil]
	idx = 0
	while idx < len(code):
		c = code[idx]
		if "(" == c:
			tree = growth(tree, buff, False)
			co = find_co_paren(code[idx + 1:])
			tree = cons(wrap_readmacros(lread(code[idx + 1: idx + co + 1])
						, buff[1]), tree)
			buff = ["", nil]
			idx += co + 1
		elif ")" == c:
			raise Erro(ErroId.Syntax, "found excess close parenthesis.")
		elif "[" == c:
			tree = growth(tree, buff, False)
			co = find_co_bracket(code[idx + 1:])
			buff[0] = lread(code[idx + 1: idx + co + 1])
			if buff[1]:
				tree = cons(l(Symb("to-vect"), wrap_readmacros(buff[0], buff[1]))
						, tree)
			else:
				tree = cons(cons(Symb("vect"), buff[0]), tree)
			buff = ["", nil]
			idx += co + 1
		elif "]" == c:
			raise Erro(ErroId.Syntax, "found excess close brackets.")
		elif " " == c or "\t" == c or "\n" == c:
			tree = growth(tree, buff, False)
		elif ";" == c:
			tree = growth(tree, buff, False)
			while idx < len(code) and "\n" != code[idx]:
				idx += 1
		elif "\"" == c:
			tree = growth(tree, buff, False)
			(strn, inc) = take_string(code[idx + 1:])
			idx += inc
			tree = cons(strn, tree)
			buff = ["", nil]
		elif "'" == c:
			tree = growth(tree, buff, False)
			buff[1] = cons(Symb("quote"), buff[1])
		elif "`" == c:
			tree = growth(tree, buff, False)
			buff[1] = cons(Symb("quasiquote"), buff[1])
		elif "," == c:
			tree = growth(tree, buff, False)
			buff[1] = cons(Symb("unquote"), buff[1])
		elif "@" == c:
			tree = growth(tree, buff, False)
			buff[1] = cons(Symb("splicing"), buff[1])
		elif "." == c:
			if buff[0]:
				buff[0] += "."
			else:
				return append(reverse(cdr(tree))
						, cons(car(tree), car(lread(code[idx + 1:]))))
		else:
			buff[0] += c
		idx += 1

	tree = growth(tree, buff, False)
	return reverse(tree)

def leval (expr, env):
	while True:
		if isinstance(expr, Cons):
			args = cdr(expr)
			proc = leval(car(expr), env)
			if isinstance(proc, Func):
				expr = proc.body
				env = cons(bind_tree(proc.args, mapeval(args, env)),  proc.env)
			elif isinstance(proc, Spfm):
				if "if" == proc.name:
					if leval(car(args), env) is nil:
						expr = car(cdr(cdr(args)))
					else:
						expr = car(cdr(args))
				elif "do" == proc.name:
					rest = args
					while isinstance(cdr(rest), Cons):
						leval(car(rest), env)
						rest = cdr(rest)
					expr = car(rest)
				elif "!" == proc.name:
					expr = lapply(leval(car(args), env), cdr(args))
				else:
					return proc(args, env)
			elif callable(proc):
				return lapply(proc, mapeval(args, env))
			else:
				raise Erro(ErroId.UnCallable, lprint(proc) + " is not callable.")
		elif isinstance(expr, Symb):
			return seekenv(env, expr)
		else:
			return expr

def lapply (proc, args):
	if isinstance(proc, Func):
		return proc(args)
	if callable(proc):
		return proc(*cons2array(args))
	raise Erro(ErroId.UnCallable, lprint(proc) + " is not callable.")

def lthrow (eid, mess):
	raise Erro(eid, mess)

def lempty (coll):
	if isinstance(coll, NIL):
		return t
	if isinstance(coll, list) or isinstance(coll, str):
		if len(coll) < 1:
			return t
		return nil
	if isinstance(coll, Queu):
		if len(coll.mem) < 1:
			return t
		return nil
	if isinstance(coll, Symb):
		if len(coll.name) < 1:
			return t
		return nil
	return nil

def lcatch (env, args):
	exce = leval(car(args), env)
	try:
		return leval(car(cdr(args)), env)
	except Erro as erro:
		return lapply(exce, l(erro.eid, leval(erro.message, env)))

def llprin (*args):
	for a in args:
		if isinstance(a, str):
			sys.stdout.write(a)
		else:
			sys.stdout.write(lprint(a))
	return nil

def llprint (*args):
	llprin(*args)
	print("")
	return nil

def lgetat (vect, idx):
	if isinstance(vect, list) or isinstance(vect, str):
		return vect[idx]
	if isinstance(vect, Symb):
		return vect.name[idx]
	raise Erro(ErroId.Type, "cannot apply getat to {0}".format(lprint(vect)))

def lsetat (vect, idx, val):
	if isinstance(vect, list) or isinstance(vect, str):
		vect[idx] = val
	elif isinstance(vect, Symb):
		vect.name[idx] = val
	else:
		raise Erro(ErroId.Type, "cannot apply setat to {0}".format(lprint(vect)))
	return vect

def lprint (expr):
	dup = seek_dup(expr, nil, nil)
	s = ""
	try:
		rest = dup
		idx = 0
		while not atom(rest):
			s += "$" + str(idx) + " = " + lprint_rec(car(rest), dup, False) + "\n"
			rest = cdr(rest)
			idx += 1
		s += lprint_rec(expr, dup, True)
	except RecursionError as e:
		print("RecursionError")
	return s

def seek_dup (expr, printed, dup):
	if find(expr, printed):
		return cons(expr, dup)
	if atom(expr):
		return dup
	pd = cons(expr, printed)
	return append(seek_dup(car(expr), pd, dup)
			, seek_dup(cdr(expr), pd, dup))

def lprint_rec (expr, dup, rec):
	idx = findidx_eq(expr, dup)
	if rec and not idx is nil:
		return "$" + str(idx)
	if expr is nil:
		return "NIL"
	elif atom(expr):
		if isinstance(expr, Symb):
			return expr.name
		elif isinstance(expr, str):
			return "\"" + expr + "\""
		elif isinstance(expr, list):
			return "[{0}]".format(" ".join([lprint(e) for e in expr]))
		elif isinstance(expr, Queu):
			return "/({0})/".format(" ".join([lprint(e) for e in expr.mem]))
		elif isinstance(expr, Func):
			return "<Func {0} {1}>".format(lprint(expr.args), lprint(expr.body))
		elif isinstance(expr, Spfm):
			return "<Spfm {0}>".format(expr.name)
		elif callable(expr):
			return "<Subr {0}>".format(expr.__name__)
		else:
			return str(expr)
	else:
		return printcons_rec(expr, dup, True)
	
def printcons_rec (coll, dup, rec):
	a = car(coll)
	d = cdr(coll)
	if d is nil:
		return "(" + lprint_rec(a, dup, rec) + ")"
	elif atom(d):
		return "(" + lprint_rec(a, dup, rec) + " . " + lprint_rec(d, dup, rec) + ")"
	elif findidx_eq(d, dup) is nil:
	 	return "(" + lprint_rec(a, dup, rec) + " " + lprint_rec(d, dup, rec)[1:]
	else:
		return "(" + lprint_rec(a, dup, rec) + " " + lprint_rec(d, dup, rec) + ")"

def lprint_raw (expr):
	if expr is nil:
		return "NIL"
	elif atom(expr):
		if hasattr(expr, "__name__"):
			return expr.__name__
		elif isinstance(expr, Symb):
			return expr.name
		elif isinstance(expr, str):
			return "\"" + expr + "\""
		else:
			return str(expr)
	else:
		return printcons_raw(expr)

def printcons_raw (coll):
	a = car(coll)
	d = cdr(coll)
	if d is nil:
		return "(" + lprint_raw(a) + ")"
	elif atom(d):
		return "(" + lprint_raw(a) + " . " + lprint_raw(d) + ")"
	else:
	 	return "(" + lprint_raw(a) + " " + lprint_raw(d)[1:]

def inumable (s):
	try:
		int(s)
	except ValueError:
		return nil
	return t

def fnumable (s):
	try:
		float(s)
	except ValueError:
		return nil
	return t

def growth (tree, buff, rawmode):
	buf = buff[0]
	rmacs = buff[1]
	if buf:
		buff[0] = ""
		buff[1] = nil
		if rawmode:
			return cons(wrap_readmacros(buf, rmacs), tree)
		else:
			if inumable(buf):
				return cons(wrap_readmacros(int(buf), rmacs), tree)
			if fnumable(buf):
				return cons(wrap_readmacros(float(buf), rmacs), tree)
			return cons(wrap_readmacros(Symb(buf), rmacs), tree)
	return tree

def wrap_readmacros (tree, rmacs):
	wraped = tree
	rest = rmacs
	while not atom(rest):
		wraped = l(car(rest), wraped)
		rest = cdr(rest)
	return wraped

def find_co_paren (code):
	sflg = False
	layer = 1
	idx = 0
	while idx < len(code):
		c = code[idx]
		if not sflg and "(" == c:
			layer += 1
		elif not sflg and ")" == c:
			layer -= 1
		elif "\\" == c:
			idx += 1
		elif "\"" == c:
			sflg = not sflg

		if layer < 1:
			return idx
		idx += 1
	raise Erro(ErroId.Syntax, "not found close parenthesis.")

def find_co_bracket (code):
	sflg = False
	layer = 1
	idx = 0
	while idx < len(code):
		c = code[idx]
		if not sflg and "[" == c:
			layer += 1
		elif not sflg and "]" == c:
			layer -= 1
		elif "\\" == c:
			idx += 1
		elif "\"" == c:
			sflg = not sflg

		if layer < 1:
			return idx
		idx += 1
	raise Erro(ErroId.Syntax, "not found close brackets.")

def take_string (code):
	idx = 0
	strn = ""
	while idx < len(code):
		c = code[idx]
		if "\"" == c:
			return (strn, idx + 1)
		if "\\" == c:
			idx += 1
			c = code[idx]
			if "a" == c:
				c = "\a"
			elif "b" == c:
				c = "\b"
			elif "f" == c:
				c = "\f"
			elif "n" == c:
				c = "\n"
			elif "r" == c:
				c = "\r"
			elif "t" == c:
				c = "\t"
			elif "v" == c:
				c = "\v"
			elif "0" == c:
				c = "\0"
		strn += c
		idx += 1
	raise Erro(ErroId.Syntax, "not found close double quote.")

def cons2array (c):
	arr = []
	rest = c
	while not atom(rest):
		arr.append(car(rest))
		rest = cdr(rest)
	return arr

def array2cons (l):
	c = nil
	for e in l:
		c = cons(e, c)
	return reverse(c)

def bind_tree (treea, treeb):
	if treea:
		if atom(treea):
			return l(cons(treea, treeb))
		if atom(treeb) and treeb:
			raise Erro(ErroId.Syntax, "cannot bind: {0} and {1}".format(
						lprint(treea), lprint(treeb)))
		try:
			return append(bind_tree(car(treea), car(treeb))
					, bind_tree(cdr(treea), cdr(treeb)))
		except Erro as erro:
			raise Erro(ErroId.Syntax, "cannot bind: {0} and {1}".format(
						lprint(treea), lprint(treeb)))
	return nil

def bind (syms, vals):
	rests = syms
	restv = vals
	ret = nil
	while not atom(rests):
		s = car(rests)
		v = car(restv)
		ret = cons(cons(s, v), ret)
		rests = cdr(rests)
		restv = cdr(restv)
	return reverse(ret)

def mapeval (args, env):
	eargs = nil
	rest = args
	while not atom(rest):
		e = car(rest)
		eargs = cons(leval(e, env), eargs)
		rest = cdr(rest)
	return reverse(eargs)

def append (colla, collb):
	app = collb
	rest = reverse(colla)
	while not atom(rest):
		app = cons(car(rest), app)
		rest = cdr(rest)
	return app

def reverse (coll):
	rev = nil
	rest = coll
	while not atom(rest):
		e = car(rest)
		rev = cons(e, rev)
		rest = cdr(rest)
	return rev

def find (val, coll):
	rest = coll
	while not atom(rest):
		if val == car(rest):
			return val
		rest = cdr(rest)
	return nil

def findidx_eq (val, coll):
	idx = 0
	rest = coll
	while not atom(rest):
		if val is car(rest):
			return idx
		rest = cdr(rest)
		idx += 1
	return nil

def rplaca (c, o):
	c.car = o
	return c

def rplacd (c, o):
	c.cdr = o
	return c

def last (a):
	rest = a
	while not atom(cdr(rest)):
		rest = cdr(rest)
	return rest

def nconc (*args):
	if not args:
		return nil
	arr = args[0]
	for a in args[1:]:
		if not a is nil:
			rplacd(last(arr), a)
	return arr

def lload (path):
	if not isinstance(path, str):
		raise Erro(ErroId.Type, "cannot apply load to {0}".format(path))
	try:
		with open(path, "r") as fin:
			expr = leval(cons(Symb("do"), lread(fin.read())), genv)
		return expr
	except FileNotFoundError:
		raise Erro(ErroId.FileNotFound, "not found file: {0}".format(lprint(path)))

def limport (path):
	return importlib.import_module(path)

def vect (*args):
	return list(args)

def queue (*args):
	queu = Queu()
	queu.mem = list(args)
	return queu

def to_list (obj):
	if isinstance(obj, list):
		return l(*obj)
	elif isinstance(obj, Symb):
		return l(*[ord(c) for c in obj.name])
	elif isinstance(obj, str):
		return l(*[ord(c) for c in obj])
	elif isinstance(obj, Queu):
		return l(*obj.mem)
	elif isinstance(obj, Cons):
		return obj
	elif obj is nil:
		return obj
	raise Erro(ErroId.Type, "cannot cast {0} to ConsT.".format(obj))

def to_vect (obj):
	if isinstance(obj, Cons):
		rest = obj
		vect = []
		while not atom(rest):
			vect.append(car(rest))
			rest = cdr(rest)
		return vect
	elif isinstance(obj, Symb):
		return [ord(c) for c in obj.name]
	elif isinstance(obj, str):
		return [ord(c) for c in obj]
	elif isinstance(obj, Queu):
		return obj.mem
	elif isinstance(obj, list):
		return obj
	elif obj is nil:
		return []
	raise Erro(ErroId.Type, "cannot cast {0} to VectT.".format(obj))

def to_queue (obj):
	if isinstance(obj, Cons):
		rest = obj
		queu = Queu()
		while not atom(rest):
			queu.push(car(rest))
			rest = cdr(rest)
		return queu
	elif isinstance(obj, Symb):
		queu = Queu()
		queu.mem = [ord(c) for c in obj.name]
		return queu
	elif isinstance(obj, str):
		queu = Queu()
		queu.mem = [ord(c) for c in obj]
		return queu
	elif isinstance(obj, list):
		queu = Queu()
		queu.mem = [c for c in obj]
		return queu
	elif isinstance(obj, Queu):
		return obj
	elif obj is nil:
		return Queu()
	raise Erro(ErroId.Type, "cannot cast {0} to QueuT.".format(obj))

def symbol (obj):
	if isinstance(obj, Cons):
		rest = obj
		strn = ""
		while not atom(rest):
			strn += chr(car(rest))
			rest = cdr(rest)
		return Symb(strn)
	elif isinstance(obj, Queu):
		return Symb("".join([chr(e) for e in obj.mem]))
	elif isinstance(obj, list):
		return Symb("".join([chr(e) for e in obj]))
	elif isinstance(obj, str):
		return Symb(obj)
	elif isinstance(obj, Symb):
		return obj
	raise Erro(ErroId.Type, "cannot cast {0} to SymbT.".format(obj))

def sprint (*args):
	strn = ""
	for e in args:
		if isinstance(e, str):
			strn += e
		else:
		 	strn += lprint(e)
	return strn



def take (coll, n):
	head = nil
	rest = coll
	for i in range(0, n):
		head = cons(car(rest), head)
		rest = cdr(rest)
	return reverse(head)

def drop (coll, n):
	rest = coll
	for i in range(0, n):
		rest = cdr(rest)
	return rest

def associdx (alist, key):
	idx = 0
	rest = alist
	while not atom(rest):
		e = car(rest)
		if car(e) == key:
			return idx
		idx += 1
		rest = cdr(rest)
	return None

def assocdr (alist, key):
	rest = alist
	while not atom(rest):
		e = car(rest)
		if car(e) == key:
			return cdr(e)
		rest = cdr(rest)
	return None

def seekenv (env, sym):
	rest = env
	while not atom(rest):
		e = car(rest)
		val = assocdr(e, sym)
		if not val is None:
			return val
		rest = cdr(rest)
	raise Erro(ErroId.Symbol, lprint(sym) + " is not defined.")

def l (*args):
	return array2cons(args)



def initenv ():
	ienv = nil
	ienv = cons(cons(Symb("nil"), nil), ienv)
	ienv = cons(cons(Symb("t"), t), ienv)
	ienv = cons(cons(Symb("T"), t), ienv)
	ienv = cons(cons(Symb("cons"), cons), ienv)
	ienv = cons(cons(Symb("car"), car), ienv)
	ienv = cons(cons(Symb("cdr"), cdr), ienv)
	ienv = cons(cons(Symb("eq"), eq), ienv)
	ienv = cons(cons(Symb("equal"), equal), ienv)
	ienv = cons(cons(Symb("atom"), atom), ienv)
	ienv = cons(cons(Symb("list"), l), ienv)
	ienv = cons(cons(Symb("+"), add), ienv)
	ienv = cons(cons(Symb("-"), sub), ienv)
	ienv = cons(cons(Symb("*"), mul), ienv)
	ienv = cons(cons(Symb("/"), div), ienv)
	ienv = cons(cons(Symb("%"), mod), ienv)
	ienv = cons(cons(Symb(">"), gt), ienv)
	ienv = cons(cons(Symb(">="), ge), ienv)
	ienv = cons(cons(Symb("<"), lt), ienv)
	ienv = cons(cons(Symb("<="), le), ienv)
	ienv = cons(cons(Symb("reverse"), reverse), ienv)
	ienv = cons(cons(Symb("append"), append), ienv)
	ienv = cons(cons(Symb("take"), take), ienv)
	ienv = cons(cons(Symb("drop"), drop), ienv)
	ienv = cons(cons(Symb("rplaca"), rplaca), ienv)
	ienv = cons(cons(Symb("rplacd"), rplacd), ienv)
	ienv = cons(cons(Symb("last"), last), ienv)
	ienv = cons(cons(Symb("nconc"), nconc), ienv)
	ienv = cons(cons(Symb("load"), lload), ienv)
	ienv = cons(cons(Symb("vect"), vect), ienv)
	ienv = cons(cons(Symb("queue"), queue), ienv)
	ienv = cons(cons(Symb("pushqueue"), (lambda queu, val: queu.push(val))), ienv)
	ienv = cons(cons(Symb("popqueue"), (lambda queu: queu.pop())), ienv)
	ienv = cons(cons(Symb("to-list"), to_list), ienv)
	ienv = cons(cons(Symb("to-vect"), to_vect), ienv)
	ienv = cons(cons(Symb("to-queue"), to_queue), ienv)
	ienv = cons(cons(Symb("symbol"), symbol), ienv)
	ienv = cons(cons(Symb("sprint"), sprint), ienv)
	ienv = cons(cons(Symb("apply"), lapply), ienv)
	ienv = cons(cons(Symb("throw"), lthrow), ienv)
	ienv = cons(cons(Symb("empty"), lempty), ienv)
	ienv = cons(cons(Symb("print"), llprint), ienv)
	ienv = cons(cons(Symb("prin"), llprin), ienv)
	ienv = cons(cons(Symb("type"), ltype), ienv)
	ienv = cons(cons(Symb("getat"), lgetat), ienv)
	ienv = cons(cons(Symb("setat"), lsetat), ienv)
	ienv = cons(cons(Symb("if"), Spfm(lif, "if")), ienv)
	ienv = cons(cons(Symb("lambda")
				, Spfm((lambda env, args: Func(car(args), car(cdr(args)), env))
					, "lambda")), ienv)
	ienv = cons(cons(Symb("define"), Spfm(ldefine, "define")), ienv)
	ienv = cons(cons(Symb("setq"), Spfm(lsetq, "setq")), ienv)
	ienv = cons(cons(Symb("quote")
				, Spfm((lambda env, args: car(args)), "quote")), ienv)
	ienv = cons(cons(Symb("and"), Spfm(land, "and")), ienv)
	ienv = cons(cons(Symb("or"), Spfm(lor, "or")), ienv)
	ienv = cons(cons(Symb("quasiquote")
				, Spfm((lambda env, args: expand_quasiquote(car(args), env))
					, "quasiquote")), ienv)
	ienv = cons(cons(Symb("environment")
			, Spfm((lambda env, args: env), "environment")) , ienv)
	ienv = cons(cons(Symb("!")
				, Spfm((lambda env, args:
						leval(lapply(leval(car(args), env), cdr(args)), env))
					, "!")), ienv)
	ienv = cons(cons(Symb("py"), Spfm((lambda env, args: eval(car(args).name)), "py")), ienv)
	ienv = cons(cons(Symb("import"), Spfm((lambda env, args:
						importlib.import_module(car(args)))
                                                , "import")), ienv)
	ienv = cons(cons(Symb("->"), Spfm(attr, "->")), ienv)
	ienv = cons(cons(Symb("do"), Spfm(ldo, "do")), ienv)
	ienv = cons(cons(Symb("catch"), Spfm(lcatch, "catch")), ienv)
	return ienv

rplaca(genv, initenv())

def repl (genv, read_dbg):
	while True:
		if read_dbg:
			sys.stdout.write("rdbg> ")
		else:
			sys.stdout.write("mal> ")
		try:
			if read_dbg:
				print(lprint(cons(Symb("do"), lread(input()))))
			else:
				print(lprint(leval(cons(Symb("do"), lread(input())), genv)))
		except EOFError:
			print("")
			break
		except Exception as e:
			print(e)

if __name__ == "__main__":
	read_dbg = False
	if len(sys.argv) > 1 and "-r" == sys.argv[1]:
		read_dbg = True
	repl(genv, read_dbg)
