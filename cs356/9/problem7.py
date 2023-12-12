import networkx as nx
import matplotlib.pyplot as plt

#returns augmented path in graph or else nothing
#graph is a graph, M is a matching of graph (list of edges), and R is the list of vertices not in M
def augmented_path(graph, M, R):
    #start = R[0]

    for start in R:
        # keep track of all visited nodes
        explored = []
        # keep track of nodes to be checked
        queue = [start]

        levels = {}         # this dict keeps track of levels
        levels[start]= 0    # depth of start node is 0

        visited= [start]     # to avoid inserting the same node twice into the queue

        count = 0           #to prevent a single edge between two elements of R from being an augmented path

        # keep looping until there are nodes still to be checked
        while queue:
            count = count + 1

           # pop shallowest node (first node) from queue
            node = queue.pop(0)
            explored.append(node)
            neighbours = list(graph.adj(node))

            # add neighbours of node to queue
            for neighbour in neighbours:

                if levels[node] % 2 == 0:
                    if ((node, neighbour) not in M or (neighbour, node) not in M):
                         if neighbour not in levels:
                            if neighbour not in visited:
                                queue.append(neighbour)
                                visited.append(neighbour)

                                #add neighbour in the next odd level
                                levels[neighbour]= levels[node]+1

                                # if neighbour is unmatched in M
                                if neighbour not in M:
                                    #return path from start to neighbour
                                    return

                          elif levels[neighbour] % 2 == 0:
                                #contract blossom
                                return augmented_path(graph_B, M, R):

                 else:
                     # If j is in the current level, (i,j) is a matched edge in M, and i has not been placed in a previous level
                     if ((node, neighbour) in M or (neighbour, node) in M) and neighbour not in levels:
                         if neighbour not in visited:
                             #add neighbour in the next even level
                             queue.append(neighbour)
                             visited.append(neighbour)

                             levels[neighbour]= levels[node]+1
                    # print(neighbour, ">>", levels[neighbour])

        print(levels)

    #return explored

if __name__ == "__main__":
    vertices = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j"]
