#include "Packet.hpp"
#include "Global.hpp"
#include "Simulation.hpp"

// Packet static member initialization
int Packet::nextPacketId = 1;

// Packet implementation
Packet::Packet(int packetId, int src, int dst, int packetSize, int messageId, int type, const std::vector<int>* routeTable)
    : packetId(packetId), src(src), dst(dst), packetSize(packetSize), messageId(messageId), type(type), transmissionUntil(0), sendingTime(0), ttl(TTL), routePos(0)
{
    if (type == VIRTUAL_CHANNEL && routeTable != nullptr)
    {
        this->routeTable = routeTable;
    }
}

Packet Packet::create(int src, int dst, int packetSize, int messageId, int type, const std::vector<int>* routeTable = nullptr) 
{
    return Packet(nextPacketId++, src, dst, packetSize, messageId, type, routeTable);
}

int Packet::getNextNode(const Graph& g, int currentNode) const 
{
    if (type == VIRTUAL_CHANNEL) {
        // Use fixed route table
        if (routeTable && routePos + 1 < (int)routeTable->size()) {
            return (*routeTable)[routePos + 1];
        }
        return -1;
    } else {
        // DATAGRAM: use node's routing table to determine next hop
        // This will be handled by the router logic, return -1 here
        return -1;
    }
}