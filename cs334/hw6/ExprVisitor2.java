/*
  a) We would add one class, Div, which extends Expr, to represent division expressions. We don't need to change other classes.
  b) We would add a method drawParseTree to all n subclasses of Expr, and the abstract method drawParseTree to the abstrac class Expr. Thus, we can use this method to draw the expression parse tree for each subclass of Expr.
  c) e.accept(printer) -> e.left.accept(printer) -> printer.visitNumber(left.n) ->e.right.accept(printer) -> printer.visitNumber(right.n) -> printer.visitSum(leftVal, rightVal) -> System.out.print(stringRep).
  d) 
 */
/** Abstract class for all expressions */
abstract class Expr {
    abstract <T> T accept(Visitor<T> v);
}

class Number extends Expr {
    protected int n;

    public Number(int n) { this.n = n; }

    public <T> T accept(Visitor<T> v) {
	return v.visitNumber(this.n);
    }
}

class Sum extends Expr {
    protected Expr left, right;

    public Sum(Expr left, Expr right) {
	this.left = left;
	this.right = right;
    }

    public <T> T accept(Visitor<T> v) {
	return v.visitSum(left.accept(v), right.accept(v));
    }
}


/** Abstract class for all visitors */
abstract class Visitor<T> {
    abstract T visitNumber(int n);
    abstract T visitSum(T left, T right);
}

/** Example visitor to convert an Expr to a String */
class ToString extends Visitor<String> {
    public String visitNumber(int n) { 
	return "" + n;
    }
    public String visitSum(String left, String right) {
	return "(" + left + " + " + right + ")";
    }
}


public class ExprVisitor { 
    public static void main(String args[]) { 
	Expr e = new Sum(new Number(3), new Number(2)); 
	ToString printer = new ToString();
	String stringRep = e.accept(printer); 
	System.out.println(stringRep);
    }

}
