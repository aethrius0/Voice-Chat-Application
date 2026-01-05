#ifndef VOICESERVER_H
#define VOICESERVER_H

#include <boost/asio.hpp>
#include <array>
#include <vector>


class VoiceServer
{
public:
    VoiceServer(boost::asio::io_context& io, unsigned short port);

    void start();
    void stop();


private:

    void startReceive();
    void handlePacket(size_t bytesReceived);

    static constexpr size_t maxPacketSize = 1500;

    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remoteEndpoint_;
    std::array<char, maxPacketSize> recvBuffer_;

    std::vector<boost::asio::ip::udp::endpoint> clients_;

};



#endif // VOICESERVER_H
