import java.util.*;

public class Treap {

  public Treap left;
  public Treap right;
  public Treap parent;
  public Integer value;
  public Integer priority;
  //private Comparator<? super Integer> comparator;
  Random ran = new Random();

  public Treap() {
    left = null;
    right = null;
    parent = null;
    value = null;
    priority = -1;
  }

  // public Treap(Comparator<? super Integer> c) {
  //   this();
  //   comparator = c;
  // }

  public Treap(Collection<Integer> c) {
    this();
    for (Integer item: c) {
      insert(item, ran.nextInt());
    }

  }

  public void insert(Integer k, Integer p) {
    insertHelper(k, p, this);
  }

  public void delete(Integer k) {
    deleteHelper(k, this);
  }

  public boolean contains(Integer k) {
    return containsHelper(k, this);
  }

  private boolean containsHelper(Integer k, Treap t) {
    if (t == null) {
      return false;
    }
    if (t.value == k) {
      return true;
    }
    if (k < t.value) {
      return containsHelper(k, t.left);
    }
    return containsHelper(k, t.right);
  }

  private void deleteHelper(Integer k, Treap t) {
    if (k < t.value) {
      deleteHelper(k, t.left);
    } else if (k > t.value) {
      deleteHelper(k, t.right);
    } else {
      rootDelete(k, t);
    }
  }

  private void rootDelete(Integer k, Treap t) {
    if (t == null || (t.left == null && t.right == null)) {
      t = null;
    } else if ((t.left != null && t.right != null) && t.left.priority > t.right.priority){
      rotateRight(t);
      rootDelete(k,t.right);
    } else if ((t.left != null && t.right != null) && t.left.priority < t.right.priority){
      rotateLeft(t);
      rootDelete(k, t.left);
    }
  }

  public Treap insertHelper(Integer k, Integer p, Treap t) {
    if (t == null || (t.value == null && t.priority == -1)) {
      t = new Treap();
      //System.out.println(t);
      t.priority = p;
      t.value = k;
      return t;
    }
    else if (k < t.value) {
      t.left = insertHelper(k, p, t.left);
      if (t.left.priority > t.priority) {
        rotateRight(t);
      }
    }
    else if (k > t.value) {
      t.right = insertHelper(k, p, t.right);
      if (t.right.priority > t.priority) {
        rotateLeft(t);
      }
    }
    return t;
  }

  private void rotateLeft(Treap t) {
    if (t.right.left != null) {
    t = t.right;
    t.right = t.right.left;
    t.right.left = t;
  }
  }

  private void rotateRight(Treap t) {
    if (t.left.right != null) {
      t = t.left;
      t.left = t.left.right;
      t.left.right = t;
    }
  }

  public static void main(String args[]) {
    Treap t = new Treap();
    TreeSet ts = new TreeSet();
    Random rand = new Random();
    int randomList[] = new int[50000];
    for (int i = 0; i < 50000; i++) {
      randomList[i] = rand.nextInt();
    }
    System.out.println(randomList);
    long start = System.currentTimeMillis();
    for (int i = 0; i < 50000; i++) {
      t.insert(randomList[i], rand.nextInt());
    }
    long finish = System.currentTimeMillis();
    long timeElapsedTreap = finish - start;
    System.out.println("Elapsed time for Treap: " + timeElapsedTreap);
    long startTree = System.currentTimeMillis();
    for (int i = 0; i < 50000; i++) {
      ts.add(randomList[i]);
    }
    long finishTree = System.currentTimeMillis();
    long timeElapsedTree = finishTree - startTree;
    System.out.println("Elapsed time for TreeSet: " + timeElapsedTree);
  }

}
