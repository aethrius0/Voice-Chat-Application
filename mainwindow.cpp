#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Başlangıçta offline - butonlar disabled
    m_isOnline = false;
    ui->recordButton->setEnabled(false);
    ui->stopButton->setEnabled(false);

    //---- SES AYARLARI ----

    // 1- AUDIO DEVICES

    // Varsayılan giriş cihazı
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();

    // Varsayılan çıkış cihazı
    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();

    if (inputDevice.isNull() || outputDevice.isNull()) {
        QMessageBox::warning(this, "Hata", "Ses cihazı bulunamadı!");
        ui->recordButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->onlineButton->setEnabled(false);
        return;
    }

    // Formatlar - HER İKİ TARAFTA AYNI OLMALI
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    // Mikrofon ve hoparlör objeleri
    m_audioSource = new QAudioSource(inputDevice, format, this);
    m_audioOutput = new QAudioSink(outputDevice, format, this);

    // Hoparlörü hemen başlat
    m_outputDevice = m_audioOutput->start();
    if (!m_outputDevice) {
        qWarning() << "Audio output device başlatılamadı!";
    }

    // 2- UDP SOKETİ
    m_udpSocket = new QUdpSocket(this);

    // Sabit bir port kullan (her seferinde aynı port = server'da aynı client)
    m_localPort = 45000 + (QRandomGenerator::global()->bounded(1000));
    
    if (!m_udpSocket->bind(QHostAddress::AnyIPv4, m_localPort)) {
        // Port kullanımdaysa rastgele port dene
        m_udpSocket->bind(QHostAddress::AnyIPv4, 0);
        m_localPort = m_udpSocket->localPort();
    }

    qDebug() << "Local UDP port:" << m_localPort;

    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &MainWindow::onUdpReadyRead);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Record butonu
void MainWindow::on_recordButton_clicked()
{
    if (!m_isOnline) {
        qWarning() << "Offline iken konuşamazsın.";
        return;
    }

    if (m_isStreaming)
        return;

    if (!m_audioSource) {
        qWarning() << "Audio source yok..";
        return;
    }

    m_isStreaming = true;

    // Mikrofondan veri almayı başlat
    m_inputDevice = m_audioSource->start();   // 🔹 QAudioSource::start()
    if (!m_inputDevice) {
        qWarning() << "Audio source device başlatılamadı!";
        m_isStreaming = false;
        return;
    }

    connect(m_inputDevice, &QIODevice::readyRead,
            this, &MainWindow::onAudioReadyRead,
            Qt::UniqueConnection);

    // Buton durumları
    ui->recordButton->setEnabled(false);
    ui->stopButton->setEnabled(true);

    qDebug() << "Streaming started..";
}

// Stop butonu
void MainWindow::on_stopButton_clicked()
{
    if (!m_isStreaming)
        return;

    m_isStreaming = false;

    if (m_audioSource) {
        m_audioSource->stop();   // 🔹 QAudioSource::stop()
    }

    m_inputDevice = nullptr;

    ui->recordButton->setEnabled(true);
    ui->stopButton->setEnabled(false);

    qDebug() << "Streaming stopped..";
}

// SES -> UDP
void MainWindow::onAudioReadyRead()
{
    if (!m_isStreaming || !m_inputDevice || !m_udpSocket || !m_isOnline)
        return;

    QByteArray data = m_inputDevice->readAll();
    if (data.isEmpty())
        return;

    // Ses frame'ini UDP ile gönder
    qint64 sent = m_udpSocket->writeDatagram(
        data,
        m_remoteAddress,
        m_remotePort
        );

    if (sent == -1) {
        qWarning() << "UDP send error " << m_udpSocket->errorString();
    }
}

// UDP -> SES
void MainWindow::onUdpReadyRead()
{
    if (!m_outputDevice || !m_isOnline)
        return;

    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(int(m_udpSocket->pendingDatagramSize()));

        m_udpSocket->readDatagram(buffer.data(), buffer.size());

        // Gelen ses verisini hoparlöre ver
        m_outputDevice->write(buffer);
    }
}

void MainWindow::on_onlineButton_clicked()
{
    if (!m_isOnline) {
        // CONNECT
        QString ipText = ui->serverIpEdit->text().trimmed();
        
        if (ipText.isEmpty()) {
            QMessageBox::warning(this, "Hata", "Server IP adresi girin!");
            return;
        }

        QHostAddress serverAddr(ipText);
        if (serverAddr.isNull()) {
            QMessageBox::warning(this, "Hata", "Geçersiz IP adresi!");
            return;
        }

        m_remoteAddress = serverAddr;
        m_remotePort = 50000;  // Server portu

        m_isOnline = true;
        ui->onlineButton->setText("Disconnect");
        ui->serverIpEdit->setEnabled(false);
        ui->statusLabel->setText("🟢 Connected to " + ipText);
        ui->recordButton->setEnabled(true);
        ui->stopButton->setEnabled(false);

        // Keepalive timer başlat - her 3 saniyede server'a sinyal gönder
        if (!m_keepAliveTimer) {
            m_keepAliveTimer = new QTimer(this);
            connect(m_keepAliveTimer, &QTimer::timeout, this, &MainWindow::sendKeepAlive);
        }
        m_keepAliveTimer->start(3000);  // 3 saniye
        
        // Hemen bir keepalive gönder (server'a kayıt ol)
        sendKeepAlive();

        qDebug() << "Connected to server:" << ipText << ":" << m_remotePort;

    } else {
        // DISCONNECT
        if (m_isStreaming) {
            on_stopButton_clicked();
        }

        // Keepalive timer durdur
        if (m_keepAliveTimer) {
            m_keepAliveTimer->stop();
        }

        m_isOnline = false;
        ui->onlineButton->setText("Connect");
        ui->serverIpEdit->setEnabled(true);
        ui->statusLabel->setText("⚫ Disconnected");
        ui->recordButton->setEnabled(false);
        ui->stopButton->setEnabled(false);

        qDebug() << "Disconnected from server";
    }
}

// Server'a keepalive paketi gönder (bağlantıyı canlı tut)
void MainWindow::sendKeepAlive()
{
    if (!m_isOnline || !m_udpSocket)
        return;
    
    // Boş bir byte gönder - server bizi aktif olarak görsün
    QByteArray keepAlive(1, 0);
    m_udpSocket->writeDatagram(keepAlive, m_remoteAddress, m_remotePort);
}
