#include "ftp_client.h"
#include "ui_ftp_client.h"

FTP_Client::FTP_Client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FTP_Client)
{
    ui->setupUi(this);

    SetUpModeSwitchButton();
    SetUpFileTableWidget();
    SetUpTaskTableWidget();

    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(ConnectFTP()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(Quit()));
    connect(ui->fileTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(FileDoubleClicked(QTableWidgetItem*)));
}

FTP_Client::~FTP_Client()
{
    delete ui;
}


int FTP_Client::ConnectFTP()
{
    server_addr = "";
    ui->commandTextEdit->clear();
    ui->fileTableWidget->clearContents();

    cur_status = "Welcome";
    QString user, passwd;
    int port;
//    addr = ui->ipLineEdit->text();
//    QString ip_pattern =
//        "^(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|[1-9])\\."
//        "(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)\\."
//        "(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)\\."
//        "(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]\\d|\\d)$";
//    QRegExp ip_reg(ip_pattern);
//    if (ip_reg.exactMatch(addr))
//        server_ip = addr;

    server_addr = ui->ipLineEdit->text();
    if(server_addr.isEmpty())
        QMessageBox::warning(NULL, "Warning", "You must input an address in IP/Domain.", QMessageBox::Ok);

    port = ui->portLineEdit->text().isEmpty() ? 21 : ui->portLineEdit->text().toInt();
    if (port < 1 || port > 65535)
        QMessageBox::warning(NULL, "Warning", "The port number is out of range. [1~65535]", QMessageBox::Ok);

    user = ui->userLineEdit->text().isEmpty() ? "anonymous" : ui->userLineEdit->text();
    client_user = user;
    passwd = ui->passwdLineEdit->text().isEmpty() ? "anonymous@" : ui->passwdLineEdit->text();
    client_passwd = passwd;

    if (command_socket->isOpen())
        command_socket->close();

    WriteCommandPrompt("Trying to connect to " + server_addr + ":" + QString::number(port) + "...", NORMAL);

    command_socket->connectToHost(server_addr, port);
    if (!command_socket->waitForConnected(500))
    {
        WriteCommandPrompt(QString("Command Connect failed, error: ") + command_socket->errorString(), ERROR);
        command_socket->close();
        return -1;
    }

    WriteCommandPrompt(QString("Connect established."), NORMAL);

    QString res = RecvLine(command_socket);
    WriteCommandPrompt(res, RESPONSE);
    if (!res.startsWith("220"))
    {
        WriteCommandPrompt(QString("Rejected by server."), ERROR);
        return -1;
    }

    if (Login(command_socket, user, passwd) == -1)
        return -1;

    return 0;
}

int FTP_Client::Login(QTcpSocket *socket, QString user, QString passwd)
{
    SendLine(socket, QString("USER ") + user);
    cur_status = "USER";
    QString res_user = RecvLine(socket);
    WriteCommandPrompt(res_user, RESPONSE);

    if (res_user.startsWith("2"))
    {
        WriteCommandPrompt(QString("Successfully log in."), NORMAL);
        return 0;
    }
    else if (res_user.startsWith("4") || res_user.startsWith("5"))
    {
        WriteCommandPrompt(QString("User ") + user + QString(" rejected by server."), ERROR);
        return -1;
    }

    SendLine(socket, QString("PASS ") + passwd);
    cur_status = "PASS";
    QStringList res_pass = RecvAll(socket);
    int i, len = res_pass.size();
    for (i = 0; i < len; i++)
        WriteCommandPrompt(res_pass[i], RESPONSE);

    if (res_pass[i - 1].startsWith("2"))
    {
        WriteCommandPrompt(QString("Successfully log in."), NORMAL);
    }
    else
    {
        WriteCommandPrompt(QString("User and password don't match."), ERROR);
        return -1;
    }

    SendLine(socket, QString("TYPE I"));
    cur_status = "TYPE";
    WriteCommandPrompt(RecvLine(socket), RESPONSE);

    SendLine(socket, QString("SYST"));
    cur_status = "SYST";
    WriteCommandPrompt(RecvLine(socket), RESPONSE);

    GetList(socket);

    return 0;
}

int FTP_Client::ConnectPasv()
{
    cur_mode = "PASV";
    SendLine(command_socket, QString("PASV"));

    QString res_pasv = RecvLine(command_socket);
    WriteCommandPrompt(res_pasv, RESPONSE);

    if (!res_pasv.startsWith("227"))
    {
        WriteCommandPrompt(QString("Passive mode is not supported by server."), ERROR);
        return -1;
    }

    QString ip_addr;
    int port = ParsePasvResponsePort(res_pasv, ip_addr);

    data_socket = new QTcpSocket(this);
    data_socket->connectToHost(ip_addr, port);
    if (!data_socket->waitForConnected(1000))
    {
        WriteCommandPrompt(QString("Connect pasv response address failed, error: ") + data_socket->errorString(), ERROR);
        WriteCommandPrompt(QString("Try to connect server address"), NORMAL);
        data_socket->connectToHost(server_addr, port);
        if (!data_socket->waitForConnected(1000))
        {
            WriteCommandPrompt(QString("Connect server address failed, error: ") + data_socket->errorString(), ERROR);
            data_socket->close();
            delete data_socket;
            return -1;
        }
    }

    return 0;
}

int FTP_Client::ConnectPort()
{
    cur_mode = "PORT";
    QHostAddress addr = command_socket->localAddress();
    if(addr == QHostAddress::Null)
    {
        WriteCommandPrompt(QString("Port mode failed on client."), ERROR);
        return -1;
    }
    QString addr_str = addr.toString();
    qsrand(time(NULL));
    int port = qrand() % 15000 + 20000;
    int port1 = port >> 8;
    int port2 = port & 0xFF;
    addr_str.replace(".", ",");
    addr_str = addr_str + "," + QString::number(port1) + "," + QString::number(port2);

    data_server = new QTcpServer(this);
    data_server->listen(QHostAddress::Any, port);

    SendLine(command_socket, QString("PORT ") + addr_str);

    QString res_port = RecvLine(command_socket);
    WriteCommandPrompt(res_port, RESPONSE);

    if (!res_port.startsWith("2"))
    {
        WriteCommandPrompt(QString("Port mode is not supported by server."), ERROR);
        data_server->close();
        delete data_server;
        return -1;
    }
    return 0;
}

int FTP_Client::GetList(QTcpSocket * socket, QString dir)
{
    QString temp_dir = QString();
    if (dir.isEmpty())
    {
        temp_dir = GetWd(socket);
        if (temp_dir.isEmpty())
            return -1;
    }
    else
    {
        temp_dir = dir;
    }

    // Try Connect
    if(mode_switch->checkedId() == PASV_MODE)
    {
        if(ConnectPasv() == -1)
        {
            return -1;
        }
    }
    else if(mode_switch->checkedId() == PORT_MODE)
    {
        if(ConnectPort() == -1)
        {
            return -1;
        }
    }
    else
        return -1;

    SendLine(command_socket, QString("LIST ") + temp_dir);
    cur_status = "LIST";

    QString res_list = RecvLine(command_socket);
    WriteCommandPrompt(res_list, RESPONSE);
    if (!res_list.startsWith("150"))
    {
        WriteCommandPrompt(QString("Get directory list failed."), ERROR);
        if (cur_mode == "PASV")
        {
            data_socket->disconnectFromHost();
            data_socket->close();
            delete data_socket;
        }
        else
        {
            delete data_server;
        }
        return -1;
    }

    // Try Connect
    if (cur_mode == "PORT")
    {
        data_server->waitForNewConnection();
        data_socket = data_server->nextPendingConnection();
    }

    QStringList dir_list = RecvAll(data_socket);
    int len = dir_list.size();
    ui->fileTableWidget->clearContents();
    ui->fileTableWidget->setRowCount(len + 1);
    ui->fileTableWidget->setItem(0, 0, new QTableWidgetItem(GetFileIconByExtension("folder"), QString("..")));
    ui->fileTableWidget->setItem(0, 1, new QTableWidgetItem(QString("folder")));

    for (int i = 0; i < len; i++)
    {
        FTP_File file = ParseLsFormat(dir_list.at(i));
        SetFileTableItem(file, i + 1);
    }


    // Close connect
    data_socket->flush();
    data_socket->disconnectFromHost();
    data_socket->close();

    if (cur_mode == "PORT")
        delete data_server;

    res_list = RecvLine(command_socket);
    WriteCommandPrompt(res_list, RESPONSE);
    if (!res_list.startsWith("2"))
    {
        WriteCommandPrompt(QString("Get directory list failed."), ERROR);
        return -1;
    }

    return 0;
}

QString FTP_Client::GetWd(QTcpSocket *socket)
{
    SendLine(socket, QString("PWD"));
    cur_status = "PWD";
    QString res_pwd = RecvLine(socket);
    WriteCommandPrompt(res_pwd, RESPONSE);
    if (!res_pwd.startsWith("257"))
    {
        WriteCommandPrompt(QString("Get working directory failed."), ERROR);
        return QString();
    }

    QString path = ParsePwdResponsePath(res_pwd);
    if (!path.isEmpty())
    {
        WriteCommandPrompt(QString("Current working directory is ") + path, NORMAL);
        return path;
    }
    else
    {
        WriteCommandPrompt(QString("Get working directory failed."), ERROR);
        return QString();
    }
}

int FTP_Client::SetWd(QTcpSocket *socket, QString str)
{
    SendLine(socket, QString("CWD ") + str);
    cur_status = "CWD";
    QString res_cwd = RecvLine(socket);
    WriteCommandPrompt(res_cwd, RESPONSE);
    if (!res_cwd.startsWith("2"))
    {
        WriteCommandPrompt(QString("Set working directory failed."), ERROR);
        return -1;
    }
    WriteCommandPrompt(QString("Set working directory successfully."), NORMAL);
    return 0;
}

QString FTP_Client::MakeDirectory(QTcpSocket * socket, QString str)
{
    SendLine(socket, QString("MKD ") + str);
    cur_status = "MKD";
    QString res_mkd = RecvLine(socket);
    WriteCommandPrompt(res_mkd, RESPONSE);
    if (!res_mkd.startsWith("2"))
    {
        WriteCommandPrompt(QString("Make directory failed."), ERROR);
        return QString();
    }
    QString path = ParsePwdResponsePath(res_mkd);
    if (!path.isEmpty())
    {
        WriteCommandPrompt(QString("Make directory is ") + path, NORMAL);
        return path;
    }
    else
    {
        WriteCommandPrompt(QString("Get make directory failed."), ERROR);
        return QString();
    }
}

int FTP_Client::RemoveDirectory(QTcpSocket * socket, QString str)
{
    SendLine(socket, QString("RMD ") + str);
    cur_status = "RMD";
    QString res_rmd = RecvLine(socket);
    WriteCommandPrompt(res_rmd, RESPONSE);
    if (!res_rmd.startsWith("2"))
    {
        WriteCommandPrompt(QString("Remove directory failed."), ERROR);
        return -1;
    }
    WriteCommandPrompt(QString("Remove directory successfully."), NORMAL);
    return 0;
}

int FTP_Client::UploadFile(QTcpSocket * socket, QString str)
{

    QString path = str;
    QStringList list = str.split("/");
    int len = list.size();
    QString filename = list[len - 1];
    QFile *temp_file = new QFile(path);
    temp_file->open(QFile::ReadOnly);
    WriteCommandPrompt(QString("Open file ") + str, NORMAL);

    // Try Connect
    if(mode_switch->checkedId() == PASV_MODE)
    {
        if(ConnectPasv() == -1)
        {
            return -1;
        }
    }
    else if(mode_switch->checkedId() == PORT_MODE)
    {
        if(ConnectPort() == -1)
        {
            return -1;
        }
    }
    else
        return -1;

    cur_status = "STOR";
    SendLine(command_socket, QString("STOR ") + filename);
    QString res_stor = RecvLine(socket);
    WriteCommandPrompt(res_stor, RESPONSE);
    if (!res_stor.startsWith("150"))
    {
        WriteCommandPrompt(QString("Upload file failed."), ERROR);
        data_socket->disconnectFromHost();
        data_socket->close();
        delete data_socket;
        return -1;
    }
    command_socket->flush();

    // Try Connect
    if (cur_mode == "PORT")
    {
        data_server->waitForNewConnection();
        data_socket = data_server->nextPendingConnection();
    }

    int task_row = AddTaskTableItem(filename, path);

    qint64 total_bytes = temp_file->size();
    qint64 bytes_to_write = total_bytes;
    QByteArray out_block;
    QDataStream out(&out_block, QIODevice::WriteOnly);
    while (bytes_to_write > 0)
    {
        data_socket->waitForBytesWritten();
        out_block = temp_file->read(qMin(bytes_to_write, (qint64)BUFFER_SIZE));
        data_socket->write(out_block);
        bytes_to_write -= out_block.size();
        int progress = (100 * (total_bytes - bytes_to_write)) / total_bytes;
        SetTaskProgress(task_row, progress);
    }

    // Close connect
    data_socket->flush();
    data_socket->disconnectFromHost();
    data_socket->close();
    if (cur_mode == "PORT")
        delete data_server;

    temp_file->close();

    WriteCommandPrompt(QString("Upload file finished, waiting for response..."), NORMAL);

    res_stor = RecvLine(socket);
    WriteCommandPrompt(res_stor, RESPONSE);
    if (!res_stor.startsWith("2"))
    {
        WriteCommandPrompt(QString("Upload file failed."), ERROR);
        return -1;
    }
    WriteCommandPrompt(QString("Upload file successfully."), NORMAL);

    return 0;
}

int FTP_Client::DownloadFile(QTcpSocket * socket, QString name, QString dir, int size)
{
    QString full_path = dir + "/" + name;
    QFile temp_file(full_path);
    temp_file.open(QFile::WriteOnly);
    temp_file.close();
    temp_file.open(QFile::ReadWrite);

    WriteCommandPrompt(QString("Create file ") + full_path, NORMAL);

    // Try Connect
    if(mode_switch->checkedId() == PASV_MODE)
    {
        if(ConnectPasv() == -1)
        {
            return -1;
        }
    }
    else if(mode_switch->checkedId() == PORT_MODE)
    {
        if(ConnectPort() == -1)
        {
            return -1;
        }
    }
    else
        return -1;

    cur_status = "RETR";
    SendLine(command_socket, QString("RETR ") + name);
    QString res_retr = RecvLine(socket);
    WriteCommandPrompt(res_retr, RESPONSE);
    if (!res_retr.startsWith("150"))
    {
        WriteCommandPrompt(QString("Download file failed."), ERROR);
        data_socket->disconnectFromHost();
        data_socket->close();
        delete data_socket;
        return -1;
    }
    command_socket->flush();

    // Try Connect
    if (cur_mode == "PORT")
    {
        data_server->waitForNewConnection();
        data_socket = data_server->nextPendingConnection();
    }

    int task_row = AddTaskTableItem(name, full_path);

    qint64 total_bytes = size;
    qint64 bytes_to_write = total_bytes;
    QByteArray in_block;
    QDataStream in(&in_block, QIODevice::WriteOnly);

    while (bytes_to_write > 0)
    {
        data_socket->waitForReadyRead();
        in_block = data_socket->readAll();
        temp_file.write(in_block);
        bytes_to_write -= in_block.size();
        int progress = (100 * (total_bytes - bytes_to_write)) / total_bytes;
        SetTaskProgress(task_row, progress);
    }

    // Close connect
    data_socket->flush();
    data_socket->disconnectFromHost();
    data_socket->close();
    if (cur_mode == "PORT")
        delete data_server;

    temp_file.close();

    WriteCommandPrompt(QString("Download file finished."), NORMAL);


    res_retr = RecvLine(socket);
    WriteCommandPrompt(res_retr, RESPONSE);
    if (!res_retr.startsWith("2"))
    {
        WriteCommandPrompt(QString("Download file failed."), ERROR);
        return -1;
    }
    WriteCommandPrompt(QString("Download file successfully."), NORMAL);
    return 0;


}

QString FTP_Client::RecvLine(QTcpSocket *socket)
{
    if (!socket->isOpen())
    {
        qDebug() << "Error RecvLine(): socket isn't open." << endl;
        return QString();
    }

    char buffer[BUFFER_SIZE];
    socket->waitForReadyRead();
    socket->readLine(buffer, BUFFER_SIZE);
    socket->read(socket->bytesAvailable());
    return QString(buffer);
}

QStringList FTP_Client::RecvAll(QTcpSocket * socket)
{
    if (!socket->isOpen())
    {
        qDebug() << "Error RecvAll(): socket isn't open." << endl;
        return QStringList();
    }

    QStringList list = QStringList();
    QByteArray byte_array = QByteArray();
    socket->waitForReadyRead();

    while (1) {

        byte_array = socket->readAll();
        QString str(byte_array);
        if (str == "")
            break;
        QStringList str_list = str.split(QRegExp("\\r\\n"));
        str_list.removeLast();
        list.append(str_list);
    }

    return list;
}

int FTP_Client::SendLine(QTcpSocket *socket, QString str)
{
    if (!socket->isOpen())
    {
        qDebug() << "Error SendLine(): socket isn't open." << endl;
        return -1;
    }

    WriteCommandPrompt(str, COMMAND);

    socket->waitForBytesWritten();
    str.append("\r\n");
    socket->write(str.toStdString().c_str(), str.size());

    return 0;
}

int FTP_Client::FileDoubleClicked(QTableWidgetItem * item)
{
    int row = item->row();
    QString name = ui->fileTableWidget->item(row, 0)->text();
    QString type = ui->fileTableWidget->item(row, 1)->text();
    if (type == "folder")
    {
        if (name == ".." && GetWd(command_socket) == "/")
            return 0;
        SetWd(command_socket, name);
        GetList(command_socket);
    }
    else
        QMessageBox::information(NULL, "Info", "Double click file hasn't been implemented.", QMessageBox::Ok);

    return 0;
}

int FTP_Client::ShowFileMenu(QPoint p)
{
    QModelIndex index = ui->fileTableWidget->indexAt(p);
    cur_file_row = index.row();

    QMenu *menu = new QMenu(ui->fileTableWidget);
    QAction *act_refresh = new QAction("Refresh", ui->fileTableWidget);
    QAction *act_delete = new QAction("Delete", ui->fileTableWidget);
    QAction *act_create = new QAction("Create New Folder", ui->fileTableWidget);
    QAction *act_upload = new QAction("Upload", ui->fileTableWidget);
    QAction *act_download = new QAction("Download", ui->fileTableWidget);

    connect(act_refresh, SIGNAL(triggered()), this, SLOT(RefreshFileList()));
    connect(act_delete, SIGNAL(triggered()), this, SLOT(Delete()));
    connect(act_create, SIGNAL(triggered()), this, SLOT(AddFolder()));
    connect(act_upload, SIGNAL(triggered()), this, SLOT(Upload()));
    connect(act_download, SIGNAL(triggered()), this, SLOT(Download()));

    menu->addAction(act_refresh);
    menu->addAction(act_delete);
    menu->addAction(act_create);
    menu->addAction(act_upload);
    menu->addAction(act_download);

    menu->move(cursor().pos());
    menu->show();

    return 0;
}

int FTP_Client::ShowTaskMenu(QPoint p)
{
    QModelIndex index = ui->taskTableWidget->indexAt(p);
    cur_file_row = index.row();

    QMenu *menu = new QMenu(ui->taskTableWidget);
    QAction *act_clear = new QAction("Clear All", ui->taskTableWidget);

    connect(act_clear, SIGNAL(triggered()), this, SLOT(ClearTaskList()));

    menu->addAction(act_clear);

    menu->move(cursor().pos());
    menu->show();

    return 0;
}

int FTP_Client::RefreshFileList()
{
    GetList(command_socket);
    return 0;
}

int FTP_Client::Delete()
{
    QString name = ui->fileTableWidget->item(cur_file_row, 0)->text();
    QString type = ui->fileTableWidget->item(cur_file_row, 1)->text();

    if (type == "folder")
    {
        if (name == ".." || name == ".")
        {
            QMessageBox::warning(NULL, "Warning", "This folder cannot be deleted.", QMessageBox::Ok);
            return -1;
        }
        RemoveDirectory(command_socket, name);
        GetList(command_socket);
    }
    else
        QMessageBox::information(NULL, "Info", "Delete file hasn't been implemented.", QMessageBox::Ok);

    return 0;
}

int FTP_Client::AddFolder()
{
    bool isOk;
    QString name = QInputDialog::getText(NULL, "Create new Folder", "Please input new folder name", QLineEdit::Normal, "new folder", &isOk);
    if (isOk)
    {
        MakeDirectory(command_socket, name);
        GetList(command_socket);
    }
    return 0;
}

int FTP_Client::Upload()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Upload File"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        tr("All Files(*.*)")
    );

    if (!fileName.isEmpty())
        UploadFile(command_socket, fileName);

    return 0;
}

int FTP_Client::Download()
{
    QString name = ui->fileTableWidget->item(cur_file_row, 0)->text();
    QString type = ui->fileTableWidget->item(cur_file_row, 1)->text();
    int size = ui->fileTableWidget->item(cur_file_row, 2)->text().toInt();
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Choose Directory"),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (type != "folder")
    {
        DownloadFile(command_socket, name, dir, size);
    }
    else
        QMessageBox::information(NULL, "Info", "Download folder hasn't been implemented.", QMessageBox::Ok);

    return 0;
    return 0;
}

int FTP_Client::ClearTaskList()
{
    ui->taskTableWidget->clearContents();
    progress_bar.clear();
    return 0;
}

int FTP_Client::Quit()
{
    SendLine(command_socket, QString("QUIT"));

    QString res_quit = RecvLine(command_socket);
    WriteCommandPrompt(res_quit, RESPONSE);
    if (!res_quit.startsWith("2"))
    {
        WriteCommandPrompt(QString("Disconnect failed."), ERROR);
        return -1;
    }
    command_socket->disconnectFromHost();
    command_socket->abort();
    command_socket->close();
    WriteCommandPrompt(QString("Disconnect from host successfully."), NORMAL);

    return 0;
}

int FTP_Client::WriteCommandPrompt(QString str, int flag)
{
    QString colors[4] = { "#000000", "#865FC5", "#006400", "#FF2525" };
    QString html_content = "<span style='color:" + colors[flag] + ";'>" + str + "</span><br />";
    ui->commandTextEdit->insertHtml(html_content);

    QScrollBar *scrollbar = ui->commandTextEdit->verticalScrollBar();
    if (scrollbar)
        scrollbar->setSliderPosition(scrollbar->maximum());

    QApplication::processEvents();
    return 0;
}

int FTP_Client::ParsePasvResponsePort(QString str, QString &ip_addr)
{
    QByteArray byte_array = str.toLatin1();
    char *char_str = byte_array.data();
    int len = str.length();
    int ip[4] = {0, 0, 0, 0};
    int port[2] = {0, 0};
    int space_flag = 0, digit_flag = 0, comma_flag = 0;
    for (int i = 0; i < len; i++)
    {
        if (char_str[i] == ' ')
        {
            space_flag = 1;
        }
        else if (char_str[i] == ',')
        {
            if (digit_flag)
                comma_flag++;
        }
        else if (char_str[i] >= '0' && char_str[i] <= '9')
        {
            if (space_flag)
            {

                if(comma_flag <= 3)
                    ip[comma_flag] = ip[comma_flag] * 10 + char_str[i] - '0';
                else if (comma_flag <= 5)
                    port[comma_flag - 4] = port[comma_flag - 4] * 10 + char_str[i] - '0';

                digit_flag = 1;
            }
        }
    }

    ip_addr = QString::number(ip[0]) + "." + QString::number(ip[1]) + "." + QString::number(ip[2]) + "." + QString::number(ip[3]);

    return ((port[0] << 8) + (port[1]));
}

QString FTP_Client::ParsePwdResponsePath(QString str)
{
    int len = str.length();
    int start = 0, end = 0;
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '"')
        {
            if (!start)
                start = i;
            else if (!end)
                end = i;
            else
                break;
        }
    }
    if (start && end)
        return str.mid(start + 1, end - start - 1);
    else
        return QString();
}

FTP_File FTP_Client::ParseLsFormat(QString str)
{
    // /bin/ls format
    // -rw-r--r-- 1 owner group           213 Aug 26 16:31 README
    // -rw-r--r-- 1 owner group          1383 Apr 10  1997 ip.c
    QStringList info_list = str.split(QRegExp("\\s+"));
    QString auth = info_list[0];
    QString owner = info_list[2];
    QString group = info_list[3];
    int size = info_list[4].toInt();

    QDate date = QDate::currentDate();
    QTime time = QTime(0, 0);
    if (info_list[7].contains(":"))
    {
        int y = date.year();
        int m = ParseMonthShort(info_list[5]);
        int d = info_list[6].toInt();
        date.setDate(y, m, d);
        QStringList t = info_list[7].split(':');
        time.setHMS(t[0].toInt(), t[1].toInt(), 0);
    }
    else
    {
        int y = info_list[7].toInt();
        int m = ParseMonthShort(info_list[5]);
        int d = info_list[6].toInt();
        date.setDate(y, m, d);
    }
    QDateTime datetime = QDateTime(date, time);

    QString mark = info_list[5];
    int index = str.indexOf(mark);
    int len = str.length();
    int space_flag = 0, space_count = 0;
    int i;
    for (i = index; i < len; i++)
    {
        if (str[i] == ' ' && !space_flag)
        {
            space_flag = 1;
            space_count++;
        }
        else
        {
            space_flag = 0;
            if (space_count == 3)
                break;
        }
    }
    QString name = str.right(len - i).replace(QRegExp("[\\r\\n]"), "");

    QString type = QString();
    if (auth[0] == 'd')
    {
        type = "folder";
    }
    else
    {
        if (name.contains("."))
        {
            QStringList f = name.split(".");
            int s = f.size();
            type = f[s - 1];
        }
        else
        {
            type = "file";
        }

    }

    return FTP_File(name, size, type, datetime, auth, owner, group);
}

int FTP_Client::ParseMonthShort(QString str)
{
    if (str == "Jan")
        return 1;
    else if (str == "Feb")
        return 2;
    else if (str == "Mar")
        return 3;
    else if (str == "Apr")
        return 4;
    else if (str == "May")
        return 5;
    else if (str == "Jun")
        return 6;
    else if (str == "Jul")
        return 7;
    else if (str == "Aug")
        return 8;
    else if (str == "Sep")
        return 9;
    else if (str == "Oct")
        return 10;
    else if (str == "Nov")
        return 11;
    else if (str == "Dec")
        return 12;
    else
        return 0;
}

void FTP_Client::SetUpModeSwitchButton()
{
    mode_switch = new QButtonGroup(this);
    mode_switch->addButton(ui->pasvRadioButton, PASV_MODE);
    mode_switch->addButton(ui->portRadioButton, PORT_MODE);

    ui->pasvRadioButton->setChecked(true);
}

void FTP_Client::SetUpFileTableWidget()
{
    ui->fileTableWidget->setColumnCount(6);
    ui->fileTableWidget->horizontalHeader()->setDefaultSectionSize(150);
    ui->fileTableWidget->horizontalHeader()->setSectionsClickable(false);
    ui->fileTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fileTableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowFileMenu(QPoint)));

    QStringList headers;
    headers << tr("File Name") << tr("Type") << tr("Size") << tr("Time") << tr("Authority") << tr("Owner/Group");
    ui->fileTableWidget->setHorizontalHeaderLabels(headers);
    QFont font = ui->fileTableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->fileTableWidget->horizontalHeader()->setFont(font);
    ui->fileTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->fileTableWidget->verticalHeader()->setDefaultSectionSize(20);
    ui->fileTableWidget->setFrameShape(QFrame::NoFrame);
    ui->fileTableWidget->setShowGrid(false);
    ui->fileTableWidget->verticalHeader()->setVisible(false);
    ui->fileTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->fileTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->fileTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void FTP_Client::SetUpTaskTableWidget()
{
    ui->taskTableWidget->setColumnCount(3);
    ui->taskTableWidget->horizontalHeader()->setDefaultSectionSize(150);
    ui->taskTableWidget->horizontalHeader()->setSectionsClickable(false);
    ui->taskTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->taskTableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowTaskMenu(QPoint)));

    QStringList headers;
    headers << tr("File Name") << tr("Path") << tr("Progress");
    ui->taskTableWidget->setHorizontalHeaderLabels(headers);
    QFont font = ui->taskTableWidget->horizontalHeader()->font();
    font.setBold(true);
    ui->taskTableWidget->horizontalHeader()->setFont(font);
    ui->taskTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->taskTableWidget->verticalHeader()->setDefaultSectionSize(20);
    ui->taskTableWidget->setFrameShape(QFrame::NoFrame);
    ui->taskTableWidget->setShowGrid(false);
    ui->taskTableWidget->verticalHeader()->setVisible(false);
    ui->taskTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->taskTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->taskTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void FTP_Client::SetFileTableItem(FTP_File file, int row)
{
    ui->fileTableWidget->setItem(row, 0, new QTableWidgetItem(GetFileIconByExtension(file.file_type), file.file_name));
    ui->fileTableWidget->setItem(row, 1, new QTableWidgetItem(GetFileTypeByExtension(file.file_type)));
    if (file.file_type == "folder")
        ui->fileTableWidget->setItem(row, 2, new QTableWidgetItem(QString("")));
    else
        ui->fileTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(file.file_size)));
    ui->fileTableWidget->setItem(row, 3, new QTableWidgetItem(file.file_time.toString("yyyy-MM-dd hh:mm:ss")));
    ui->fileTableWidget->setItem(row, 4, new QTableWidgetItem(file.file_auth));
    ui->fileTableWidget->setItem(row, 5, new QTableWidgetItem(file.file_owner + QString("/") + file.file_group));
}

int FTP_Client::AddTaskTableItem(QString name, QString path)
{
    int row = ui->taskTableWidget->rowCount();
    ui->taskTableWidget->setRowCount(row + 1);
    ui->taskTableWidget->setItem(row, 0, new QTableWidgetItem(name));
    ui->taskTableWidget->setItem(row, 1, new QTableWidgetItem(path));
    QProgressBar *bar = new QProgressBar(ui->taskTableWidget);
    bar->setTextVisible(true);
    bar->setMinimum(0);
    bar->setValue(0);
    bar->setMaximum(100);
    ui->taskTableWidget->setCellWidget(row, 2, bar);
    progress_bar.append(bar);

    QApplication::processEvents();
    return row;
}

void FTP_Client::SetTaskProgress(int row, int progress)
{
    progress_bar[row]->setValue(progress);
    QApplication::processEvents();
}

QIcon FTP_Client::GetFileIconByFilename(const QString filename)
{
    // https://stackoverflow.com/questions/4617981/how-to-get-qt-icon-qicon-given-a-file-extension

    QMimeDatabase mime_database;
    QIcon icon;
    QList<QMimeType> mime_types = mime_database.mimeTypesForFileName(filename);
    for (int i = 0; i < mime_types.count() && icon.isNull();i++)
        icon = QIcon::fromTheme(mime_types[i].iconName());

    if (icon.isNull())
        return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    else
        return icon;
}

QIcon FTP_Client::GetFileIconByExtension(const QString extension)
{
    // http://blog.csdn.net/liang19890820/article/details/51821622

    QFileIconProvider provider;
    QIcon icon;
    if (extension == "folder")
    {
        icon = provider.icon(QFileIconProvider::Folder);
    }
    else
    {
        QString strTemplateName = QDir::tempPath() + QDir::separator() +
            QCoreApplication::applicationName() + "_XXXXXX." + extension;
        QTemporaryFile tmpFile(strTemplateName);
        tmpFile.setAutoRemove(false);
        if (tmpFile.open())
        {
            tmpFile.write(QByteArray());
            tmpFile.close();
            icon = provider.icon(QFileInfo(tmpFile.fileName()));
            tmpFile.remove();
        }
        else
        {
            qCritical() << QString("failed to write temporary file %1").arg(tmpFile.fileName());
        }
    }

    return icon;
}

QString FTP_Client::GetFileTypeByExtension(const QString extension)
{
    // http://blog.csdn.net/liang19890820/article/details/51821622

    QFileIconProvider provider;
    QString strType;
    if (extension == "folder" || extension == "file")
    {
        strType = extension;
    }
    else
    {
        QString strFileName = QDir::tempPath() + QDir::separator() +
            QCoreApplication::applicationName() + "_XXXXXX." + extension;
        QTemporaryFile tmpFile(strFileName);

        if (tmpFile.open())
        {
            tmpFile.close();
            strType = provider.type(QFileInfo(tmpFile.fileName()));
        }
        else
        {
            qCritical() << QString("failed to write temporary file %1").arg(tmpFile.fileName());
        }

    }

    return strType;
}

FTP_File::FTP_File()
{
    file_name = QString();
    file_size = 0;
    file_type = QString();
    file_time = QDateTime();
    file_auth = QString();
    file_owner = QString();
    file_group = QString();
}

FTP_File::FTP_File(QString name, int size, QString type, QDateTime time, QString auth, QString owner, QString group)
{
    file_name = name;
    file_size = size;
    file_type = type;
    file_time = time;
    file_auth = auth;
    file_owner = owner;
    file_group = group;
}
