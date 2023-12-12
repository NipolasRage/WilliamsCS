/*
  a) We would add one class, Div, which extends Expr, to represent division expressions. We don't need to change other classes.
  b) We would add a method drawParseTree to all n subclasses of Expr, and the abstract method drawParseTree to the abstrac class Expr. Thus, we can use this method to draw the expression parse tree for each subclass of Expr.
  c) e.accept(printer) -> e.left.accept(printer) -> printer.visitNumber(left.n) ->e.right.accept(printer) -> printer.visitNumber(right.n) -> printer.visitSum(leftVal, rightVal) -> System.out.print(stringRep).
  e) We would create one new subclass of expr and change m subclasses of Visitor to have a visitDivision method.
  f) We would create one new visitor Draw that has n methods to visit all expressions defined.
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

class Subtract extends Expr {
    protected Expr left, right;

    public Subtract(Expr left, Expr right) {
	     this.left = left;
	      this.right = right;
    }

    public <T> T accept(Visitor<T> v) {
	     return v.visitSubtract(left.accept(v), right.accept(v));
    }
}

class Times extends Expr {
    protected Expr left, right;

    public Times(Expr left, Expr right) {
	     this.left = left;
	      this.right = right;
    }

    public <T> T accept(Visitor<T> v) {
	     return v.visitTimes(left.accept(v), right.accept(v));
    }
}

/** Abstract class for all visitors */
abstract class Visitor<T> {
    abstract T visitNumber(int n);
    abstract T visitSum(T left, T right);
    abstract T visitSubtract(T left, T right);
    abstract T visitTimes(T left, T right);
}

/** Example visitor to convert an Expr to a String */
class ToString extends Visitor<String> {
    public String visitNumber(int n) {
	return "" + n;
    }
    public String visitSum(String left, String right) {
	return "(" + left + " + " + right + ")";
    }
    public String visitSubtract(String left, String right) {
	return "(" + left + " - " + right + ")";
    }
    public String visitTimes(String left, String right) {
	return "(" + left + " * " + right + ")";
    }
}

class Eval extends Visitor<Integer> {
  public Integer visitNumber(int n) {
    return n;
  }
  public Integer visitSum(Integer left, Integer right) {
    return left + right;
  }
  public Integer visitSubtract(Integer left, Integer right) {
    return left - right;
  }
  public Integer visitTimes(Integer left, Integer right) {
    return left * right;
  }
}

class Compile extends Visitor<String> {
    public String visitNumber(int n) {
	     return "PUSH("+n + ") ";
    }
    public String visitSum(String left, String right) {
	     return "" + left + right + "ADD ";
    }
    public String visitSubtract(String left, String right) {
	return "PUSH(" + left + ")" + "PUSH(" + right + ")" + "SUB";
    }
    public String visitTimes(String left, String right) {
	return "" + left + right + "MULT ";
    }
}

public class ExprVisitor {
    public static void main(String args[]) {
	Expr e = new Sum(new Number(3), new Number(2));
  Expr d = new Times(new Number(3), e);
  ToString printer2 = new ToString();
	Compile printer = new Compile();
  String stringRep2 = d.accept(printer2);
	String stringRep = d.accept(printer);
  System.out.println(stringRep2);
	System.out.println(stringRep);
    }

}
