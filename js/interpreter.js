let nil = false;
let t = true;

function Cons (a, d)
{
	this.car = a;
	this.cdr = d;
	this.type = "cons";
}

function cons (a, d)
{
	return new Cons(a, d);
}

function car (o)
{
	if (isnil(o)) { return nil; }
	return o.car;
}

function cdr (o)
{
	if (isnil(o)) { return nil; }
	return o.cdr;
}

function eq (a, b)
{
	if (a === b) { return t; }
	return nil;
}

function equal (a, b)
{
	if (a == b) { return t; }
	return nil;
}

function atom (o)
{
	if (consp(o)) { return nil; }
	return t;
}

