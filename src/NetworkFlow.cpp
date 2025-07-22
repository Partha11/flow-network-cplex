/**
 * @file NetworkFlow.cpp
 * @brief Implementation of minimum cost network flow solver using IBM CPLEX
 * @author Abir Chakraborty Partha
 * @date 22 Jul, 2025
 */

#include "NetworkFlow.hpp"
#include <ilcplex/ilocplex.h>
#include <numeric>
#include <stdexcept>
#include <cmath>

using namespace std;

/**
 * @brief Constructor for NetworkFlow class
 * @param n Number of nodes in the network (1-indexed)
 * @throws None
 * 
 * Initializes a network flow problem with n nodes and zero balances for all nodes.
 * Node indices are expected to be 1-indexed in the public interface.
 */
NetworkFlow::NetworkFlow(int n) : numNodes(n), balances(n, 0.0) {}

/**
 * @brief Set the supply/demand balance for a specific node
 * @param node Node index (1-indexed)
 * @param b Balance value (positive for supply, negative for demand)
 * @throws std::out_of_range If node index is invalid
 * 
 * Sets the supply/demand balance for the specified node:
 * - Positive values indicate supply nodes (sources)
 * - Negative values indicate demand nodes (sinks)
 * - Zero indicates transshipment nodes
 */
void NetworkFlow::setBalance(int node, double b) {
    if (node < 1 || node > numNodes)
        throw std::out_of_range("Node out of range: " + to_string(node));
    balances[node - 1] = b;
}

/**
 * @brief Add a directed edge to the network
 * @param from Source node index (1-indexed)
 * @param to Destination node index (1-indexed)
 * @param cost Cost per unit of flow along this edge
 * @throws std::out_of_range If either node index is invalid
 * 
 * Adds a directed edge from 'from' to 'to' with the specified cost per unit flow.
 * Edge capacities are assumed to be unlimited. Multiple edges between the same
 * pair of nodes are allowed.
 */
void NetworkFlow::addEdge(int from, int to, double cost) {
    if (from < 1 || from > numNodes || to < 1 || to > numNodes)
        throw std::out_of_range("Invalid node in edge: " + to_string(from) +
                                "->" + to_string(to));
    edges.emplace_back(from, to, cost);
}

/**
 * @brief Get the number of nodes in the network
 * @return Number of nodes
 */
int NetworkFlow::getNumNodes() const { return numNodes; }

/**
 * @brief Get the supply/demand balance for a specific node
 * @param node Node index (1-indexed)
 * @return Balance value for the node, or 0.0 if node index is invalid
 * 
 * Returns the balance value for the specified node:
 * - Positive values indicate supply (source nodes)
 * - Negative values indicate demand (sink nodes)
 * - Zero indicates transshipment nodes
 */
double NetworkFlow::getBalance(int node) const {
    if (node < 1 || node > numNodes)
        return 0.0;
    return balances[node - 1];
}

/**
 * @brief Get a const reference to all edges in the network
 * @return Const reference to vector of Edge objects
 */
const vector<Edge> &NetworkFlow::getEdges() const { return edges; }

/**
 * @brief Check if the network has balanced supply and demand
 * @return True if total supply equals total demand, false otherwise
 * 
 * A network is balanced if the sum of all node balances equals zero.
 * This is a necessary condition for a feasible flow solution to exist.
 * Uses a tolerance of 1e-5 for floating-point comparison.
 */
bool NetworkFlow::isBalanced() const {
    double sum = std::accumulate(balances.begin(), balances.end(), 0.0);
    return std::abs(sum) < 1e-5;
}

/**
 * @brief Validate the network configuration
 * @return "valid" if network is properly configured, error message otherwise
 * 
 * Performs validation checks on the network:
 * - Verifies that supply and demand are balanced
 * - Checks that all edge endpoints are valid node indices
 * 
 * This should be called before attempting to solve the network flow problem.
 */
string NetworkFlow::validate() const {
    if (!isBalanced()) {
        return "Supply and demand are not balanced.";
    }
    for (const auto &e : edges) {
        if (e.from < 1 || e.from > numNodes || e.to < 1 || e.to > numNodes) {
            return "Invalid edge: " + to_string(e.from) + "->" +
                   to_string(e.to);
        }
    }
    return "valid";
}

/**
 * @brief Solve the minimum cost network flow problem using CPLEX
 * @return Solution object containing results and status information
 * 
 * Formulates and solves the minimum cost flow problem as a linear program:
 * 
 * Minimize: Σ(c_ij * x_ij) for all edges (i,j)
 * Subject to:
 * - Flow conservation: Σ(x_ji) - Σ(x_ij) = -b_i for all nodes i
 * - Non-negativity: x_ij ≥ 0 for all edges (i,j)
 * 
 * Where:
 * - x_ij = flow on edge from node i to node j
 * - c_ij = cost per unit flow on edge (i,j)
 * - b_i = balance at node i (supply if positive, demand if negative)
 * 
 * @note Assumes unlimited edge capacities
 * @note Properly manages CPLEX environment to prevent memory leaks
 * @throws Handles CPLEX and standard exceptions internally
 */
Solution NetworkFlow::solve() const {
    IloEnv env;
    Solution result;
    
    try {
        IloModel model(env, "MinimumCostFlow");
        IloCplex cplex(model);
        
        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());

        // Map: (from, to) -> variable
        map<pair<int, int>, IloNumVar> edgeVars;

        // Create variables and build objective function
        IloExpr totalCost(env);
        for (const auto &e : edges) {
            IloNumVar var(env, 0, IloInfinity, ILOFLOAT);
            string name = "x_" + to_string(e.from) + "_" + to_string(e.to);
            var.setName(name.c_str());
            edgeVars[{e.from, e.to}] = var;
            totalCost += e.cost * var;
            model.add(var);
        }
        
        // Add objective
        model.add(IloMinimize(env, totalCost));

        // Flow conservation constraints
        for (int node = 1; node <= numNodes; ++node) {
            IloExpr netFlow(env);
            double supply = getBalance(node); // b_i

            // Sum of inflow - outflow = -b_i
            for (const auto &e : edges) {
                if (e.to == node) {
                    netFlow += edgeVars.at({e.from, e.to});
                }
                if (e.from == node) {
                    netFlow -= edgeVars.at({e.from, e.to});
                }
            }

            model.add(netFlow == -supply);
            netFlow.end();  // Clean up the expression
        }

        totalCost.end();  // Clean up the objective expression

        // Solve
        if (cplex.solve()) {
            result.solved = true;
            result.totalCost = cplex.getObjValue();
            result.status = "Optimal";

            for (const auto &e : edges) {
                double flow = cplex.getValue(edgeVars.at({e.from, e.to}));
                if (flow > 1e-6) {
                    result.flows[{e.from, e.to}] = flow;
                }
            }
        } else {
            result.status = "No solution found";
            if (cplex.getStatus() == IloAlgorithm::Infeasible)
                result.status = "Infeasible";
            else if (cplex.getStatus() == IloAlgorithm::Unbounded)
                result.status = "Unbounded";
        }

    } catch (const IloException &ex) {
        result.status = "CPLEX Exception: " + string(ex.getMessage());
    } catch (const std::exception &ex) {
        result.status = "STD Exception: " + string(ex.what());
    }

    env.end();
    return result;
}