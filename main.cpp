#include "mainwindow.h"

#include <QApplication>

#include "server/voiceserver.h"
#include <boost/asio.hpp>
#include <thread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    boost::asio::io_context io;
    VoiceServer server(io, 50000); //server port

    //server için thread
    std::thread serverThread([&io, &server](){
        server.start();
        io.run();
    });

    MainWindow w;
    w.show();

    int result = a.exec(); //GUI kapanana kadar bekle

    //Temiz kapatma
    server.stop();
    io.stop();
    serverThread.join();


    return result;
}
