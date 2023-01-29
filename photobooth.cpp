#include "PhotoBooth.h"
#include "ui_PhotoBooth.h"
#include "lib/utils.h"

PhotoBooth::PhotoBooth(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::PhotoBooth),
    m_settingFile("settings.ini")
{
    ui->setupUi(this);

    QObject::connect(ui->veilleButton, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(ui->buttonIncrease, &QPushButton::clicked, this, [=]() {PhotoBooth::updateNbPrint(+1);});
    QObject::connect(ui->buttonDecrease, &QPushButton::clicked, this, [=]() {PhotoBooth::updateNbPrint(-1);});
    QObject::connect(ui->buttonPrinter, &QPushButton::clicked, this, &PhotoBooth::print);
    QObject::connect(ui->buttonPhoto, &QPushButton::clicked, this, &PhotoBooth::takePhoto);
    QObject::connect(ui->buttonCancel, &QPushButton::clicked, this, &PhotoBooth::goToSleep);
    QObject::connect(ui->buttonRestart, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(ui->buttonExit, &QPushButton::clicked, this, &PhotoBooth::showCam);

    if (!readingSettingsFile())
        this->close();

    settingDisplay();

    //showCam();
    showPhoto();
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
    if (!QFile(m_settingFile).exists()) {
        qDebug() << "Error: The file " << QDir::currentPath() + "/" + m_settingFile << "doesn't exist.";
        return false;
    }
    qDebug() << "Reading file " << QDir::currentPath() + "/" + m_settingFile;

    // Read printer section
    QSettings settings(m_settingFile, QSettings::IniFormat);
    settings.beginReadArray("printer");
    m_printCounter = settings.value("counter").toUInt();
    m_nbPrintMax = settings.value("nbPrintMax").toUInt();
    settings.endArray();

    // read relay section
    settings.beginReadArray("relay");
    for (const QString &system : {"pcFan", "printerFan", "light"}){
        m_relaysConfig[system] = settings.value(system).toUInt();
    }
    settings.endArray();

    // read camera section
    settings.beginReadArray("camera");
    m_upsideDown = settings.value("upsideDown").toBool();
    settings.endArray();

    // read dev section
    settings.beginReadArray("dev");
    m_modeDev = settings.value("modeDev").toBool();
    settings.endArray();

    // read HMI section
    settings.beginReadArray("HMI");
    m_font = settings.value("font").toString();
    m_fontSizeRatio = settings.value("fontSizeRatio").toDouble();

    if (!transformColor(settings.value("backgroundColor").toString(), m_backgroundColor))
        return false;
    settings.endArray();

    return true;
}

void PhotoBooth::settingDisplay()
{
    ui->compteur->setText(QString::number(m_printCounter));
    if (!m_modeDev)
        ui->warning->hide();
}

void PhotoBooth::showCam()
{
    ui->veilleButton->hide();
    ui->widgetPrint->hide();

    m_videoFlow.start();
    ui->widgetPhoto->show();

    m_state = SHOWING_CAM;
}

void PhotoBooth::showPhoto()
{
    ui->veilleButton->hide();
    ui->widgetPhoto->hide();
    m_state = DISPLAY_PIC;
    m_nbPrint = 1;
    updateNbPrint(0);
    ui->widgetPrint->show();
}

void PhotoBooth::goToSleep()
{
    ui->veilleButton->show();
}

void PhotoBooth::exit()
{
    this->close();
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
        ui->buttonPrinter->hide();
        return;
    }

    qDebug() << "increment: " << increment;

    btnActivate(ui->buttonIncrease);
    btnActivate(ui->buttonDecrease);
    btnActivate(ui->buttonPrinter);

    m_nbPrint += increment;
    if (m_nbPrint <= 1) {
        m_nbPrint = 1;
        btnDisable(ui->buttonDecrease);
    }
    else if (m_nbPrint >= fmin(m_nbPrintMax, m_printCounter))
    {
        m_nbPrint = fmin(m_nbPrintMax, m_printCounter);
        btnDisable(ui->buttonIncrease);
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
    // if no photo left in the printer, no effect on the print button
    if (m_printCounter ==0)
        return;

    // send photo to printer
    m_printer.print(m_nbPrint);

    // update print counter
    m_printCounter -= m_nbPrint;
    ui->compteur->setText(QString::number(m_printCounter));

    // update print counter in settings.ini file
    QSettings settings(m_settingFile, QSettings::IniFormat);
    settings.beginReadArray("printer");
    settings.setValue("counter", m_printCounter);
    settings.endArray();
    settings.sync();

    // go back to showing cam
    showCam();
}

void PhotoBooth::takePhoto()
{
    showPhoto();
}
