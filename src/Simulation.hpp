#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <iomanip>
#include <memory>
#include <cmath>

// #include "Global.hpp"

class Controller;
#include "Controller.hpp"

enum TransactionType { DATAGRAM, VIRTUAL_CHANNEL };

// Message represents the data being transmitted
class Message {
public:
    Message(int messageSize, int messageId, TransactionType type);
    
    int getDispacement() const { return displacement; }
    void setDispacement(int newValue) { displacement = newValue; }
    int getMessageSize() const { return messageSize; }
    int getMessageId() const { return messageId; }
    TransactionType getType() const { return type; }
    
    void setRoute(const std::vector<int>& route);
    const std::vector<int>* getRoute() const { return &routeTable; }
    // std::vector<int>* getRoute() const { return &routeTable; }
    
private:
    int displacement;
    int messageSize;
    int messageId;
    TransactionType type;
    std::vector<int> routeTable;  // Fixed route for the message
};

// Packet is declared in its own header
class Packet;
#include "Packet.hpp"

// Transaction represents a network transmission request
struct Transaction {
    int t;           // Time
    int src;         // Source node
    int dst;         // Destination node
    int size;        // Message size
    TransactionType type;
    int id;
    std::shared_ptr<Message> message;  // The message being transmitted
    
    Transaction(int t, int src, int dst, int size, TransactionType type, int id);
    bool operator<(const Transaction& other) const;
};

class Simulation
{
public:
    Simulation(const std::string& inputfile, Graph& graph, Controller& controller);
    void run(const std::string& outputfile);

private:
    Graph& g;
    Controller& controller;
    std::priority_queue<Transaction> pq;
    long int simulationTime;
    std::ofstream file;
    
    std::vector<Transaction> parseTransactionFile(const std::string& filepath);
    std::string getPath(const std::vector<int>* routeTable) const;
    void processTransaction(Transaction& trans);
    void processBuffer();
    void outputPacketTransaction(Packet& packet, Edge& edge);

    int findNextVirtual(int from, const vector<int>& routeTable);
    Edge* findEdgePtr(int from, int to) const;
};

#endif