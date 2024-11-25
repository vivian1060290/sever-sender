#include "tcpfileserver.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QFile>

TcpFileServer::TcpFileServer(QWidget *parent)
    : QWidget(parent),
    tcpServer(new QTcpServer(this)),
    tcpServerConnection(nullptr),
    localFile(nullptr),
    progressBar(new QProgressBar(this)),
    totalBytes(0),
    bytesReceived(0),
    fileNameSize(0)
{
    // 設置 UI
    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *startButton = new QPushButton("Start Server", this);
    layout->addWidget(startButton);
    layout->addWidget(progressBar);

    connect(startButton, &QPushButton::clicked, this, &TcpFileServer::startServer);

    progressBar->setRange(0, 100); // 設置進度條範圍
    progressBar->setValue(0); // 初始化進度條值

    // 開啟服務器
    if (!tcpServer->listen(QHostAddress::Any, 8888)) {
        QMessageBox::critical(this, tr("Server Error"), tr("Unable to start the server"));
        close();
    } else {
        connect(tcpServer, &QTcpServer::newConnection, this, &TcpFileServer::acceptConnection);
    }
}

TcpFileServer::~TcpFileServer()
{
    if (localFile) {
        localFile->close();
    }
}

void TcpFileServer::startServer()
{
    // 準備進行文件接收
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::List);
    QString fileName = dialog.getSaveFileName(this, tr("Save File"));

    if (fileName.isEmpty()) {
        return; // 如果沒有選擇文件，則退出
    }

    localFile = new QFile(fileName);
    if (!localFile->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to open file for writing"));
        return;
    }

    bytesReceived = 0;
    totalBytes = 0;
    fileNameSize = 0;
    currentFileName.clear();
    progressBar->setValue(0); // 重置進度條
}

void TcpFileServer::acceptConnection()
{
    tcpServerConnection = tcpServer->nextPendingConnection();

    connect(tcpServerConnection, &QTcpSocket::readyRead, this, &TcpFileServer::updateServerProgress);
    connect(tcpServerConnection, &QTcpSocket::errorOccurred, this, &TcpFileServer::displayError);
}

void TcpFileServer::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_5_15);

    if (bytesReceived <= sizeof(qint64) * 2) {
        // 如果是第一次接收，先接收文件名大小和文件大小
        if ((tcpServerConnection->bytesAvailable() >= sizeof(qint64) * 2) && (fileNameSize == 0)) {
            in >> fileNameSize >> totalBytes;
            bytesReceived += sizeof(qint64) * 2;
        }

        // 接收文件名
        if (tcpServerConnection->bytesAvailable() >= fileNameSize) {
            in >> currentFileName;
            bytesReceived += fileNameSize;

            // 文件開始接收
            if (!localFile->open(QIODevice::WriteOnly)) {
                QMessageBox::critical(this, tr("Error"), tr("Unable to open file for writing"));
                return;
            }
        }
    }

    // 接收文件數據
    if (bytesReceived < totalBytes) {
        outBlock = tcpServerConnection->readAll();
        bytesReceived += outBlock.size();
        localFile->write(outBlock);
        outBlock.resize(0);

        // 更新進度條
        progressBar->setValue((bytesReceived * 100) / totalBytes);
    }

    // 完成接收
    if (bytesReceived == totalBytes) {
        localFile->flush();
        localFile->close();
        tcpServerConnection->close();
        progressBar->setValue(100);
        QMessageBox::information(this, tr("File Transfer"), tr("File received successfully"));
    }
}

void TcpFileServer::displayError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QMessageBox::critical(this, tr("Server Error"), tcpServerConnection->errorString());
    tcpServerConnection->close();
}
