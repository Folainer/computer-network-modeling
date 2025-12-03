#include "Simulation.hpp"

// Message implementation
Message::Message(int messageSize, int messageId, TransactionType type)
    : displacement(0), messageSize(messageSize), messageId(messageId), type(type) {}

void Message::setRoute(const std::vector<int>& route)
{
    routeTable = route;
}


// Transaction implementation
Transaction::Transaction(int t, int src, int dst, int size, TransactionType type, int id) 
    : t(t), src(src), dst(dst), size(size), type(type), id(id),
      message(std::make_shared<Message>(size, id, type)) {}

bool Transaction::operator<(const Transaction& other) const
{
    return t > other.t;  // For min-heap based on time
}

// Simulation implementation
Simulation::Simulation(const std::string& inputfile, Graph& graph, Controller& controller) 
    : g(graph), controller(controller), simulationTime(0)
{
    std::vector<Transaction> transactions = parseTransactionFile(inputfile);
    
    for (const auto& trans : transactions)
    {
        pq.push(trans);
    }
}

std::vector<Transaction> Simulation::parseTransactionFile(const std::string& filepath) 
{
    std::vector<Transaction> transactions;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return transactions;
    }
    
    std::string line;
    int id = 1;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string token;
        
        // Parse time
        std::getline(ss, token, ':');
        int t = std::stoi(token);
        
        ss >> std::ws;
        
        // Parse source
        std::getline(ss, token, '-');
        int src = std::stoi(token);
        
        // Skip '>'
        ss.ignore(1);
        
        // Parse destination
        std::getline(ss, token, ' ');
        int dst = std::stoi(token);
        
        // Parse size
        int size;
        ss >> size;
        
        // Determine type
        ss >> std::ws;
        std::string rest;
        std::getline(ss, rest);
        
        TransactionType type = (rest.find('=') != std::string::npos) 
                               ? VIRTUAL_CHANNEL 
                               : DATAGRAM;
        
        transactions.emplace_back(t, src, dst, size, type, id++);
    }
    
    file.close();
    return transactions;
}

std::string Simulation::getPath(const std::vector<int>* routeTable) const
{
    if (routeTable->empty()) {
        return "";
    }

    std::ostringstream oss;
    oss << routeTable->at(0);

    for (size_t i = 1; i < routeTable->size(); i++)
    {
        oss << "->" << routeTable->at(i);
    }

    return oss.str();
}

void Simulation::processTransaction(Transaction& trans)
{
    char channelType = (trans.type == VIRTUAL_CHANNEL) ? '+' : '-';

    // Only find and store route for VIRTUAL_CHANNEL on first packet
    if (trans.type == VIRTUAL_CHANNEL && trans.message->getDispacement() == 0)
    {
        std::vector<int> routeTable = controller.findPath(trans.src, trans.dst);
        
        if (routeTable.empty()) {
            std::cerr << "ERROR: No path found from " << trans.src << " to " << trans.dst << std::endl;
            return;
        }
        
        trans.message->setRoute(routeTable);
        
        std::string path = getPath(&routeTable);
    
        file << "M" << channelType << ' ' 
             << std::setw(5) << std::setfill('0') << simulationTime << "t " 
             << std::setw(3) << std::setfill('0') << trans.id << "id " 
             << trans.size << "w [" 
             << trans.src << "->" << trans.dst << "] "
             << path
             << std::endl;
    
        std::cout << "Time: " << trans.t 
                  << ", " << trans.src << "->" << trans.dst 
                  << ", Size: " << trans.size 
                  << ", Type: VIRTUAL_CHANNEL"
                  << ", Path: " << path
                  << std::endl;
    }
    else if (trans.type == DATAGRAM && trans.message->getDispacement() == 0)
    {
        // For DATAGRAM, just log the message start (no fixed path)
        file << "M" << channelType << ' ' 
             << std::setw(5) << std::setfill('0') << simulationTime << "t " 
             << std::setw(3) << std::setfill('0') << trans.id << "id " 
             << trans.size << "w [" 
             << trans.src << "->" << trans.dst << "]"
             << std::endl;
    
        std::cout << "Time: " << trans.t 
                  << ", " << trans.src << "->" << trans.dst 
                  << ", Size: " << trans.size 
                  << ", Type: DATAGRAM"
                  << std::endl;
    }
    
    Node& srcNode = g.nodes[trans.src];
    
    while (srcNode.buffer.size() < static_cast<std::size_t>(ROUTER_BUFFER_SIZE))
    {
        if (trans.size == trans.message->getDispacement())
        {
            break;
        }

        int packetSize;
        int displacement = trans.message->getDispacement();

        if (trans.size - displacement + header_size >= MTU)
        {
            packetSize = MTU;
            displacement = displacement + MTU - header_size;
        } 
        else
        {
            packetSize = trans.size - trans.message->getDispacement() + header_size;
            displacement = trans.size;
        }

        trans.message->setDispacement(displacement);

        // Create packet with route table only for VIRTUAL_CHANNEL
        Packet packet = (trans.type == VIRTUAL_CHANNEL)
            ? Packet::create(trans.src, trans.dst, packetSize, 
                             trans.message->getMessageId(), trans.type, trans.message->getRoute())
            : Packet::create(trans.src, trans.dst, packetSize, 
                             trans.message->getMessageId(), trans.type, nullptr);

        srcNode.buffer.push(packet);

        // Log packet creation
        std::string pathStr = "";
        if (trans.type == VIRTUAL_CHANNEL) {
            pathStr = getPath(trans.message->getRoute());
        }

        file << "P" << channelType << ' '
             << std::setw(5) << std::setfill('0') << simulationTime << "t " 
             << std::setw(3) << std::setfill('0') << trans.id << "id " 
             << std::setw(4) << std::setfill('0') << packet.getPacketId() << "pid " 
             << packet.getPacketSize() << "w {" << trans.src << "}[" 
             << trans.src << "->" << trans.dst << "] "
             << pathStr
             << std::endl;

        if (srcNode.buffer.size() == static_cast<std::size_t>(ROUTER_BUFFER_SIZE))
        {
            pq.push(trans);
            break;
        }
    }
}

void Simulation::run(const std::string& outputfile)
{
    file = std::ofstream(outputfile);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open output file " << outputfile << std::endl;
        return;
    }

    // long int simulationTime = 0;

    while (!pq.empty())
    {
        Transaction trans = pq.top();

        // If transaction time hasn't arrived yet, increment simulation time
        if (trans.t > simulationTime)
        {
            simulationTime++;
            processBuffer();
            continue;
        }
        

        // Process the transaction
        processTransaction(trans);

        // Remove processed transaction
        pq.pop();
    }
    
    file.close();
    std::cout << "\nSimulation completed. Output written to " << outputfile << std::endl;
}

void Simulation::processBuffer()
{

    for (size_t nid = 0; nid < g.nodes.size(); nid++)
    {
        Node& node = g.nodes[nid];

        if (node.buffer.empty()) continue;
        
        Packet packet = node.buffer.front();
        node.buffer.pop();
        packet.decrementTTL();

        if (packet.getTTL() == 0) continue; // drop packet and continue processing

        if (packet.getType() == VIRTUAL_CHANNEL)
        {
            // use packet's routePos to get nextNode
            int nextNode = packet.getNextNode(g, nid);

            cout << '[' << nid << ':' << nextNode << ']' << endl;

            Edge *edge = findEdgePtr(nid, nextNode);
            
            if (edge->type == HALF_DUPLEX)
            {
                if (edge->buffer.size() > 0)
                {
                    node.buffer.push(packet);
                }
                else 
                {
                    packet.setSendingTime(ceil((float)packet.getPacketSize() / (edge->weight.bandwidth_mbps * 1024)));
                    packet.setTransmissionUntil(simulationTime + edge->weight.latency_ms + packet.getSendingTime());
                    outputPacketTransaction(packet, *edge);
                    // deliver packet to next node directly
                    edge->buffer.push(packet);
                    // if we need to transmit in half_duplex mode  
                }
            }
            else
            {
                // duplex mode
            }
            // node.second.
        }
    }
}

int Simulation::findNextVirtual(int from, const vector<int>& routeTable)
{
    if (routeTable.size() < 2) return -1;

    for (size_t i = 0; i + 1 < routeTable.size(); i++)
    {
        cout << '|' << i << ' ' << routeTable[i] << endl;
        if (from == routeTable[i])
        {
            return routeTable[i + 1];
        }
    }

    return -1;
}

void Simulation::outputPacketTransaction(Packet& packet, Edge& edge)
{
     file << "PT" << ' '
             << std::setw(5) << std::setfill('0') << simulationTime << "t " 
             << std::setw(3) << std::setfill('0') << packet.getMessageId() << "id " 
             << std::setw(4) << std::setfill('0') << packet.getPacketId() << "pid " 
             << packet.getPacketSize() << "w {" << "->" << edge.to << "}["
             << packet.getSrc() << "->" << packet.getDst() << "] "
             << (packet.getType() == VIRTUAL_CHANNEL ?  getPath(packet.getRouteTable()) : "")
             << std::endl;
}

Edge* Simulation::findEdgePtr(int from, int to) const
{
    for (auto& edge : g.adj[from])
    {
        if (edge.to == to)
        {
            return &edge;
        }
    }
    return nullptr;
}