//Nevin Bernet, Jonathan Carrasco, Javier Esparza

import java.lang.Math;
import java.util.Random;
import java.util.Vector;
import java.util.TreeSet;

public class VEBTree2{

  /* Global variables */
  private int min;
  private int max;
  private final int u_size;
  private final int low_mask;
  private final VEBTree2 sum;
  private final VEBTree2[] sub;

  /* helper methods */

  /* mask away high order bits */
  private int low(int x) {
    int r = (int) Math.sqrt(u_size);
    return x % r;
    //return x & low_mask;
  }

  /* unsigned right shift to get high order bits */
  private int high(int x) {
    double r = Math.sqrt(u_size);
    return (int) Math.floor(x/r);
  }

  /* retrieve original value */
  private int index(int x, int y) {
    return (low(y)) | (high(x));
  }

  public void toStrings() {
    System.out.println("min: "+ min+" max: "+max+" u_size: "+u_size+" low_mask: "+low_mask);
  }

  /* Returns square root of highest perfect square >= number */
  public static int upperSquare(int number) {
    double exponent = Math.ceil(Math.log(number) / Math.log(2.0) / 2.0);
    return (int) Math.pow(2.0, exponent);
  }

  /* Returns square root of lowest perfect square <= number */
  private static int lowerSquare(int number) {
    double exponent = Math.floor(Math.log(number) / Math.log(2.0) / 2.0);
    return (int) Math.pow(2.0, exponent);
  }

  /* constructor */
  public VEBTree2(int u_size){
    this.u_size = u_size;
    int u_size_lower = lowerSquare(u_size);
    this.low_mask = u_size_lower - 1;
    // this.shift_by = Integer.numberOfTrailingZeros(u_size_lower);
    min = -1;
    max = -1;

    /* Recursively create sub VEBTrees */
    if (u_size != 2) {
      int upper_u_size = upperSquare(u_size);
      int lower_u_size = lowerSquare(u_size);
      this.sum = new VEBTree2(upper_u_size);
      this.sub = new VEBTree2[upper_u_size];
      /* Populate */
      for (int i = 0; i != upper_u_size; i++) {
          // this.sub[i] = new VEBTree2(lower_u_size);
          this.sub[i] = new VEBTree2(lower_u_size);
      }
    } else { /* base case */
      this.sum = null;
      this.sub = null;
    }
  }

  /* inserts integer x into the tree */
  public void add(int x) {
    /* If no min val exists (when nothing has been inserted)
      min = max = x                                        */
    if (min == -1) {
      min = x;
      max = x;
    }

    /* swap new min val and x */
    if (x < min) {
      int temp = x;
      x = min;
      min = temp;
    }

    /* */
    if (u_size != 2) {
      int sub_min = sub[high(x)].min;

      if (sub_min == -1) {
          sum.add(high(x));
          sub[high(x)].min =low(x);
          sub[high(x)].max =low(x);
      } else {
          sub[high(x)].add(low(x));
      }
    }

    if (max < x) {
        max = x;
    }
  }

  /* finds the successor of x inside the tree */
  public int ceiling(int x) {
    /* Base case */
    if (u_size == 2) {
      /* Return right most value of this substructure */
      if (x == 0 && max == 1) {

          return 1;
      }
      return -1;
    }

    /* Different subtree where min is the ceiling */
    if (min != -1 && x < min) {
        return min;
    }

    /* maximum value of sub structure holding x */
    int max_low = sub[high(x)].max;

    /* If there exists a max and index of x is less than maximum value */
    if (max_low != -1 && low(x) < max_low) {
        int offset = sub[high(x)].ceiling(low(x));
        return index(high(x), offset);
    }

    /* Find the next substructure holding a number */
    int ceil_sub = sum.ceiling(high(x));

    /* If empty return -1 */
    if (ceil_sub == -1) {
        return -1;
    }

    /* Return the first non empty number in the right substructure found */
    int offset = sub[ceil_sub].min;
    return index(ceil_sub, offset);
  }

  /* finds the predecessor of x inside the tree */
  public int floor(int x) {
    /* Base case */
    if (u_size == 2) {
      /* If there doesn't exist a min value */
      if (min == -1) {
          return -1;
      }
      if (x == 1 && min == 0) {
        return 0;
      }

      return -1;
    }

    /* Return left most value of this substructure */
    if (max != -1 && x > max) {
        return max;
    }


    /* minimum value of sub structure holding x */
    int min_low = sub[high(x)].min;

    /* If there exists a max and index of x is greater than minimum value */
    if (min_low != -1 && low(x) > min_low) {
        int offset = sub[high(x)].floor(low(x));
        return index(high(x), offset);
    }

    /* Find the next substructure holding a number */
    int floor_sub = sum.floor(high(x));

    /* If empty return -1 */
    if (floor_sub == -1) {
      if (min != -1 &&  x > min) {
        return min;
      }
      return -1;
    }

    /* Return the first non empty number in the left substructure found */
    int offset = sub[floor_sub].max;
    return index(floor_sub, offset);
  }

  /* deletes integer x from the tree */
  void remove(int x) {
    if (min == max) {
        min = -1;
        max = -1;
        return;
    }

    if (u_size == 2) {
      if (x == 0) {
        min = 1;
      } else {
        max = 0;
      }
      max = min;
      return;
    }

    if (min == x) {
        int first_sub = sum.min;
        x = index(first_sub, sub[first_sub].min);
        min = x;
    }

    sub[high(x)].remove(low(x));

    if (sub[high(x)].min == -1) {
        sum.remove(high(x));

        if (x == max) {
          int sum_max = sum.max;
            if (sum_max == -1) {
                max = min;
            } else {
                int max_val = sub[sum_max].max;
                max = index(sum_max, max_val);
            }
        }
    } else if (x == max) {
        int max_val = sub[high(x)].max;
        max = index(high(x), max_val);
    }
  }

  /* returns true iff tree contains i */
  public boolean contains(int i){
      if (u_size == 2) {
        return i == min || i == max;
      }

      return i == min || sub[high(i)].contains(low(i));
  }

  //run and test
  public static void main(String[] args){
    int size = (int)Math.pow(2.0, (int) Math.pow(2.0, 5));
    VEBTree2 veb = new VEBTree2(size);
    Random rand = new Random();
    Vector<Integer> holder = new Vector<Integer>(size);
    for (int i = 0; i < size; i++) {
      holder.add(rand.nextInt(size));
    }
    Vector<Long> timeVEB = new Vector();
    long startTime = System.currentTimeMillis();
    long endTime;
    for (int i = 0; i < holder.size(); i++) {
      veb.add(holder.get(i));
      if (i % 1000 == 0) {
        endTime = System.currentTimeMillis();
        timeVEB.add(endTime - startTime);
      }
    }

    TreeSet treap = new TreeSet();
    Vector<Long> timeTreap = new Vector();
    long startTimeTreap = System.currentTimeMillis();
    long endTimeTreap;
    for (int i = 0; i < holder.size(); i++) {
      treap.add(holder.get(i));
      if (i % 1000 == 0) {
        endTime = System.currentTimeMillis();
        timeTreap.add(endTime - startTime);
      }
    }

    System.out.println(timeVEB);
    System.out.println(timeTreap);
  }

}
