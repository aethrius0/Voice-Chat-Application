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
    void on_recordButton_clicked();
    void on_stopButton_clicked();

    void onUdpReadyRead();   // gelen datagramları okuyacağımız slot
    void onAudioReadyRead(); // mikrofondan yeni ses geldiğinde

    void on_onlineButton_clicked();
    void sendKeepAlive();

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

    // KEEPALIVE
    QTimer       *m_keepAliveTimer = nullptr;

};

#endif // MAINWINDOW_H
