#include "photobooth.h"

#include <QApplication>

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);

    QFile file("app.log");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream stream(&file);

        if(msg == " ") {
            stream << Qt::endl;
        }
        else{
            // Time Stamp
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QString timestamp = currentDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
            stream << timestamp << " ";

            // Type prefix
            switch (type)
            {
                case QtDebugMsg:
                    stream << "[DEBUG] ";
                    break;
                case QtWarningMsg:
                    stream << "[WARNING] ";
                    break;
                case QtCriticalMsg:
                    stream << "[CRITICAL] ";
                    break;
                case QtFatalMsg:
                    stream << "[FATAL] ";
                    break;
                case QtInfoMsg:
                    stream << "[INFO] ";
                    break;
            }

            // Message
            stream << msg << Qt::endl;
        }
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifndef QT_DEBUG
    qInstallMessageHandler(messageHandler);
#endif

    PhotoBooth photoBooth;
    photoBooth.show();
    return a.exec();

}
