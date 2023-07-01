#include "printer.h"
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>


/**
 * @brief Printer::Printer printer constructor
 * @param rotate of true, the photo is rotated of 180° before printed
 * @param watermark if true, the photoBoot logo is added as watermark on the photo
 * @param settingFile path to the file where update the total number of remaining prints
 */
Printer::Printer(bool rotate, bool watermark, QString settingFile) :
    m_thread(nullptr),
    m_rotate(rotate),
    m_watermark(watermark),
    m_settingFile(settingFile)
{
    // Move this object in a thread
    m_thread = new QThread();
    m_thread->start();
    this->moveToThread(m_thread);

    m_logFile = new QFile("printLog.csv");
}


/**
 * @brief Printer::~Printer destructor
 */
Printer::~Printer()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;

    delete m_logFile;
}

/**
 * @brief Printer::print slot to print the photo
 * @param photo path to the photo to print
 * @param nbPrint number of copies to print
 * @param printCounter total number of remaining prints
 */
void Printer::print(QString photo, unsigned int nbPrint, unsigned int printCounter){

    // Prepare Python command
    QString pythonScript = "printer";
    QString pythonFunction = "printer";
    QStringList arguments;
    arguments << photo;
    arguments << QString::number(nbPrint);
    arguments << (m_rotate ? "True" : "False");
    arguments << (m_watermark ? "True" : "False");

    QString command = QString("python -c \"from %1 import %2; %2('%3', %4, %5, %6)\"")
        .arg(pythonScript, pythonFunction, arguments.at(0), arguments.at(1), arguments.at(2), arguments.at(3));

    qDebug() << "PRINTER - Printing" << nbPrint << "copies of" << photo;

    // Execute Python command
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(command);
    process.waitForFinished();

    // Get the output and check it
    QByteArray output = process.readAllStandardOutput();

    if (output != "ok\r\n") {
        qDebug() << "PRINTER - ERROR: " << output;
    }

    // update print counter in settings.ini file
    QSettings settings(m_settingFile, QSettings::IniFormat);
    settings.beginReadArray("printer");
    settings.setValue("counter", printCounter);
    settings.endArray();
    settings.sync();


    if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(m_logFile);

        QDateTime currentDateTime = QDateTime::currentDateTime();
        stream << currentDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz") << ";";
        stream << photo << ";";
        stream << nbPrint << Qt::endl;

        m_logFile->close();
    } else {
        // Gérer l'échec de l'ouverture du fichier
        qDebug() << "PRINTER - ERROR: impossible to open printLog.csv";
    }

}
