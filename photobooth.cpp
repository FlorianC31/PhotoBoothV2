#include "PhotoBooth.h"
#include "ui_PhotoBooth.h"
#include "lib/utils.h"

PhotoBooth::PhotoBooth(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::PhotoBooth),
    m_settings(nullptr)
{
    ui->setupUi(this);

    QObject::connect(ui->veilleButton, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(ui->buttonIncrease, &QPushButton::clicked, [=]() {updateNbPrint(+1);});
    QObject::connect(ui->buttonDecrease, &QPushButton::clicked, [=]() {updateNbPrint(-1);});
    QObject::connect(ui->buttonPrinter, &QPushButton::clicked, this, &PhotoBooth::print);
    QObject::connect(ui->buttonPhoto, &QPushButton::clicked, this, &PhotoBooth::takePhoto);

    if (!readingSettingsFile())
        this->close();

    settingDisplay();

    showCam();
}

PhotoBooth::~PhotoBooth()
{
    delete ui;
}

/**
 * @brief PhotoBooth::readingSettingsFile Read the settings.ini file to get all the user settings
 * @return true is no error
 */
bool PhotoBooth::readingSettingsFile()
{
    // Check and open ini file
    QString settingFile("settings.ini");
    if (!QFile(settingFile).exists()) {
        qDebug() << "Error: The file " << QDir::currentPath() + "/" + settingFile << "doesn't exist.";
        return false;
    }

    // Read printer section
    m_settings->beginReadArray("printer");
    m_printCounter = m_settings->value("count").toUInt();
    m_nbPrintMax = m_settings->value("nbPrintMax").toUInt();
    m_settings->endArray();
    ui->compteur->setText(QString::number(m_printCounter));

    // read relay section
    m_settings->beginReadArray("relay");
    for (const QString &system : {"pcFan", "printerFan", "light"}){
        m_relaysConfig[system] = m_settings->value(system).toUInt();
    }
    m_settings->endArray();

    // read HMI section
    m_settings->beginReadArray("HMI");
    m_font = m_settings->value("font").toString();
    m_fontSizeRatio = m_settings->value("fontSizeRatio").toDouble();

    // read

    if (!transformColor(m_settings->value("backgroundColor").toString(), m_backgroundColor))
        return false;
    m_settings->endArray();

    return true;
}

void PhotoBooth::settingDisplay()
{
    m_settings->beginReadArray("printer");
    m_settings->setValue("count", m_printCounter);
    m_settings->endArray();
    m_settings->sync();
}

void PhotoBooth::showCam()
{
    ui->veilleButton->hide();
    ui->widgetPhoto->hide();
    ui->widgetPrint->hide();

    m_videoFlow.start();

    m_state = SHOWING_CAM;
}


void PhotoBooth::showPhoto()
{
    ui->veilleButton->hide();
    m_state = DISPLAY_PIC;
    updateNbPrint(-m_nbPrintMax);
    ui->widgetPhoto->show();
}


void PhotoBooth::updateNbPrint(int increment)
{
    if (m_state != DISPLAY_PIC)
        return;

    // If no photo left in the printer, the print button are disabled
    if (m_printCounter == 0) {
        ui->nbPrintLabel->setText(QString::number(0));
        btnDisable(ui->buttonIncrease);
        btnDisable(ui->buttonDecrease);
        btnDisable(ui->buttonPrinter);
        return;
    }

    btnActivate(ui->buttonIncrease);
    btnActivate(ui->buttonDecrease);
    btnActivate(ui->buttonPrinter);

    m_nbPrint += increment;
    if (m_nbPrint <= 1) {
        m_nbPrint = 1;
        btnDisable(ui->buttonDecrease);
    }
    else if (m_nbPrint >= m_nbPrintMax)
    {
        m_nbPrint = m_nbPrintMax;
        btnActivate(ui->buttonIncrease);
    }

    ui->nbPrintLabel->setText(QString::number(m_nbPrint));

}

void PhotoBooth::btnActivate(QPushButton* button)
{
    button->setStyleSheet("background-color: transparent;\ncolor: rgb(255, 255, 255);");
}

void PhotoBooth::btnDisable(QPushButton* button)
{
    button->setStyleSheet("background-color: transparent;\ncolor: rgb(127, 127, 127);");
}

void PhotoBooth::print()
{
    m_printer.print(m_nbPrint);
    m_printCounter -= m_nbPrint;
    ui->compteur->setText(QString::number(m_printCounter));

    showCam();
}

void PhotoBooth::takePhoto()
{

}
