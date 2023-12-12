# (c) Javier Esparza, Jonathan Carrasco, Nevin Bernet

import networkx as nx
import math

#reads in a text file called file_name and returns a list of node number, x coordinate, and y coordinate
def read_problem_data(file_name):
    file = open(file_name, "r")
    raw = file.readlines()
    raw = raw[6:]

    formatted = []
    for e in raw:
        temp = [elem for elem in e[:len(e)-1].split(" ") if elem != ""]
        if temp[0] != "EOF":
            formatted.append([int(elem) for elem in temp])
    return formatted

#reads in an optimal solution file and returns a list of ints as the path
def read_solution_data(file_name):
    file = open(file_name, "r")
    raw = file.readlines()
    raw = raw[4:]

    formatted = [int(elem[:len(elem)-1]) for elem in raw]
    return formatted[:len(formatted)-1]

#takes two lists of three elements as arguments (i.e. one element of the list returned by read_problem_data)
#returns the distance between the two points
def calculate_distance(v1, v2):
    return math.sqrt(math.pow(v2[1]-v1[1], 2) + math.pow(v2[2]-v1[2], 2))

def build_graph(processed_data):
    G = nx.Graph()
    for i in range(len(processed_data)):
        for j in range(i+1, len(processed_data)):
            node1 = processed_data[i]
            node2 = processed_data[j]
            G.add_edge(node1[0], node2[0], weight = calculate_distance(node1, node2))
    return G

def tour_weight(tour, data):
    weight = 0
    for i in range(len(tour) - 2):
        weight += calculate_distance(data[i], data[i+1])
    weight += calculate_distance(data[-2], data[-1])
    return weight

def solution_weight(solution, data):
        return tour_weight(solution, data)

def TSP_Two_Approximation(G, data):
    MST = nx.minimum_spanning_tree(G)

    nodes = list(nx.nodes(MST))
    #picks an arbitrary node as the root of the tree
    root = nodes[0]

    # returns a dfs preorder listing of the nodes and appends the root to the end
    tour = list(nx.dfs_preorder_nodes(MST,root)) + [root]
    weight = tour_weight(tour, data)
    return (tour, weight)

if __name__ == "__main__":
    processed_data1 = read_problem_data("tspData.txt")
    processed_solution1 = read_solution_data("tspOpt.txt")
    G1 = build_graph(processed_data1)
    print(TSP_Two_Approximation(G1, processed_data1))
    print(solution_weight(processed_solution1, processed_data1))
