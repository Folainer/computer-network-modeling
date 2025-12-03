#include "Simulation.hpp"
#include "Controller.hpp"
#include <cmath>

// Message implementation
Message::Message(int messageSize, int messageId, TransactionType type)
    : displacement(0), messageSize(messageSize), messageId(messageId), type(type) {}

void Message::setRoute(const vector<int>& route) {
    routeTable = route;
}

// Transaction implementation
Transaction::Transaction(int t, int src, int dst, int size, TransactionType type, int id)
    : t(t), src(src), dst(dst), size(size), type(type), id(id),
      message(make_shared<Message>(size, id, type)) {}

bool Transaction::operator<(const Transaction& other) const {
    return t > other.t;  // Min-heap based on time
}

// Simulation implementation
Simulation::Simulation(const string& inputfile, Graph& graph, Controller& controller)
    : g(graph), controller(controller), simulationTime(0) {
    
    vector<Transaction> transactions = parseTransactionFile(inputfile);
    
    for (const auto& trans : transactions) {
        pq.push(trans);
    }
    
    if (pq.empty()) {
        cout << "Warning: No transactions loaded from file" << endl;
    } else {
        cout << "Loaded " << transactions.size() << " transactions" << endl;
    }
}

vector<Transaction> Simulation::parseTransactionFile(const string& filepath) {
    vector<Transaction> transactions;
    ifstream file(filepath);
    
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filepath << endl;
        return transactions;
    }
    
    string line;
    int id = 1;
    int lineNum = 0;
    
    while (getline(file, line)) {
        lineNum++;
        if (line.empty() || line[0] == '#') continue;  // Skip empty lines and comments
        
        try {
            stringstream ss(line);
            string token;
            
            // Parse time
            getline(ss, token, ':');
            int t = stoi(token);
            
            ss >> ws;
            
            // Parse source
            getline(ss, token, '-');
            int src = stoi(token);
            
            // Skip '>'
            ss.ignore(1);
            
            // Parse destination
            getline(ss, token, ' ');
            int dst = stoi(token);
            
            // Parse size
            int size;
            ss >> size;
            
            if (size <= 0) {
                cerr << "Warning: Invalid size on line " << lineNum << ", skipping" << endl;
                continue;
            }
            
            // Determine type
            ss >> ws;
            string rest;
            getline(ss, rest);
            
            TransactionType type = (rest.find('=') != string::npos) 
                                   ? VIRTUAL_CHANNEL 
                                   : DATAGRAM;
            
            transactions.emplace_back(t, src, dst, size, type, id++);
        }
        catch (const exception& e) {
            cerr << "Error parsing line " << lineNum << ": " << e.what() << endl;
        }
    }
    
    file.close();
    return transactions;
}

string Simulation::getPath(const vector<int>& routeTable) const {
    if (routeTable.empty()) return "";

    ostringstream oss;
    oss << routeTable[0];

    for (size_t i = 1; i < routeTable.size(); i++) {
        oss << "->" << routeTable[i];
    }

    return oss.str();
}

void Simulation::outputMessageStart(const Transaction& trans, const string& path) {
    char channelType = (trans.type == VIRTUAL_CHANNEL) ? '+' : '-';
    
    file << "M" << channelType << ' '
         << setw(5) << setfill('0') << simulationTime << "t "
         << setw(3) << setfill('0') << trans.id << "id "
         << trans.size << "w ["
         << trans.src << "->" << trans.dst << "] "
         << path
         << endl;
}

void Simulation::outputPacketCreation(const shared_ptr<Packet>& packet, int nodeId,
                                     const string& path, char channelType) {
    file << "P" << channelType << ' '
         << setw(5) << setfill('0') << simulationTime << "t "
         << setw(3) << setfill('0') << packet->getMessageId() << "id "
         << setw(4) << setfill('0') << packet->getPacketId() << "pid "
         << packet->getPacketSize() << "w {"
         << nodeId << "}["
         << packet->getSrc() << "->" << packet->getDst() << "] "
         << path
         << endl;
}

void Simulation::outputPacketTransmission(const shared_ptr<Packet>& packet,
                                         int fromNode, int toNode, const string& path) {
    file << "PT "
         << setw(5) << setfill('0') << simulationTime << "t "
         << setw(3) << setfill('0') << packet->getMessageId() << "id "
         << setw(4) << setfill('0') << packet->getPacketId() << "pid "
         << packet->getPacketSize() << "w {"
         << fromNode << "->" << toNode << "}["
         << packet->getSrc() << "->" << packet->getDst() << "] "
         << path
         << endl;
}

void Simulation::processTransaction(Transaction& trans) {
    char channelType = (trans.type == VIRTUAL_CHANNEL) ? '+' : '-';
    
    // Get source node
    Node* srcNode = g.getNode(trans.src);
    if (!srcNode) {
        cerr << "Error: Source node " << trans.src << " not found" << endl;
        return;
    }

    // For VIRTUAL_CHANNEL, compute and store route once
    if (trans.type == VIRTUAL_CHANNEL && !trans.message->hasRoute()) {
        vector<int> routeTable = controller.findPath(trans.src, trans.dst);
        
        if (routeTable.empty() || routeTable.size() < 2) {
            cerr << "Error: No valid path from " << trans.src << " to " << trans.dst << endl;
            return;
        }
        
        trans.message->setRoute(routeTable);
    }

    // Output message start on first packet
    if (trans.message->getDisplacement() == 0) {
        string path = "";
        if (trans.type == VIRTUAL_CHANNEL) {
            path = getPath(trans.message->getRoute());
        }
        
        outputMessageStart(trans, path);
        
        cout << "Time " << simulationTime << ": Message " << trans.id
             << " (" << (trans.type == VIRTUAL_CHANNEL ? "VC" : "DG")
             << ") " << trans.src << "->" << trans.dst
             << ", size=" << trans.size;
        if (trans.type == VIRTUAL_CHANNEL) {
            cout << ", path=" << path;
        }
        cout << endl;
    }
    
    // Create packets while buffer has space and message not complete
    int packetsCreated = 0;
    while (srcNode->buffer.size() < static_cast<size_t>(ROUTER_BUFFER_SIZE)) {
        int remaining = trans.size - trans.message->getDisplacement();
        if (remaining <= 0) break;

        int packetSize;
        int dataSize;

        if (remaining + header_size >= MTU) {
            packetSize = MTU;
            dataSize = MTU - header_size;
        } else {
            packetSize = remaining + header_size;
            dataSize = remaining;
        }

        trans.message->setDisplacement(trans.message->getDisplacement() + dataSize);

        // Create packet
        shared_ptr<Packet> packet;
        if (trans.type == VIRTUAL_CHANNEL) {
            const vector<int>& route = trans.message->getRoute();
            packet = std::make_shared<Packet>(Packet::create(trans.src, trans.dst, packetSize,
                                   trans.message->getMessageId(), trans.type, &route));
        } else {
            packet = std::make_shared<Packet>(Packet::create(trans.src, trans.dst, packetSize,
                                   trans.message->getMessageId(), trans.type, nullptr));
        }

        if (!packet) {
            cerr << "Error: Failed to create packet" << endl;
            break;
        }

        srcNode->buffer.push(packet);
        packetsCreated++;

        // Output packet creation
        string path = "";
        if (trans.type == VIRTUAL_CHANNEL) {
            path = getPath(trans.message->getRoute());
        }
        outputPacketCreation(packet, trans.src, path, channelType);

        // If buffer is full, reschedule transaction
        if (srcNode->buffer.size() >= static_cast<size_t>(ROUTER_BUFFER_SIZE)) {
            if (trans.message->getDisplacement() < trans.size) {
                pq.push(trans);
            }
            break;
        }
    }
    
    if (packetsCreated > 0 && trans.message->getDisplacement() >= trans.size) {
        cout << "  Message " << trans.id << " fully packetized (" 
             << packetsCreated << " packets)" << endl;
    }
}

int Simulation::findNextHop(int currentNode, int destination) {
    vector<int> path = controller.findPath(currentNode, destination);
    
    if (path.size() < 2) {
        return -1;
    }
    
    return path[1];
}

void Simulation::processBuffer() {
    // Process node buffers - forward packets to edges
    for (int nid = 0; nid < g.n; nid++) {
        Node* node = g.getNode(nid);
        if (!node || node->buffer.empty()) continue;
        
        shared_ptr<Packet> packet = node->buffer.front();
        node->buffer.pop();
        
        // Decrement TTL
        packet->decrementTTL();
        if (packet->getTTL() <= 0) {
            cout << "Time " << simulationTime << ": Packet " << packet->getPacketId()
                 << " dropped (TTL expired) at node " << nid << endl;
            continue;
        }

        // Check if packet reached destination
        if (nid == packet->getDst()) {
            cout << "Time " << simulationTime << ": Packet " << packet->getPacketId()
                 << " delivered to destination " << nid << endl;
            continue;
        }

        // Determine next hop
        int nextNode = -1;
        if (packet->getType() == VIRTUAL_CHANNEL) {
            nextNode = packet->getNextNode(g, nid);
            if (nextNode != -1) {
                packet->incrementRoutePos();
            }
        } else {
            nextNode = findNextHop(nid, packet->getDst());
        }

        if (nextNode == -1) {
            cerr << "Time " << simulationTime << ": No route for packet "
                 << packet->getPacketId() << " at node " << nid << endl;
            continue;
        }

        // Find edge to next node
        Edge* edge = g.findEdge(nid, nextNode);
        if (!edge) {
            cerr << "Time " << simulationTime << ": No edge from " << nid
                 << " to " << nextNode << endl;
            continue;
        }

        // Check if edge can accept packet (HALF_DUPLEX only allows one packet at a time)
        if (edge->type == HALF_DUPLEX && !edge->buffer.empty()) {
            // Channel busy, put packet back in node buffer
            node->buffer.push(packet);
            continue;
        }

        // Calculate transmission time
        // transmission_time = packet_size_bits / bandwidth_bits_per_ms
        // packet_size_bits = packet_size_bytes * 8
        // bandwidth_bits_per_ms = bandwidth_mbps * 1024 * 1024 / 1000 / 8 = bandwidth_mbps * 128
        double transmissionTime = (double)packet->getPacketSize() * 8.0 / 
                                  (edge->weight.bandwidth_mbps * 1024.0 * 1024.0 / 1000.0);
        int sendTime = (int)ceil(transmissionTime);
        
        packet->setSendingTime(sendTime);
        packet->setTransmissionUntil(simulationTime + (long)edge->weight.latency_ms + sendTime);

        // Add to edge buffer
        edge->buffer.push(packet);

        // Output transmission
        string path = "";
        if (packet->getType() == VIRTUAL_CHANNEL) {
            path = getPath(*packet->getRouteTable());
        }
        outputPacketTransmission(packet, nid, nextNode, path);
    }
}

void Simulation::processEdgeBuffers() {
    // Process edge buffers - move packets to destination nodes when transmission completes
    for (int u = 0; u < g.n; u++) {
        for (auto& edge : g.adj[u]) {
            if (edge.buffer.empty()) continue;
            
            shared_ptr<Packet> packet = edge.buffer.front();
            
            // Check if transmission is complete
            if (packet->getTransmissionUntil() <= simulationTime) {
                edge.buffer.pop();
                
                Node* destNode = g.getNode(edge.to);
                if (!destNode) {
                    cerr << "Time " << simulationTime << ": Destination node "
                         << edge.to << " not found" << endl;
                    continue;
                }
                
                // Check if destination buffer has space
                if (destNode->buffer.size() < static_cast<size_t>(ROUTER_BUFFER_SIZE)) {
                    destNode->buffer.push(packet);
                } else {
                    cout << "Time " << simulationTime << ": Packet "
                         << packet->getPacketId() << " dropped (buffer full) at node "
                         << edge.to << endl;
                }
            }
        }
    }
}

bool Simulation::hasActivePackets() {
    for (int i = 0; i < g.n; i++) {
        Node* node = g.getNode(i);
        if (node && !node->buffer.empty()) {
            return true;
        }
        
        for (const auto& edge : g.adj[i]) {
            if (!edge.buffer.empty()) {
                return true;
            }
        }
    }
    return false;
}

void Simulation::run(const string& outputfile) {
    file.open(outputfile);
    
    if (!file.is_open()) {
        cerr << "Error: Could not open output file " << outputfile << endl;
        return;
    }

    cout << "\nStarting simulation..." << endl;
    cout << "Output file: " << outputfile << endl << endl;

    int maxIterations = 100000;  // Prevent infinite loops
    int iterations = 0;

    while ((iterations < maxIterations) && (!pq.empty() || hasActivePackets())) {
        // Process transactions scheduled for current time
        while (!pq.empty() && pq.top().t <= simulationTime) {
            Transaction trans = pq.top();
            pq.pop();
            processTransaction(trans);
        }

        // Process network buffers
        processEdgeBuffers();
        processBuffer();

        // Advance time
        simulationTime++;
        iterations++;
    }
    
    if (iterations >= maxIterations) {
        cout << "\nWarning: Simulation stopped at maximum iterations" << endl;
    }
    
    file.close();
    cout << "\nSimulation completed at time " << simulationTime << endl;
    cout << "Total iterations: " << iterations << endl;
    cout << "Output written to " << outputfile << endl;
}