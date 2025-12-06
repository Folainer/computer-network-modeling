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
#include "Global.hpp"
#include "Packet.hpp"

class Controller;

// Message represents data being transmitted
class Message {
public:
    Message(int messageSize, int messageId, TransactionType type);
    
    int getDisplacement() const { return displacement; }
    void setDisplacement(int newValue) { displacement = newValue; }
    int getMessageSize() const { return messageSize; }
    int getMessageId() const { return messageId; }
    TransactionType getType() const { return type; }
    
    void setRoute(const vector<int>& route);
    const vector<int>& getRoute() const { return routeTable; }
    bool hasRoute() const { return !routeTable.empty(); }
    
private:
    int displacement;
    int messageSize;
    int messageId;
    TransactionType type;
    vector<int> routeTable;
};

// Transaction represents a network transmission request
struct Transaction {
    int t;
    int src;
    int dst;
    int size;
    TransactionType type;
    int id;
    shared_ptr<Message> message;
    
    Transaction(int t, int src, int dst, int size, TransactionType type, int id);
    bool operator<(const Transaction& other) const;
};

// Network simulation engine
class Simulation {
public:
    Simulation(const string& inputfile, Graph& graph, Controller& controller);
    void run(const string& outputfile);

private:
    Graph& g;
    Controller& controller;
    priority_queue<Transaction> pq;
    long simulationTime;
    ofstream file;
    
    // File parsing
    vector<Transaction> parseTransactionFile(const string& filepath);
    
    // Processing
    void processTransaction(Transaction& trans);
    void processBuffer();
    void processEdgeBuffers();
    
    // Output helpers
    void outputMessageStart(const Transaction& trans, const string& path);
    void outputPacketCreation(const shared_ptr<Packet>& packet, int nodeId, const string& path, char channelType);
    void outputPacketTransmission(const shared_ptr<Packet>& packet, int fromNode, int toNode, const string& path);
    void outputPacketArrival(const shared_ptr<Packet>& packet, int node, const string& path);
    void outputPacketFail(const shared_ptr<Packet>& packet, int node, const string& path, string failReason);
    
    // Utility
    string getPath(const vector<int>& routeTable) const;
    int findNextHop(int currentNode, int destination, int seed);
    bool hasActivePackets();
    void updatePacketSentStats(const shared_ptr<Packet>& packet) const;
    void updatePacketRecieveStats(const shared_ptr<Packet>& packet) const;
    void updateDroppedStats(const shared_ptr<Packet>& packet) const;
    void outputStats() const;
};

#endif // SIMULATION_HPP