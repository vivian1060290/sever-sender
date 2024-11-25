#include "tcpfilesender.h"
#include <QFileDialog>
#include <QMessageBox>

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent),
    tcpClient(new QTcpSocket(this)),
    localFile(nullptr),
    totalBytes(0),
    bytesWritten(0),
    bytesToWrite(0),
    loadSize(64 * 1024)
{
    // 創建分頁
    tabWidget = new QTabWidget(this);

    // 傳送分頁
    sendTab = new QWidget(this);
    clientProgressBar = new QProgressBar(sendTab);
    clientStatusLabel = new QLabel(tr("客戶端就緒"), sendTab);
    startButton = new QPushButton(tr("開始"), sendTab);
    quitButton = new QPushButton(tr("退出"), sendTab);
    openButton = new QPushButton(tr("開檔"), sendTab);

    // IP 和 Port 輸入框及標籤
    ipLabel = new QLabel(tr("IP:"), sendTab);
    ipLineEdit = new QLineEdit(sendTab);
    portLabel = new QLabel(tr("Port:"), sendTab);
    portLineEdit = new QLineEdit(sendTab);

    // 設置預設值
    ipLineEdit->setText("127.0.0.1");
    portLineEdit->setText("12345");

    ipLineEdit->setPlaceholderText("Server IP");
    portLineEdit->setPlaceholderText("Server Port");

    // 排版
    QVBoxLayout* sendLayout = new QVBoxLayout(sendTab);

    sendLayout->addWidget(clientProgressBar);
    sendLayout->addWidget(clientStatusLabel);

    // 添加 IP 標籤和輸入框
    QHBoxLayout* ipLayout = new QHBoxLayout();
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(ipLineEdit);
    sendLayout->addLayout(ipLayout);

    // 添加 Port 標籤和輸入框
    QHBoxLayout* portLayout = new QHBoxLayout();
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portLineEdit);
    sendLayout->addLayout(portLayout);

    // 按鈕水平佈局
    QHBoxLayout* sendButtonLayout = new QHBoxLayout();
    sendButtonLayout->addWidget(openButton);
    sendButtonLayout->addWidget(startButton);
    sendButtonLayout->addWidget(quitButton);
    sendLayout->addLayout(sendButtonLayout);

    sendTab->setLayout(sendLayout);

    // 接收分頁
    receiveTab = new QWidget(this);
    serverProgressBar = new QProgressBar(receiveTab);
    receiveStatusLabel = new QLabel(tr("伺服端就緒"), receiveTab);
    receiveButton = new QPushButton(tr("接收"), receiveTab);
    quitReceiveButton = new QPushButton(tr("退出"), receiveTab);

    // 添加 IP 和 Port 輸入框
    receiveIpLabel = new QLabel(tr("IP:"), receiveTab);
    receiveIpLineEdit = new QLineEdit(receiveTab);
    receivePortLabel = new QLabel(tr("Port:"), receiveTab);
    receivePortLineEdit = new QLineEdit(receiveTab);

    // 設置預設值
    receiveIpLineEdit->setText("127.0.0.1");
    receivePortLineEdit->setText("12345");

    // 排版
    QVBoxLayout* receiveLayout = new QVBoxLayout(receiveTab);
    receiveLayout->addWidget(serverProgressBar);
    receiveLayout->addWidget(receiveStatusLabel);

    // 添加 IP 標籤和輸入框
    QHBoxLayout* receiveIpLayout = new QHBoxLayout();
    receiveIpLayout->addWidget(receiveIpLabel);
    receiveIpLayout->addWidget(receiveIpLineEdit);
    receiveLayout->addLayout(receiveIpLayout);

    // 添加 Port 標籤和輸入框
    QHBoxLayout* receivePortLayout = new QHBoxLayout();
    receivePortLayout->addWidget(receivePortLabel);
    receivePortLayout->addWidget(receivePortLineEdit);
    receiveLayout->addLayout(receivePortLayout);

    // 按鈕水平佈局
    QHBoxLayout* receiveButtonLayout = new QHBoxLayout();
    receiveButtonLayout->addWidget(receiveButton);
    receiveButtonLayout->addWidget(quitReceiveButton);
    receiveLayout->addLayout(receiveButtonLayout);

    receiveTab->setLayout(receiveLayout);

    // 添加分頁到 TabWidget
    tabWidget->addTab(sendTab, tr("傳送檔案"));
    tabWidget->addTab(receiveTab, tr("接收檔案"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    // 信號與槽
    connect(openButton, &QPushButton::clicked, this, &TcpFileSender::openFile);
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(tcpClient, &QTcpSocket::bytesWritten, this, &TcpFileSender::updateClientProgress);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
}

TcpFileSender::~TcpFileSender()
{
    if (localFile) {
        localFile->close();
        delete localFile;
    }
}

void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this, tr("選擇文件"));
    if (!fileName.isEmpty()) {
        clientStatusLabel->setText(tr("文件已選擇: %1").arg(fileName));
    }
}

void TcpFileSender::start()
{
    serverIp = ipLineEdit->text();
    serverPort = portLineEdit->text().toUShort();

    if (serverIp.isEmpty() || serverPort == 0) {
        QMessageBox::warning(this, tr("錯誤"), tr("請輸入有效的 IP 和端口"));
        return;
    }

    tcpClient->connectToHost(serverIp, serverPort);

    if (!tcpClient->waitForConnected(3000)) {
        QMessageBox::warning(this, tr("錯誤"), tr("無法連接到服務器"));
        return;
    }

    startTransfer();
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName, this);
    if (!localFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("錯誤"), tr("無法打開文件"));
        return;
    }

    totalBytes = localFile->size();
    bytesToWrite = totalBytes;

    clientStatusLabel->setText(tr("開始發送文件..."));
    outBlock = localFile->read(qMin(bytesToWrite, loadSize));
    bytesWritten += tcpClient->write(outBlock);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += numBytes;
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= tcpClient->write(outBlock);
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);

    if (bytesWritten == totalBytes) {
        clientStatusLabel->setText(tr("文件發送完成!"));
        localFile->close();
    }
}
