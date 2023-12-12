import matplotlib.pyplot as plt
import networkx as nx
import random as rand
import time
import sys

class Random_Graph:

    def __init__(self, n, p):
        if (n > 0 and p <= 1.0 and p >= 0.0):
            self.n = n
            self.p = p

            # we construct G by making a complete graph and flipping a biased
            # coin with probability 1-p to randomly remove edges
            self.G = nx.complete_graph(n)

            self.remove_random_edges()

        else:
            self.G = nx.Graph()


    def remove_random_edges(self):
        inverse_p = 1 - self.p
        for edge in self.G.edges():
            #generates a number [0,1)
            coin_flip = rand.random()

            # if success
            if coin_flip < inverse_p:
                self.G.remove_edge(*edge)


    # A recursive function that uses visited[] and parent to detect
    # cycle in subgraph reachable from vertex v.
    def isCyclicUtil(self,v,visited,parent):

        #Mark the current node as visited
        visited[v]= True

        #Recur for all the vertices adjacent to this vertex
        for i in self.G.neighbors(v):
            # If the node is not visited then recurse on it
            if  not visited[i]:
                if(self.isCyclicUtil(i,visited,v)):
                    return True
            # If an adjacent vertex is visited and not parent of current vertex,
            # then there is a cycle
            elif  parent!=i:
                return True

        return False

    #Returns true if the graph contains a cycle, else false.
    def isCyclic(self):
        # Mark all the vertices as not visited
        visited =[False]*(self.n)
        # Call the recursive helper function to detect cycle in different
        #DFS trees
        for i in range(self.G.number_of_nodes()):
            if not visited[i]: #Don't recur for u if it is already visited
                if(self.isCyclicUtil(i,visited,-1)):
                    return True

        return False

    # returns the number of cycles in the graph
    def number_of_cycles(self):
        components = nx.connected_components(self.G)
        cycles = 0
        for component in components:
            sub = self.G.subgraph(component)
            T = nx.minimum_spanning_tree(sub)
            cycles += len(sub.edges()) - len(T.edges())
        return cycles


if __name__ == "__main__":
    n = int(sys.argv[1])
    p = float(sys.argv[2])
    graph_1 = Random_Graph(n, p)
    attempts = 0
    while not graph_1.isCyclic() and p >= 1/n:
        graph_1 = Random_Graph(n, p)
        print("attempt number " + str(attempts))
        attempts += 1
    print("after " + str(attempts) + " attempts")
    print(str(graph_1.number_of_cycles()) + " cycles")
    pos=nx.spring_layout(graph_1.G)
    nx.draw_networkx(graph_1.G, node_size = 10, pos=pos,with_labels = False)
    for i in nx.cycle_basis(graph_1.G):
        nx.draw_networkx_nodes(graph_1.G, pos,
                           nodelist=i,
                           node_color='r',
                           node_size=20,
                           alpha=0.8)
    plt.show()
