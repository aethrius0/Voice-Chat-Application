#ifndef VOICESERVER_H
#define VOICESERVER_H

#include <boost/asio.hpp>
#include <array>
#include <vector>
#include <map>
#include <chrono>


class VoiceServer
{
public:
    VoiceServer(boost::asio::io_context& io, unsigned short port);

    void start();


private:

    void startReceive();
    void handlePacket(size_t bytesReceived);
    void cleanupOldClients();

    static constexpr size_t maxPacketSize = 1500;
    static constexpr int clientTimeoutSeconds = 10;

    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remoteEndpoint_;
    std::array<char, maxPacketSize> recvBuffer_;

    // Client endpoint -> last seen time
    std::map<boost::asio::ip::udp::endpoint, std::chrono::steady_clock::time_point> clients_;

};



#endif // VOICESERVER_H
