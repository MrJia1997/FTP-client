#include "ftp_client.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/FTP_Client/FTP_Client.ico"));
    FTP_Client w;
    w.show();

    return a.exec();
}
