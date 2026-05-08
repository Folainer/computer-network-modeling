#ifndef PACKET_HPP
#define PACKET_HPP

#include <vector>
#include <memory>

class Graph;
#include "Global.hpp"

class Packet {
public:
    static Packet create(int src, int dst, int packetSize, int messageId, int type, const std::vector<int>* routeTable);
    static void reset();

    int getPacketId() const { return packetId; }
    int getMessageId() const { return messageId; }
    int getPacketSize() const { return packetSize; }
    int getType() const { return type; }
    long getTransmissionUntil() const { return transmissionUntil; }
    void setTransmissionUntil(long int newValue) { transmissionUntil = newValue; }
    int getSendingTime() const { return sendingTime; }
    void setSendingTime(int newValue) { sendingTime = newValue; }
    int getTTL() const { return ttl; }
    int getSrc() const { return src; }
    int getDst() const { return dst; }
    void decrementTTL() { ttl--; }
    const std::shared_ptr<const std::vector<int>>& getRouteTable() const { return routeTable; }
    int getNextNode(const Graph& g, int currentNode) const;
    void incrementRoutePos() { routePos++; }
    void decrementRoutePos() { routePos--; }
    ~Packet();
    
private:
    Packet(int packetId, int src, int dst, int packetSize, int messageId, int type, const std::vector<int>* routeTable);

    static int nextPacketId;

    int packetId;
    int src;
    int dst;
    int packetSize;
    int messageId;
    int type;
    long int transmissionUntil;
    int sendingTime;
    int ttl;
    std::shared_ptr<const std::vector<int>> routeTable; 

    int routePos;
};

#endif
