#include "Simulation.hpp"
#include "Controller.hpp"
#include <cmath>

// Реалізація повідомлення
Message::Message(int messageSize, int messageId, TransactionType type)
    : displacement(0), messageSize(messageSize), messageId(messageId), type(type) {}

void Message::setRoute(const vector<int>& route) {
    routeTable = route;
}

// Реалізація транзакції
Transaction::Transaction(int t, int src, int dst, int size, TransactionType type, int id)
    : t(t), src(src), dst(dst), size(size), type(type), id(id),
      message(make_shared<Message>(size, id, type)) {}

bool Transaction::operator<(const Transaction& other) const {
    return t > other.t;  // Min-heap на основі часу
}

// Реалізація симуляції
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
        if (line.empty() || line[0] == '#') continue;  // Пропустити порожні рядки та коментарі
        
        try {
            stringstream ss(line);
            string token;
            
            // Парсинг часу
            getline(ss, token, ':');
            int t = stoi(token);
            
            ss >> ws;
            
            // Парсингу джерела
            getline(ss, token, '-');
            int src = stoi(token);
            
            // Пропустити '>'
            ss.ignore(1);
            
            // Парсити місце призначення
            getline(ss, token, ' ');
            int dst = stoi(token);
            
            // Парсити розмір
            int size;
            ss >> size;
            
            if (size <= 0) {
                cerr << "Warning: Invalid size on line " << lineNum << ", skipping" << endl;
                continue;
            }
            
            // Визначити тип
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

void Simulation::outputPacketCreation(const shared_ptr<Packet>& packet, int nodeId, const string& path, char channelType) {
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

void Simulation::outputPacketTransmission(const shared_ptr<Packet>& packet, int fromNode, int toNode, const string& path) {
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

void Simulation::outputPacketArrival(const shared_ptr<Packet>& packet, int node, const string& path) {
    file << "PR " 
         << setw(5) << setfill('0') << simulationTime << "t "
         << setw(3) << setfill('0') << packet->getMessageId() << "id "
         << setw(4) << setfill('0') << packet->getPacketId() << "pid "
         << packet->getPacketSize() << "w {"
         << node << "}["
         << packet->getSrc() << "->" << packet->getDst() << "] "
         << path
         << endl;
}

void Simulation::outputPacketFail(const shared_ptr<Packet>& packet, int node, const string& path, string failReason)
{
    file << "PF " 
         << setw(5) << setfill('0') << simulationTime << "t "
         << setw(3) << setfill('0') << packet->getMessageId() << "id "
         << setw(4) << setfill('0') << packet->getPacketId() << "pid "
         << packet->getPacketSize() << "w {"
         << node << "}["
         << packet->getSrc() << "->" << packet->getDst() << "] "
         << path
         << endl
         << '\t' << "Reason: " << failReason
         << endl;
}

void Simulation::outputStats() const
{
    for (const auto& [id, m] : messageStats) {
        std::cout << "============================\n";
        std::cout << " Message ID: " << id << "\n";
        std::cout << "----------------------------\n";
        std::cout << " Packets sent:         " << m.packetsSent << "\n";
        std::cout << " Bytes sent:           " << m.bytesSent << "\n";
        std::cout << " Total bytes sent:     " << m.bytesTotalSent << "\n";
        std::cout << " Packets received:     " << m.packetsReceived << "\n";
        std::cout << " Bytes received:       " << m.bytesReceived << "\n";
        std::cout << " Total bytes received: " << m.bytesTotalReceived << "\n";
        std::cout << " Packets dropped:      " << m.packetsDropped << "\n";
        std::cout << " Delivered:            " << (m.delivered ? "YES" : "NO") << "\n";
        std::cout << " Delivered percentage: " << (round(((double)m.bytesReceived / m.bytesSent) * 100 * 100)) / 100 << "%\n";
        std::cout << " First send time:      " << m.firstSendTime << "\n";
        std::cout << " Delivery time:        " << m.deliveryTime << "\n";
        std::cout << "============================\n\n";
    }
}

void Simulation::processTransaction(Transaction& trans) {
    char channelType = (trans.type == VIRTUAL_CHANNEL) ? '+' : '-';
    
    // Отримати вузол джерела
    Node* srcNode = g.getNode(trans.src);
    if (!srcNode) {
        cerr << "Error: Source node " << trans.src << " not found" << endl;
        return;
    }

    // Для VIRTUAL_CHANNEL обчисліть і збережіть маршрут один раз
    if (trans.type == VIRTUAL_CHANNEL && !trans.message->hasRoute()) {
        vector<int> routeTable = controller.findPath(trans.src, trans.dst);
        
        if (routeTable.empty() || routeTable.size() < 2) {
            cerr << "Error: No valid path from " << trans.src << " to " << trans.dst << endl;
            return;
        }
        
        trans.message->setRoute(routeTable);
    }

    // Початок вихідного повідомлення в першому пакеті
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
    
    // Створити пакети, поки в буфері є місце і повідомлення не завершено
    int packetsCreated = 0;
    while (srcNode->buffer.size() < static_cast<size_t>(ROUTER_BUFFER_SIZE)) {
        int remaining = trans.size - trans.message->getDisplacement();
        if (remaining <= 0) break;

        int packetSize;
        int dataSize;

        if (remaining >= MTU - header_size) {
            packetSize = MTU;
            dataSize = MTU - header_size;
        } else {
            packetSize = remaining + header_size;
            dataSize = remaining;
        }

        trans.message->setDisplacement(trans.message->getDisplacement() + dataSize);

        // Створити пакет
        shared_ptr<Packet> packet;
        if (trans.type == VIRTUAL_CHANNEL) {
            const vector<int>* route = &trans.message->getRoute();
            packet = std::make_shared<Packet>(Packet::create(trans.src, trans.dst, packetSize,
                                   trans.message->getMessageId(), trans.type, route));
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

        // Створення вихідного пакета
        string path = "";
        if (trans.type == VIRTUAL_CHANNEL) {
            path = getPath(trans.message->getRoute());
        }
        updatePacketSentStats(packet);
        outputPacketCreation(packet, trans.src, path, channelType);

        // Якщо буфер заповнений, переплануйте транзакцію
        if (srcNode->buffer.size() >= static_cast<size_t>(ROUTER_BUFFER_SIZE)) {
            if (trans.message->getDisplacement() < trans.size) {
                trans.t = simulationTime + 1;
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

int Simulation::findNextHop(int currentNode, int destination, int seed) {
    vector<int> path;

    if (seed % 6 == 0)
    {
        path = g.nodes.at(currentNode).findReservedPath(currentNode, destination, g);
    }
    else
    {
        if (generateRandomDouble() >= 0.10)
        {
            path = controller.findPath(currentNode, destination);
        }
        else
        {
            path = g.nodes.at(currentNode).findReservedPath(currentNode, destination, g);
        }
    }

    
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

        string path = "";
        if (packet->getType() == VIRTUAL_CHANNEL) {
            path = getPath(*packet->getRouteTable());
        }
        
        // Decrement TTL
        packet->decrementTTL();
        if (packet->getTTL() <= 0) {
            // cout << "Time " << simulationTime << ": Packet " << packet->getPacketId()
            //      << " dropped (TTL expired) at node " << nid << endl;
            outputPacketFail(packet, nid, path, string("Packet dropped (ttl expired)"));
            updateDroppedStats(packet);
            continue;
        }

        // Check if packet reached destination
        if (nid == packet->getDst()) {
            // cout << "Time " << simulationTime << ": Packet " << packet->getPacketId()
            //      << " delivered to destination " << nid << endl;
            string path = "";
            if (packet->getType() == VIRTUAL_CHANNEL) {
                path = getPath(*packet->getRouteTable());
            }
            outputPacketArrival(packet, nid, path);
            updatePacketRecieveStats(packet);
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
            nextNode = findNextHop(nid, packet->getDst(), packet->getPacketId());
        }

        if (nextNode == -1) {
            // cerr << "Time " << simulationTime << ": No route for packet "
            //      << packet->getPacketId() << " at node " << nid << endl;
            outputPacketFail(packet, nid, path, string("No route"));
            updateDroppedStats(packet);
            continue;
        }

        // Find edge to next node
        Edge* edge = g.findEdge(nid, nextNode);
        Edge* backEdge = g.findEdge(nextNode, nid);
        
        if (!edge) {
            // cerr << "Time " << simulationTime << ": No edge from " << nid
            //      << " to " << nextNode << " (" << packet->getPacketId() << ')' << endl;
            outputPacketFail(packet, nid, path, string("No edge between nodes"));
            updateDroppedStats(packet);
            continue;
        }

        // Check if edge can accept packet (HALF_DUPLEX only allows one packet at a time)
        if (edge->type == HALF_DUPLEX && (!backEdge->buffer.empty() || !edge->buffer.empty())) {
            // Channel busy, put packet back in node buffer
            if (packet->getType() == VIRTUAL_CHANNEL)
            {
                packet->decrementRoutePos();
            }
            node->buffer.push(packet);
            continue;
        }

        // Calculate transmission time
        // transmission_time = packet_size_bits / bandwidth_bits_per_ms
        // packet_size_bits = packet_size_bytes * 8
        // bandwidth_bits_per_ms = bandwidth_mbps * 1024 * 1024 / 1000 / 8 = bandwidth_mbps * 128
        double transmissionTime = (double)packet->getPacketSize() * 8 / 
                                  (edge->weight.bandwidth_mbps * 1024.0 * 1024.0 / 1000 / 8);
        int sendTime = (int)ceil(transmissionTime);
        
        packet->setSendingTime(sendTime);
        packet->setTransmissionUntil(simulationTime + (long)edge->weight.latency_ms + sendTime);

        // Add to edge buffer
        edge->buffer.push(packet);

        // Output transmission
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

                string path = "";
                if (packet->getType() == VIRTUAL_CHANNEL) {
                    path = getPath(*packet->getRouteTable());
                }
                
                Node* destNode = g.getNode(edge.to);
                if (!destNode) {
                    cerr << "Time " << simulationTime << ": Destination node "
                         << edge.to << " not found" << endl;
                    outputPacketFail(packet, edge.to, path, string("Destination not found"));
                    updateDroppedStats(packet);
                    continue;
                }

                // Random rint(0.31);
                
                // Check if destination buffer has space
                if (destNode->buffer.size() < static_cast<size_t>(ROUTER_BUFFER_SIZE)) {
                    double randomValue = generateRandomDouble();
                    if (randomValue <= edge.p_error)
                    {
                        // cerr << "Time " << simulationTime << ": Packet " << packet->getPacketId() << " on destination node [" << edge.to << "] has error" << endl;
                        // outputPacketFail(packet, edge.to, path, string("Error occured"));
                        updateDroppedStats(packet);
                        continue;
                    }

                    destNode->buffer.push(packet);

                    // string path = "";
                    // if (packet->getType() == VIRTUAL_CHANNEL) {
                    //     path = getPath(*packet->getRouteTable());
                    // }
                    // outputPacketTransmission(packet, u, edge.to, path);
                } else {
                    // cout << "Time " << simulationTime << ": Packet "
                    //      << packet->getPacketId() << " dropped (buffer full) at node "
                    //      << edge.to << endl;
                    outputPacketFail(packet, edge.to, path, string("Packet dropped (buffer full)"));
                    updateDroppedStats(packet);
                }
            }
        }
    }
}

void Simulation::updatePacketSentStats(const shared_ptr<Packet>& packet) const
{
    bool isCreated = false;

    if (messageStats.find(packet->getMessageId()) != messageStats.end())
    {
        isCreated = true;
    }

    auto& stats = messageStats[packet->getMessageId()];
    
    if (!isCreated) {
        stats.firstSendTime = simulationTime;
    }

    stats.packetsSent++;
    stats.bytesSent += packet->getPacketSize() - header_size;
    stats.bytesTotalSent += packet->getPacketSize();
}

void Simulation::updatePacketRecieveStats(const shared_ptr<Packet>& packet) const
{
    auto& stats = messageStats[packet->getMessageId()];

    stats.packetsReceived++;
    stats.deliveryTime = simulationTime;
    stats.bytesReceived += packet->getPacketSize() - header_size;
    stats.bytesTotalReceived += packet->getPacketSize();

    if (stats.packetsSent == stats.packetsReceived)
    {
        stats.delivered = true;
    }
    else
    {
        stats.delivered = false;
    }
}

void Simulation::updateDroppedStats(const shared_ptr<Packet>& packet) const
{
    auto& stats = messageStats[packet->getMessageId()];
    
    stats.delivered = false;
    stats.deliveryTime = simulationTime;
    stats.packetsDropped++;
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
    messageStats.clear();
    Packet::reset();
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
        // Process network buffers
        processEdgeBuffers();
        processBuffer();
        
        // Process transactions scheduled for current time
        while (!pq.empty() && pq.top().t <= simulationTime) {
            Transaction trans = pq.top();
            pq.pop();
            processTransaction(trans);
        }

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
    cout << "Output written to " << outputfile << endl << endl;;

    outputStats();
}