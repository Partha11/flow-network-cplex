/**
 * @file main.cpp
 * @brief Main program demonstrating minimum cost network flow solver
 * @author Abir Chakraborty Partha
 * @date 22 Jul, 2025
 * 
 * This program solves a lubricant transportation problem using the
 * NetworkFlow class and IBM CPLEX optimization library. The problem involves
 * transporting lubricant from supply nodes to demand nodes at minimum cost.
 */

#include <iostream>
#include "NetworkFlow.hpp"

using namespace std;

/**
 * @brief Main function - Entry point for the lubricant transportation optimization
 * @return 0 if successful, 1 if error occurred
 */
int main() {
    try {
        NetworkFlow net(7);

        // Set balances (1-indexed)
        net.setBalance(1, 40);
        net.setBalance(3, -20);
        net.setBalance(4, 10);
        net.setBalance(7, -30);

        // Add edges
        net.addEdge(1, 2, 5);
        net.addEdge(1, 4, 2);
        net.addEdge(1, 6, 8);
        net.addEdge(2, 3, 10);
        net.addEdge(3, 1, 3);
        net.addEdge(3, 5, 5);
        net.addEdge(3, 7, 7);
        net.addEdge(4, 5, 6);
        net.addEdge(5, 1, 12);
        net.addEdge(5, 6, 12);
        net.addEdge(5, 3, 5);
        net.addEdge(6, 3, 9);
        net.addEdge(6, 7, 20);

        // Validate
        if (std::string validation = net.validate(); validation != "valid") {
            std::cerr << "Validation failed: " << validation << std::endl;
            return 1;
        }

        // Solve
        Solution sol = net.solve();

        if (sol.solved) {
            std::cout << "Solution Status: " << sol.status << std::endl;
            std::cout << "Total Minimum Cost: " << sol.totalCost << std::endl << std::endl;

            std::cout << "Flow Assignment:" << std::endl;
            for (const auto& [edge, flow] : sol.flows) {
                int i = edge.first;
                int j = edge.second;
                double cost = 0;

                // Find cost
                for (const auto& e : net.getEdges()) {
                    if (e.from == i && e.to == j) {
                        cost = e.cost;
                        break;
                    }
                }
                std::cout << "  " << i << " → " << j
                          << " : " << flow
                          << " units (cost/unit: " << cost
                          << ", total: " << flow * cost << ")" << std::endl;
            }
        } else {
            std::cerr << "Failed to solve: " << sol.status << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}