#include "voiceserver.h"
#include <iostream>
#include <algorithm>

using boost::asio::ip::udp;


VoiceServer::VoiceServer(boost::asio::io_context& io, unsigned short port)
    : socket_(io)  // Socket'i bind etmeden oluştur
{
    // Socket'i aç
    socket_.open(udp::v4());
    
    // SO_REUSEADDR ayarla - aynı portu tekrar kullanabilmek için
    socket_.set_option(boost::asio::socket_base::reuse_address(true));
    
    // Şimdi bind et
    socket_.bind(udp::endpoint(udp::v4(), port));
    
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
            if(ec)
            {
                //Socket kapatıldıysa sessizce çık
                return;
            }
            if(bytesReceived > 0)
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

void VoiceServer::stop(){
    boost::system::error_code ec;
    socket_.close(ec);
    if(ec){
        std::cerr << "[SERVER] Error closing socket: " << ec.message() << "\n";
    } else {
        std::cout << "[SERVER] Voice server stopped.\n";
    }
}