//Nevin Bernet, Jonathan Carrasco, Javier Esparza

import java.lang.Math;

public class VEBTree{

    int u;
    int r;
    VEBTree[] sub;
    boolean[] esum;
    //currently, my implentation has the base base as a boolean and a binary array of length 4. If you can think of a better way to handle, I strongly encourage.
    boolean[] baseCase;
    boolean bc;
    Integer max;
    Integer min;

//constructors ***
  public VEBTree(){

  	u = 64;
  	r = sqrt(u);
  	max = Integer.MAX_VALUE;
  	min = Integer.MIN_VALUE;
  	sub = new VEBTree[r];
  	esum = new boolean[r];

  	//populate sub appropriately
  	if(r<3){
  	  baseCase = new boolean[u];
  	  bc = true;
  	} else {
  	  for(int k = 0; k<r; k++){
  		    sub[k] = new VEBTree(r);
  	  }
  	  bc = false;
  	}
  }

  private int nextPerfectSquare(int N)  {
    int nextN = (int)Math.floor(Math.sqrt(N)) + 1;

    return nextN * nextN;
  }

  public VEBTree(int univ){

  	//u is smallest perfect square where u>=univ
  	u = nextPerfectSquare(univ);
  	r = sqrt(u);
    // maybe
  	max = Integer.MIN_VALUE;
  	min = Integer.MAXVALUE;
  	sub = new VEBTree[r];
  	esum = new boolean[r];

  	//populate sub appropriately
  	if(r<3){
  	    baseCase = new boolean[u];
  	    bc = true;
  	}else{
  	    for(int k = 0; k<r; k++;){
  		sub[k] = new VEBTree(r);
  	    }
  	    bc = false;
  	}
  }
//***

//required methods ***
    //inserts integer i into the tree
    public boolean add(int i, VEBTree t){

	//swap min and i if i is smaller than min
	if(i < t.min){
	    int temp = i;
	    i = t.min;
	    t.min = temp;
	}
	//if block is empty, set summary value for that block to true and then set min value for that block to i
	if(!t.esum[high(i)]){
	    t.esum[high(i)] = true;
	    t.sub[high(i)].min = low(i);
	//if block is not empty, insert i into that block
	}else{
	    //handle base case
	    if (t.bc){
		for (int k = 0; k<t.baseCase.length; k++){
		    //I don't know exactly how to do this here
		}
	    }
	    add(low(i), t.sub[high(i)]);
	}
	if(i > t.max){
	    t.max = i;
	}

	return false;

    }

    //deletes an integer i from the tree
    public boolean delete(int i){

	//todo
	return false;

    }

    //returns the smallest number strictly larger than i
    public int ceiling(int i, VEBTree t){

	if (low(i) < t.sub[high(i)].max){
	    //handle base case (perhaps unnecessary
	    if(t.bc){
		for(int k = 0; k<t.baseCase.length()-1; k++){
		    if(t.baseCase[k] == i){
			return k;
		    }if(t.baseCase[k]<i && t.baseCase[k+1]>i){
			return k+1;
		    }
		}
	    }
	    int j = ceiling(low(i), t.sub[high(i)]);
	    return high(i)*t.r+j;
	}else{
//	    int j = the next index of esum after esum[high(i)] that is true;
	    return j*t.r+t.sub[j].min;
	}

    }

    //returns the largest number strictly smaller than i
    public int floor(int i){

	//todo
	return 0;

    }

    //return true iff tree contains i
    public boolean contains(int i){

	//handle base case
	if (bc){
	    for(int k = 0; k<baseCase.length(); k++){
		if(baseCase[k] == i){
		    return true;
		}
	    }
	    return false;
	}
	//norma, recursive case
	return sub[high(i)].contains(low(i));

    }
//***

//helper methods ***
    //return the int represented by the lower order half of x's bits
    private int high(int x){

	     return x/r;

    }

    //return the int represented by the higher order half of x's bits
    private int low(int x){

	     return x%r;

    }

    //returns the ceiling of the square root of an int
    private int sqrt(int x){

	     return (int) Math.floor(Math.sqrt((double)x));

    }
    //***

    //run and test
    public static void main(String[] args){

	     VEBTree t = new VEBTree();
	      System.out.println(t.sqrt(t.u));

    }

}
