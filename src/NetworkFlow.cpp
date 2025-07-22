#include <numeric>
#include <stdexcept>
#include <ilcplex/ilocplex.h>
#include "NetworkFlow.hpp"

using namespace std;

NetworkFlow::NetworkFlow(int n) : numNodes(n), balances(n, 0.0) {}

void NetworkFlow::setBalance(int node, double b) {
    if (node < 1 || node > numNodes)
        throw std::out_of_range("Node out of range: " + to_string(node));
    balances[node - 1] = b;
}

void NetworkFlow::addEdge(int from, int to, double cost) {
    if (from < 1 || from > numNodes || to < 1 || to > numNodes)
        throw std::out_of_range("Invalid node in edge: " + to_string(from) + "->" + to_string(to));
    edges.emplace_back(from, to, cost);
}

int NetworkFlow::getNumNodes() const { return numNodes; }

double NetworkFlow::getBalance(int node) const {
    if (node < 1 || node > numNodes) return 0.0;
    return balances[node - 1];
}

const vector<Edge>& NetworkFlow::getEdges() const {
    return edges;
}

bool NetworkFlow::isBalanced() const {
    double sum = std::accumulate(balances.begin(), balances.end(), 0.0);
    return abs(sum) < 1e-5;
}

string NetworkFlow::validate() const {
    if (!isBalanced()) {
        return "Supply and demand are not balanced.";
    }
    for (const auto& e : edges) {
        if (e.from < 1 || e.from > numNodes || e.to < 1 || e.to > numNodes) {
            return "Invalid edge: " + to_string(e.from) + "->" + to_string(e.to);
        }
    }
    return "valid";
}

Solution NetworkFlow::solve() const {
    IloEnv env;
    Solution result;
    try {
        IloModel model(env, "MinimumCostFlow");
        IloCplex cplex(model);
        IloExpr totalCost(env);

        // Map: (from, to) -> variable
        map<pair<int, int>, IloNumVar> edgeVars;

        // Create variables
        for (const auto& e : edges) {
            IloNumVar var(0, IloInfinity, ILOFLOAT);
            string name = "x_" + to_string(e.from) + "_" + to_string(e.to);
            var.setName(name.c_str());
            edgeVars[{e.from, e.to}] = var;
            totalCost += e.cost * var;
        }
        model.add(IloMinimize(env, totalCost));
        totalCost.end();

        // Flow conservation constraints
        for (int node = 1; node <= numNodes; ++node) {
            IloExpr netFlow(env);
            double supply = getBalance(node); // b_i

            // Sum of inflow - outflow = -b_i
            for (const auto& e : edges) {
                if (e.to == node) netFlow += edgeVars.at({e.from, e.to});
                if (e.from == node) netFlow -= edgeVars.at({e.from, e.to});
            }

            model.add(netFlow == -supply);
            netFlow.end();
        }

        // Solve
        if (cplex.solve()) {
            result.solved = true;
            result.totalCost = cplex.getObjValue();
            result.status = "Optimal";

            for (const auto& e : edges) {
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

    } catch (const IloException& ex) {
        result.status = "Exception: " + string(ex.getMessage());
    } catch (const exception& ex) {
        result.status = "STD Exception: " + string(ex.what());
    }

    env.end();
    return result;
}P