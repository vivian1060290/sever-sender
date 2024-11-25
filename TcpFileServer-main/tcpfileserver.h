#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QProgressBar>
#include <QMessageBox>

class TcpFileServer : public QWidget
{
    Q_OBJECT

public:
    explicit TcpFileServer(QWidget *parent = nullptr);
    ~TcpFileServer();

private slots:
    void startServer();                        // 開始服務器
    void acceptConnection();                   // 接受客戶端連接
    void updateServerProgress();               // 更新傳輸進度
    void displayError(QAbstractSocket::SocketError socketError);  // 顯示錯誤信息

private:
    QTcpServer *tcpServer;                     // TCP 伺服器
    QTcpSocket *tcpServerConnection;           // TCP 伺服器連接
    QFile *localFile;                          // 文件對象
    QDataStream in;                            // 資料流
    QProgressBar *progressBar;                 // 進度條
    QString currentFileName;                   // 當前文件名
    qint64 totalBytes;                         // 文件總大小
    qint64 bytesReceived;                      // 已接收的字節數
    qint64 fileNameSize;                       // 文件名大小
    QByteArray outBlock;                       // 傳輸數據塊
};

#endif // TCPFILESERVER_H
