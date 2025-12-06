#ifndef MESSAGE_STATS_HPP
#define MESSAGE_STATS_HPP

struct MessageStats {
    int packetsSent = 0;
    int bytesSent = 0;
    int bytesTotalSent = 0;
    int packetsReceived = 0;
    int bytesReceived = 0;
    int bytesTotalReceived = 0;
    int packetsDropped = 0;
    bool delivered = false;  // final message delivered?
    int firstSendTime = 0;
    int deliveryTime = 0;
};

#endif