#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //---- SES AYARLARI ----

    // Başlangıçta buton durumu
    ui->recordButton->setEnabled(true);
    ui->stopButton->setEnabled(false);

    // 1- AUDIO DEVICES

    // Varsayılan giriş cihazı
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();

    // Varsayılan çıkış cihazı
    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();

    if (inputDevice.isNull() || outputDevice.isNull()) {
        qWarning() << "Ses cihazı bulunamadı...";

        ui->recordButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        return;
    }

    // Formatlar
    QAudioFormat inFormat  = inputDevice.preferredFormat();
    QAudioFormat outFormat = outputDevice.preferredFormat();

    // Mikrofon ve hoparlör objeleri
    m_audioSource  = new QAudioSource(inputDevice, inFormat, this);     // 🔹 QAudioSource
    m_audioOutput  = new QAudioSink(outputDevice, outFormat, this);

    // Hoparlörü hemen başlat, QIODevice elde et
    m_outputDevice = m_audioOutput->start();
    if (!m_outputDevice) {
        qWarning() << "Audio output device başlatılamadı..";
    }

    // 2- UDP SOKETİ

    m_udpSocket = new QUdpSocket(this);

    // Uygulamanın dinleyeceği port
    quint16 localPort = 45454;
    m_udpSocket->bind(QHostAddress::AnyIPv4, localPort);

    // Karşı taraf portu (şimdilik kendimiz)
    m_remoteAddress = QHostAddress("127.0.0.1");
    m_remotePort    = 45454;

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
    if (!m_isStreaming || !m_inputDevice || !m_udpSocket)
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
    if (!m_outputDevice)
        return;

    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(int(m_udpSocket->pendingDatagramSize()));

        m_udpSocket->readDatagram(buffer.data(), buffer.size());

        // Gelen ses verisini hoparlöre ver
        m_outputDevice->write(buffer);
    }
}
