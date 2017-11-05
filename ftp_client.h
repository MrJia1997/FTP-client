#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include "ui_ftp_client.h"

#include <QMainWindow>
#include <QFileIconProvider>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QProgressBar>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QtNetwork>
#include <QRegExp>
#include <QString>
#include <QAction>
#include <QThread>
#include <QTimer>
#include <QTime>
#include <QMenu>

#define BUFFER_SIZE     8192
#define NORMAL          0
#define COMMAND         1
#define RESPONSE        2
#define ERROR           3
#define PASV_MODE       1
#define PORT_MODE       2

class FTP_File
{

public:
    QString file_name;
    int file_size;
    QString file_type;
    QDateTime file_time;
    QString file_auth;
    QString file_owner;
    QString file_group;

public:
    FTP_File();
    FTP_File(QString name, int size, QString type,
        QDateTime time, QString auth, QString owner, QString group);
};

class FTP_Client : public QMainWindow
{
    Q_OBJECT

public:
    explicit FTP_Client(QWidget *parent = 0);
    ~FTP_Client();
public:
    int Login(QTcpSocket *socket, QString user, QString passwd);
    int ConnectPasv();
    int ConnectPort();
    int GetList(QTcpSocket *socket, QString dir = "");
    QString GetWd(QTcpSocket *socket);
    int SetWd(QTcpSocket *socket, QString str);
    QString MakeDirectory(QTcpSocket *socket, QString str);
    int RemoveDirectory(QTcpSocket *socket, QString str);
    int UploadFile(QTcpSocket *socket, QString str);
    int DownloadFile(QTcpSocket *socket, QString name, QString dir, int size);


    int WriteCommandPrompt(QString str, int flag);
    int ParsePasvResponsePort(QString str, QString &ip_addr);
    QString ParsePwdResponsePath(QString str);
    FTP_File ParseLsFormat(QString str);
    int ParseMonthShort(QString str);
    void SetUpModeSwitchButton();
    void SetUpFileTableWidget();
    void SetUpTaskTableWidget();
    void SetFileTableItem(FTP_File file, int row);
    int AddTaskTableItem(QString name, QString path);
    void SetTaskProgress(int row, int progress);
    QIcon GetFileIconByFilename(const QString filename);
    QIcon GetFileIconByExtension(const QString extension);
    QString GetFileTypeByExtension(const QString extension);

public slots:
    int ConnectFTP();
    QString RecvLine(QTcpSocket *socket);
    QStringList RecvAll(QTcpSocket *socket);
    int SendLine(QTcpSocket *socket, QString str);
    int FileDoubleClicked(QTableWidgetItem *item);
    int ShowFileMenu(QPoint p);
    int ShowTaskMenu(QPoint p);
    int RefreshFileList();
    int Delete();
    int AddFolder();
    int Upload();
    int Download();
    int ClearTaskList();
    int Quit();

private:
    Ui::FTP_Client *ui;
    QString server_addr;
    QString client_user, client_passwd;
    QTcpSocket *command_socket = new QTcpSocket(this);
    QTcpSocket *data_socket;
    QTcpServer *data_server;
    int cur_file_row = 0;
    QString cur_status = QString();
    QString cur_mode = "IDLE";
    QList<QProgressBar*> progress_bar;
    QButtonGroup *mode_switch;
};

#endif // FTP_CLIENT_H
