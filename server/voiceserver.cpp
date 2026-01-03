#include "voiceserver.h"
#include <iostream>
#include <algorithm>

using boost::asio::ip::udp;


VoiceServer::VoiceServer(boost::asio::io_context& io, unsigned short port)
    : socket_(io, udp::endpoint(udp::v4(),port))
{
    std::cout << "[SERVER] Listening UDP on port " << port << "\n";

}

void VoiceServer::start(){
    startReceive();
}

void VoiceServer::startReceive(){

    socket_.async_receive_from(
        boost::asio::buffer(recvBuffer_),remoteEndpoint_,
        [this](const boost::system::error_code& ec, std::size_t bytesReceived)
        {
            if(!ec && bytesReceived > 0)
            {
                handlePacket(bytesReceived);
            }
            startReceive();
        }

        );
}

void VoiceServer::handlePacket(size_t bytesReceived){

    // Client listede var mı kontrol et
    auto it = std::find(clients_.begin(), clients_.end(), remoteEndpoint_);

    if(it == clients_.end())
    {
        clients_.push_back(remoteEndpoint_);
        std::cout << "[SERVER] New client joined: " << remoteEndpoint_.address().to_string() 
                  << ":" << remoteEndpoint_.port() << "\n";
    }

    // Diğer client'lara broadcast
    for (auto& c : clients_)
    {
        if(c == remoteEndpoint_) continue;

        socket_.async_send_to(
            boost::asio::buffer(recvBuffer_.data(), bytesReceived), c,
            [](auto,auto) {}
            );
    }
}
