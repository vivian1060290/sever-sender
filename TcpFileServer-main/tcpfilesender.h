#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QVBoxLayout>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

private slots:
    void openFile();
    void start();
    void startTransfer();
    void updateClientProgress(qint64 numBytes);

private:
    QTabWidget* tabWidget;
    QWidget* sendTab;
    QWidget* receiveTab;
    // 接收檔案相關變數（新增）
    QLabel* receiveIpLabel;         // 用來顯示接收端的 IP 標籤
    QLineEdit* receiveIpLineEdit;  // 用來輸入接收端 IP
    QLabel* receivePortLabel;       // 用來顯示接收端的 Port 標籤
    QLineEdit* receivePortLineEdit;// 用來輸入接收端的 Port

    // 傳送檔案相關變數
    QTcpSocket* tcpClient;
    QString fileName;
    QFile* localFile;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;
    QByteArray outBlock;
    QProgressBar* clientProgressBar;
    QLabel* clientStatusLabel;
    QPushButton* startButton;
    QPushButton* quitButton;
    QPushButton* openButton;
    QLineEdit* ipLineEdit;
    QLineEdit* portLineEdit;

    // 新增 serverIp 和 serverPort 成員變數
    QString serverIp;
    quint16 serverPort;
    QLabel* ipLabel;
    QLabel* portLabel;
    // 接收檔案相關變數（未實現）
    QLabel* receiveStatusLabel;
    QProgressBar* serverProgressBar;
    QLineEdit* serverIpLineEdit;
    QLineEdit* serverPortLineEdit;
    QPushButton* receiveButton;
    QPushButton* quitReceiveButton;
};

#endif // TCPFILESENDER_H
