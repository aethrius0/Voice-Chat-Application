#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "config.h"

#include <QDebug>
#include <QMessageBox>
#include <QNetworkInterface>
#include <boost/asio.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Minimal glassmorphism theme
    ui->micButton->setEnabled(false);
    m_isOnline = false;

    //---- SES AYARLARI ----

    // 1- AUDIO DEVICES

    // Varsayılan giriş cihazı
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();

    // Varsayılan çıkış cihazı
    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();

    if (inputDevice.isNull() || outputDevice.isNull()) {
        QMessageBox::warning(this, "Hata", "Ses cihazı bulunamadı!");
        ui->micButton->setEnabled(false);
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
    
    // VPS IP'sini otomatik yükle
    ui->serverIpEdit->setText(VPS_SERVER_IP);
    
    // Butonlar başlangıçta disabled
    ui->micButton->setEnabled(false);
    ui->audioButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Mikrofon toggle butonu (Discord tarzı)
void MainWindow::on_micButton_clicked()
{
    if (!m_isOnline) {
        qWarning() << "Offline iken mikrofon açılamaz.";
        return;
    }

    if (!m_isStreaming) {
        // MİKROFON AÇ
        if (!m_audioSource) {
            qWarning() << "Audio source yok..";
            return;
        }

        m_inputDevice = m_audioSource->start();
        if (!m_inputDevice) {
            qWarning() << "Audio source device başlatılamadı!";
            return;
        }

        connect(m_inputDevice, &QIODevice::readyRead,
                this, &MainWindow::onAudioReadyRead,
                Qt::UniqueConnection);

        m_isStreaming = true;
        
        // Mikrofon açık - yeşil
        ui->micButton->setText("🎤");
        ui->micButton->setStyleSheet(R"(
            QPushButton {
                background-color: #a6e3a1;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:hover { background-color: #94e2d5; }
        )");
        ui->micStatusLabel->setText("Açık");
        ui->micStatusLabel->setStyleSheet("font-size: 10px; color: #a6e3a1;");
        
        qDebug() << "Mikrofon açıldı";
    } 
    else {
        // MİKROFON KAPAT
        if (m_audioSource) {
            m_audioSource->stop();
        }
        m_inputDevice = nullptr;
        m_isStreaming = false;
        
        // Mikrofon kapalı
        ui->micButton->setText("🔇");
        ui->micButton->setStyleSheet(R"(
            QPushButton {
                background-color: #45475a;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:hover { background-color: #585b70; }
        )");
        ui->micStatusLabel->setText("Kapalı");
        ui->micStatusLabel->setStyleSheet("font-size: 10px; color: #6c7086;");
        
        qDebug() << "Mikrofon kapatıldı";
    }
}

// Kulaklık toggle butonu (Ses çıkışı aç/kapa)
void MainWindow::on_audioButton_clicked()
{
    if (!m_isOnline) {
        return;
    }

    if (!m_isDeafened) {
        // SES KAPAT (Deafen)
        if (m_audioOutput) {
            m_audioOutput->suspend();
        }
        m_isDeafened = true;
        
        ui->audioButton->setText("🔇");
        ui->audioButton->setStyleSheet(R"(
            QPushButton {
                background-color: #f38ba8;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:hover { background-color: #eba0ac; }
        )");
        ui->audioStatusLabel->setText("Kapalı");
        ui->audioStatusLabel->setStyleSheet("font-size: 10px; color: #f38ba8;");
        
        qDebug() << "Ses çıkışı kapatıldı (Deafened)";
    } 
    else {
        // SES AÇ
        if (m_audioOutput) {
            m_audioOutput->resume();
        }
        m_isDeafened = false;
        
        ui->audioButton->setText("🎧");
        ui->audioButton->setStyleSheet(R"(
            QPushButton {
                background-color: #a6e3a1;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:hover { background-color: #94e2d5; }
        )");
        ui->audioStatusLabel->setText("Açık");
        ui->audioStatusLabel->setStyleSheet("font-size: 10px; color: #a6e3a1;");
        
        qDebug() << "Ses çıkışı açıldı";
    }
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

        // Deafen modunda ses çıkışı yapma
        if (!m_isDeafened) {
            m_outputDevice->write(buffer);
        }
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
        
        // Bağlı stili
        ui->onlineButton->setText("Çık");
        ui->onlineButton->setStyleSheet(R"(
            QPushButton {
                background-color: #f38ba8;
                color: #1e1e2e;
                font-size: 12px;
            }
            QPushButton:hover { background-color: #eba0ac; }
        )");
        ui->serverIpEdit->setEnabled(false);
        ui->statusLabel->setText("● " + ipText);
        ui->statusLabel->setStyleSheet("font-size: 12px; color: #a6e3a1;");
        
        // Mikrofon ve kulaklık butonlarını aktif et
        ui->micButton->setEnabled(true);
        ui->audioButton->setEnabled(true);
        
        // Mikrofonu otomatik aç
        on_micButton_clicked();
        
        // Kulaklık başlangıçta açık
        ui->audioButton->setText("🎧");
        ui->audioButton->setStyleSheet(R"(
            QPushButton {
                background-color: #a6e3a1;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:hover { background-color: #94e2d5; }
        )");
        ui->audioStatusLabel->setText("Açık");
        ui->audioStatusLabel->setStyleSheet("font-size: 10px; color: #a6e3a1;");

        // Keepalive timer başlat
        if (!m_keepAliveTimer) {
            m_keepAliveTimer = new QTimer(this);
            connect(m_keepAliveTimer, &QTimer::timeout, this, &MainWindow::sendKeepAlive);
        }
        m_keepAliveTimer->start(3000);
        sendKeepAlive();

        qDebug() << "Connected to server:" << ipText << ":" << m_remotePort;

    } else {
        // DISCONNECT
        
        if (m_isStreaming) {
            on_micButton_clicked();
        }

        if (m_keepAliveTimer) {
            m_keepAliveTimer->stop();
        }

        m_isOnline = false;
        m_isDeafened = false;
        
        // Çevrimdışı stili
        ui->onlineButton->setText("Bağlan");
        ui->onlineButton->setStyleSheet(R"(
            QPushButton {
                background-color: #a6e3a1;
                color: #1e1e2e;
                font-size: 12px;
            }
            QPushButton:hover { background-color: #94e2d5; }
        )");
        ui->serverIpEdit->setEnabled(true);
        ui->statusLabel->setText("● Çevrimdışı");
        ui->statusLabel->setStyleSheet("font-size: 12px; color: #f38ba8;");
        
        // Mikrofon butonunu deaktif et
        ui->micButton->setEnabled(false);
        ui->micButton->setText("🎤");
        ui->micButton->setStyleSheet(R"(
            QPushButton {
                background-color: #313244;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:disabled { background-color: #313244; }
        )");
        ui->micStatusLabel->setText("");
        
        // Kulaklık butonunu deaktif et
        ui->audioButton->setEnabled(false);
        ui->audioButton->setText("🎧");
        ui->audioButton->setStyleSheet(R"(
            QPushButton {
                background-color: #313244;
                border-radius: 30px;
                font-size: 28px;
            }
            QPushButton:disabled { background-color: #313244; }
        )");
        ui->audioStatusLabel->setText("");

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

// ==================== SERVER FUNCTIONS ====================

void MainWindow::on_hostButton_clicked()
{
    if (!m_isHosting) {
        startServer();
    } else {
        stopServer();
    }
}

void MainWindow::startServer()
{
    // Zaten çalışıyorsa bir şey yapma
    if (m_isHosting) {
        return;
    }
    
    // Önce eski kaynakları temizle (varsa)
    if (m_voiceServer) {
        m_voiceServer->stop();
        m_voiceServer.reset();
    }
    if (m_ioContext) {
        m_ioContext->stop();
        m_ioContext.reset();
    }
    if (m_serverThread) {
        m_serverThread->quit();
        m_serverThread->wait(1000);
        delete m_serverThread;
        m_serverThread = nullptr;
    }
    
    try {
        // io_context ve server oluştur
        m_ioContext = std::make_unique<boost::asio::io_context>();
        m_voiceServer = std::make_unique<VoiceServer>(*m_ioContext, 50000);
        m_voiceServer->start();
        
        // Ayrı thread'de çalıştır
        m_serverThread = QThread::create([this]() {
            try {
                m_ioContext->run();
            } catch (...) {
            }
        });
        m_serverThread->start();
        
        m_isHosting = true;
        
        // Sunucu açık stili
        ui->hostButton->setText("Durdur");
        ui->hostButton->setStyleSheet(R"(
            QPushButton {
                background-color: #f38ba8;
                color: #1e1e2e;
                font-size: 11px;
                border-radius: 6px;
            }
            QPushButton:hover { background-color: #eba0ac; }
        )");
        
        QString localIP = getLocalIPAddress();
        ui->hostStatusLabel->setText(localIP + ":50000");
        ui->hostStatusLabel->setStyleSheet("font-size: 10px; color: #a6e3a1;");
        ui->serverIpEdit->setText("127.0.0.1");
        
        qDebug() << "Server started on port 50000";
        qDebug() << "Local IP:" << localIP;
        
    } catch (const std::exception& e) {
        m_voiceServer.reset();
        m_ioContext.reset();
        m_isHosting = false;
        
        QMessageBox::critical(this, "Hata", 
            QString("Server başlatılamadı!\n\nPort 50000 başka bir uygulama tarafından kullanılıyor olabilir.\n\nDetay: %1").arg(e.what()));
    }
}

void MainWindow::stopServer()
{
    // Önce client bağlantısını kes
    if (m_isOnline) {
        on_onlineButton_clicked();  // Disconnect
    }
    
    // Server'ı durdur
    if (m_voiceServer) {
        m_voiceServer->stop();
    }
    
    if (m_ioContext) {
        m_ioContext->stop();
    }
    
    if (m_serverThread) {
        m_serverThread->quit();
        m_serverThread->wait(2000);
        delete m_serverThread;
        m_serverThread = nullptr;
    }
    
    m_voiceServer.reset();
    m_ioContext.reset();
    
    m_isHosting = false;
    
    // Sunucu kapalı stili
    ui->hostButton->setText("Başlat");
    ui->hostButton->setStyleSheet(R"(
        QPushButton {
            background-color: #45475a;
            color: #cdd6f4;
            font-size: 11px;
            border-radius: 6px;
        }
        QPushButton:hover { background-color: #585b70; }
    )");
    ui->hostStatusLabel->setText("Kapalı");
    ui->hostStatusLabel->setStyleSheet("font-size: 10px; color: #6c7086;");
    
    qDebug() << "Server stopped";
}

QString MainWindow::getLocalIPAddress()
{
    const QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol 
            && !address.isLoopback()) {
            return address.toString();
        }
    }
    return "127.0.0.1";
}
