#include "PhotoBooth.h"
#include "ui_PhotoBooth.h"
#include "lib/utils.h"

PhotoBooth::PhotoBooth(QWidget *parent)
    : QWidget(parent),
    m_ui(new Ui::PhotoBooth),
    m_camera(nullptr),
    m_camTrigger(nullptr),
    m_photo(nullptr),
    m_printer(nullptr),
    m_settingFile("settings.ini"),
    m_relay(nullptr),
    m_pcFan(nullptr),
    m_printerFan(nullptr),
    m_light(nullptr)
{
    m_ui->setupUi(this);

    QObject::connect(m_ui->veilleButton, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(m_ui->buttonIncrease, &QPushButton::clicked, this, [=]() {PhotoBooth::updateNbPrint(+1);});
    QObject::connect(m_ui->buttonDecrease, &QPushButton::clicked, this, [=]() {PhotoBooth::updateNbPrint(-1);});
    QObject::connect(m_ui->buttonPrinter, &QPushButton::clicked, this, &PhotoBooth::print);
    QObject::connect(m_ui->buttonPhoto, &QPushButton::clicked, this, &PhotoBooth::takePhoto);
    QObject::connect(m_ui->buttonCancel, &QPushButton::clicked, this, &PhotoBooth::goToSleep);
    QObject::connect(m_ui->buttonRestart, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(m_ui->buttonExit, &QPushButton::clicked, this, &PhotoBooth::showCam);

    if (!readingSettingsFile())
        this->close();

    settingDisplay();
    settingRelayDevices();

    m_camTrigger = new CamTrigger();

    m_camera = new Camera(m_ui->camView, m_cameraDevice, m_fps);
    showCam();
    //showPhoto();
}

PhotoBooth::~PhotoBooth()
{
    delete m_camera;
    delete m_relay;
    delete m_pcFan;
    delete m_printerFan;
    delete m_light;
    delete m_ui;
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
    QSettings settings(m_settingFile, QSettings::IniFormat);

    // Read printer section
    settings.beginReadArray("printer");
    m_printCounter = settings.value("counter").toUInt();
    m_nbPrintMax = settings.value("nbPrintMax").toUInt();
    m_addWatermark = settings.value("addWatermark").toBool();
    settings.endArray();

    // read relay section
    settings.beginReadArray("relay");
    m_relayDevice = settings.value("deviceNumber").toUInt();
    for (const QString &system : QStringList({"pcFan", "printerFan", "light"})){
        m_relaysConfig[system] = settings.value(system).toUInt();
    }
    settings.endArray();

    // read camera section
    settings.beginReadArray("camera");
    m_upsideDown = settings.value("upsideDown").toBool();
    m_cameraDevice = settings.value("deviceNumber").toUInt();
    m_mirror = settings.value("mirror").toBool();
    m_fps = settings.value("fps").toUInt();
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
    m_ui->compteur->setText(QString::number(m_printCounter));
    if (!m_modeDev)
        m_ui->warning->hide();
}

void PhotoBooth::showCam()
{
    m_ui->veilleButton->hide();
    m_ui->widgetPrint->hide();

    m_camera->start();
    m_ui->widgetPhoto->show();

    m_state = SHOWING_CAM;
}

void PhotoBooth::showPhoto()
{
    m_ui->veilleButton->hide();
    m_ui->widgetPhoto->hide();
    m_state = DISPLAY_PIC;
    m_nbPrint = 1;
    updateNbPrint(0);
    m_ui->widgetPrint->show();
}

void PhotoBooth::goToSleep()
{
    m_ui->veilleButton->show();
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
        m_ui->nbPrintLabel->setText(QString::number(0));
        btnDisable(m_ui->buttonIncrease);
        btnDisable(m_ui->buttonDecrease);
        m_ui->buttonPrinter->hide();
        return;
    }

    btnActivate(m_ui->buttonIncrease);
    btnActivate(m_ui->buttonDecrease);
    btnActivate(m_ui->buttonPrinter);

    m_nbPrint += increment;
    if (m_nbPrint <= 1) {
        m_nbPrint = 1;
        btnDisable(m_ui->buttonDecrease);
    }
    else if (m_nbPrint >= fmin(m_nbPrintMax, m_printCounter))
    {
        m_nbPrint = fmin(m_nbPrintMax, m_printCounter);
        btnDisable(m_ui->buttonIncrease);
    }

    m_ui->nbPrintLabel->setText(QString::number(m_nbPrint));

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
    m_printer->print(m_nbPrint);

    // update print counter
    m_printCounter -= m_nbPrint;
    m_ui->compteur->setText(QString::number(m_printCounter));

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

void PhotoBooth::settingRelayDevices()
{
    m_relay = new Relay(m_relayDevice);
    m_pcFan = new RelayDevice(m_relay, m_relaysConfig["pcFan"]);
    m_printerFan = new RelayDevice(m_relay, m_relaysConfig["printerFan"]);
    m_light = new RelayDevice(m_relay, m_relaysConfig["light"]);
}
