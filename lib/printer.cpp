#include "printer.h"
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>


Printer::Printer(bool rotate, bool watermark) :
    m_thread(nullptr),
    m_rotate(rotate),
    m_watermark(watermark)
{
    // Move this object in a thread
    m_thread = new QThread();
    m_thread->start();
    this->moveToThread(m_thread);
}


/**
 * @brief Printer::~Printer destructor
 */
Printer::~Printer()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}


void Printer::print(QString photo, unsigned int nbPrint){

    QString pythonScript = "printer";
    QString pythonFunction = "printer";
    QStringList arguments;
    arguments << photo;
    arguments << QString::number(nbPrint);
    arguments << (m_rotate ? "True" : "False");
    arguments << (m_watermark ? "True" : "False");

    QString command = QString("python -c \"from %1 import %2; %2('%3', %4, %5, %6)\"")
        .arg(pythonScript, pythonFunction, arguments.at(0), arguments.at(1), arguments.at(2), arguments.at(3));

    qDebug() << "PRINTER -" << command;

    // Exécuter la commande
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(command);
    process.waitForFinished();

    // Lire la sortie de la commande
    QByteArray output = process.readAllStandardOutput();

    // Traiter la sortie comme souhaité
    qDebug() << "PRINTER - Output: " << output;

}
