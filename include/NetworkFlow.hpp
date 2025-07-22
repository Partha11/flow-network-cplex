#pragma once

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

struct Edge {
    int from;
    int to;
    double cost;
    Edge(int f, int t, double c) : from(f), to(t), cost(c) {}
};

struct Solution {
    bool solved;
    double totalCost;
    std::map<std::pair<int, int>, double> flows;
    std::string status;

    Solution() : solved(false), totalCost(0.0) {}
};

class NetworkFlow {
private:
    int numNodes;
    std::vector<double> balances;
    std::vector<Edge> edges;

public:
    // Constructor
    NetworkFlow(int n);

    // Getters
    int getNumNodes() const;
    double getBalance(int node) const;
    const std::vector<Edge> &getEdges() const;

    // Setters
    void setBalance(int node, double b);
    void addEdge(int from, int to, double cost);

    // Solve and return solution
    Solution solve() const;

    // Validation
    bool isBalanced() const;
    std::string validate() const;
};