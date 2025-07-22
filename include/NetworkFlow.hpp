/**
 * @file NetworkFlow.hpp
 * @brief Header file for minimum cost network flow solver using IBM CPLEX
 * @author Abir Chakraborty Partha
 * @date 22 Jul, 2025
 * 
 * This file defines classes and structures for solving minimum cost network flow
 * problems using IBM CPLEX.
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

/**
 * @struct Edge
 * @brief Represents a directed edge in the network flow graph
 * 
 * Contains information about a single directed edge including source node,
 * destination node, and cost per unit of flow.
 */
struct Edge {
    int from;
    int to;
    double cost;

    /**
     * @brief Constructor for Edge
     * @param f Source node index
     * @param t Destination node index  
     * @param c Cost per unit flow
     */
    Edge(int f, int t, double c) : from(f), to(t), cost(c) {}
};

/**
 * @struct Solution
 * @brief Contains the solution results from the network flow optimization
 * 
 * Stores all relevant information about the optimization result including
 * feasibility status, optimal cost, and flow assignments.
 */
struct Solution {
    bool solved;
    double totalCost;
    std::map<std::pair<int, int>, double> flows;
    std::string status;

    /**
     * @brief Default constructor
     * Initializes solution with default values indicating no solution found
     */
    Solution() : solved(false), totalCost(0.0) {}
};

/**
 * @class NetworkFlow
 * @brief Main class for modeling and solving minimum cost network flow problems
 * 
 * The network uses 1-indexed node numbering for user convenience.
 * 
 * @example
 * ```cpp
 * NetworkFlow net(4);
 * net.setBalance(1, 10);   Supply node with 10 units
 * net.setBalance(4, -10);  Demand node requiring 10 units
 * net.addEdge(1, 2, 5);    Edge from 1 to 2 with cost 5
 * net.addEdge(2, 4, 3);    Edge from 2 to 4 with cost 3
 *
 * if (net.validate() == "valid") {
 *     Solution sol = net.solve();
 *     if (sol.solved) {
 *         std::cout << "Minimum cost: " << sol.totalCost << std::endl;
 *     }
 * }
 * ```
 */
class NetworkFlow {
private:
    int numNodes;
    std::vector<double> balances;
    std::vector<Edge> edges;

public:
    /**
     * @brief Construct a new Network Flow object
     * @param n Number of nodes in the network (1-indexed)
     */
    explicit NetworkFlow(int n);

    /**
     * @brief Get the number of nodes in the network
     * @return Number of nodes
     */
    int getNumNodes() const;

    /**
     * @brief Get the balance value for a specific node
     * @param node Node index (1-indexed)
     * @return Balance value (positive=supply, negative=demand, zero=transshipment)
     */
    double getBalance(int node) const;

    /**
     * @brief Get read-only access to all edges
     * @return Const reference to vector of Edge objects
     */
    const std::vector<Edge> &getEdges() const;

    /**
     * @brief Set the supply/demand balance for a node
     * @param node Node index (1-indexed)
     * @param b Balance value (positive for supply, negative for demand)
     * @throws std::out_of_range If node index is invalid
     */
    void setBalance(int node, double b);

    /**
     * @brief Add a directed edge to the network
     * @param from Source node index (1-indexed)
     * @param to Destination node index (1-indexed)
     * @param cost Cost per unit of flow on this edge
     * @throws std::out_of_range If node indices are invalid
     */
    void addEdge(int from, int to, double cost);

    /**
     * @brief Solve the minimum cost network flow problem
     * @return Solution object with results and status
     */
    Solution solve() const;

    /**
     * @brief Check if supply and demand are balanced
     * @return True if total supply equals total demand
     */
    bool isBalanced() const;

    /**
     * @brief Validate the complete network configuration
     * @return "valid" if network is properly configured, error message otherwise
     */
    std::string validate() const;
};