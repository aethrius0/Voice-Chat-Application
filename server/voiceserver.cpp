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

void VoiceServer::cleanupOldClients()
{
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = clients_.begin(); it != clients_.end(); )
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
        if (elapsed > clientTimeoutSeconds)
        {
            std::cout << "[SERVER] Client timed out: " << it->first.address().to_string() 
                      << ":" << it->first.port() << "\n";
            it = clients_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void VoiceServer::handlePacket(size_t bytesReceived){

    auto now = std::chrono::steady_clock::now();
    
    // Client'ı güncelle veya ekle
    auto it = clients_.find(remoteEndpoint_);
    if (it == clients_.end())
    {
        std::cout << "[SERVER] New client joined: " << remoteEndpoint_.address().to_string() 
                  << ":" << remoteEndpoint_.port() << "\n";
    }
    clients_[remoteEndpoint_] = now;
    
    // Eski client'ları temizle
    cleanupOldClients();

    // Diğer client'lara broadcast
    for (auto& [endpoint, lastSeen] : clients_)
    {
        if (endpoint == remoteEndpoint_) continue;

        socket_.async_send_to(
            boost::asio::buffer(recvBuffer_.data(), bytesReceived), endpoint,
            [](auto,auto) {}
            );
    }
}
