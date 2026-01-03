#include "voiceserver.h"
#include <boost/asio.hpp>
#include <iostream>

int main()
{
    try
    {
        boost::asio::io_context io;
        unsigned short port = 50000;

        VoiceServer server(io, port);
        server.start();

        std::cout << "[SERVER] Running on UDP port " << port << " ...\n";

        io.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "[SERVER] Exception: " << e.what() << "\n";
        std::cout << "Press Enter to exit...";
        std::cin.get();
    }
}
