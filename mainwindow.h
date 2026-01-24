#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// AUDIO
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioFormat>

// NETWORK
#include <QUdpSocket>
#include <QHostAddress>
#include <QRandomGenerator>
#include <QTimer>

// IO
#include <QIODevice>

// SERVER (Thread için)
#include <QThread>
#include <memory>
#include "server/voiceserver.h"

// Boost için forward declaration
namespace boost { namespace asio { class io_context; } }

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_micButton_clicked();    // Mikrofon toggle butonu
    void on_audioButton_clicked();  // Kulaklık toggle butonu

    void onUdpReadyRead();   // gelen datagramları okuyacağımız slot
    void onAudioReadyRead(); // mikrofondan yeni ses geldiğinde

    void on_onlineButton_clicked();
    void sendKeepAlive();
    
    void on_hostButton_clicked();  // Host Server butonu

private:
    Ui::MainWindow *ui;

    // SES
    QAudioSource *m_audioSource   = nullptr; // 🔹 Mikrofon (QAudioInput değil!)
    QAudioSink   *m_audioOutput   = nullptr; // Hoparlör
    QIODevice    *m_inputDevice   = nullptr; // Mikrofondan okuma
    QIODevice    *m_outputDevice  = nullptr; // Hoparlöre yazma

    // NETWORK
    QUdpSocket   *m_udpSocket     = nullptr;
    QHostAddress  m_remoteAddress;
    quint16       m_remotePort    = 0;
    quint16       m_localPort     = 0;

    bool          m_isStreaming   = false;
    bool          m_isOnline = false;
    bool          m_isDeafened = false;  // Kulaklık kapalı mı?

    // KEEPALIVE
    QTimer       *m_keepAliveTimer = nullptr;

    // EMBEDDED SERVER
    bool                              m_isHosting = false;
    std::unique_ptr<boost::asio::io_context> m_ioContext;
    std::unique_ptr<VoiceServer>      m_voiceServer;
    QThread                          *m_serverThread = nullptr;
    
    void startServer();
    void stopServer();
    QString getLocalIPAddress();

};

#endif // MAINWINDOW_H
