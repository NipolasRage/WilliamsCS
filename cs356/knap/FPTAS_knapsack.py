import random
import sys
import math
import fileinput
import matplotlib.pyplot as plt

#(c) Javier Esparza, Jonathan Carrasco, Nevin Bernet
class Knapsack:

  def __init__(self, W, n, X):
    self.W = W
    self.n = n
    self.X = X

def Knapsack_Integer(instance):
    W = instance.W
    n = instance.n
    X = instance.X

    profits = [X[i].p for i in range(n)]
    P = sum(profits)

    A = [[0] * (P+1) for i in range(n+1)]

    for p in range(1,P+1):
        A[0][p] = sys.maxsize

    for i in range(1,n+1):
        for p in range(1, P+1):
            if X[i-1].p <= p:
                # print("p " + str(p))
                # print("i " + str(i))
                A[i][p] = min(A[i-1][p], X[i-1].w + A[i-1][p - X[i-1].p])
            else:
                A[i][p] = A[i-1][p]

    opt = max([p for p in range(P+1) if A[n][p] <= W])
    S = Report_Solution(X, A, opt, n)
    return S

def Report_Solution(X, A, opt, n):
    p = opt
    S = set()
    for i in reversed(range(1,n+1)):
        if X[i-1].p <= p:
            if (X[i-1].w + A[i-1][p - X[i-1].p]) < A[i-1][p]:
                S.add(X[i-1])
                p -= X[i-1].p
    return S

def Knapsack_FPTAS(X, W, epsilon):
    LB = max(X, key = lambda x:x.p).p
    for i in X:
        i.p = math.ceil(i.p/((epsilon/len(X)) * LB))
    instance = Knapsack(W, len(X), X)
    S = Knapsack_Integer(instance)
    return S

class x_i:
    def __init__(self, w, p):
        self.w = w
        self.p = p

    def __str__(self):
        return "Weight: " + str(self.w) + " Value: " + str(self.p)

if __name__ == "__main__":
        experiment = True
        if experiment:
            l = [random.randint(0, 2**16 - 1) for i in range(100)]
            random.shuffle(l)
            X = []
            max_weight = 0
            for i in range(50):
                element = x_i(l[i], l[i+50])
                max_weight = l[i] if l[i] > max_weight else 0
                X.append(element)
            knap = Knapsack(max_weight, len(X), X)
            S_opt = Knapsack_Integer(knap)
            opt = sum([i.p for i in S_opt])

            epsilon_l = []
            p_l = []
            print("epsilon:")
            for line in fileinput.input():
                epsilon = float(line)

                S = Knapsack_FPTAS(X, max_weight, epsilon)
                S_list = [str(i) for i in S]
                print("Set S: " + str(S_list))
                weight = sum([i.w for i in S])
                print("Total weight: " + str(weight))
                profit = sum([i.p for i in S])


                print("Approximation profit: " + str(profit))
                print("Optimal profit: " + str(opt))
                epsilon_l.append(1 - epsilon)
                p_l.append(profit/opt)
                print("epsilon:")
            plt.plot(epsilon_l, p_l)
            plt.ylabel("approximation ratio")
            plt.xlabel("epsilon")
            plt.show()
        else:
            print("epsilon")
            epsilon = float(input())
            print("W")
            W = int(input())
            print("n")
            n = int(input())
            X = []
            for line in fileinput.input():
                print("w p")
                input = line.strip().split(" ")
                element = x_i(int(input[0]), int(input[1]))
                X.append(element)
            S = Knapsack_FPTAS(X, W, epsilon)
            S_list = [str(i) for i in S]
            print("Set S: " + str(S_list))
            weight = sum([i.w for i in S])
            print("Total weight: " + str(weight))
            profit = sum([i.p for i in S])
            print("Total profit: " + str(profit))
