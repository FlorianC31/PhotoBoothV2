#include "PhotoBooth.h"
#include "ui_PhotoBooth.h"
#include "lib/utils.h"

#define CHECK_REMOTE_PERIOD 1000 //ms

PhotoBooth::PhotoBooth(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::PhotoBooth),
    m_camera(nullptr),
    m_camTrigger(nullptr),
    m_photo(nullptr),
    m_printer(nullptr),
    m_settingFile("settings.ini"),
    m_countDownTimer(nullptr),
    m_sleepTimer(nullptr),
    m_cameraTimer(nullptr),
    m_lightOn(false),
    m_relay(nullptr),
    m_pcFan(nullptr),
    m_printerFan(nullptr),
    m_light(nullptr)
{
    m_ui->setupUi(this);

    // Connect all buttons
    QObject::connect(m_ui->veilleButton, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(m_ui->buttonIncrease, &QPushButton::clicked, this, [=]() {PhotoBooth::updateNbPrint(+1);});
    QObject::connect(m_ui->buttonDecrease, &QPushButton::clicked, this, [=]() {PhotoBooth::updateNbPrint(-1);});
    QObject::connect(m_ui->buttonPrinter, &QPushButton::clicked, this, &PhotoBooth::print);
    QObject::connect(m_ui->buttonPhoto, &QPushButton::clicked, this, &PhotoBooth::takePhoto);
    QObject::connect(m_ui->buttonCancel, &QPushButton::clicked, this, &PhotoBooth::goToSleep);
    QObject::connect(m_ui->buttonRestart, &QPushButton::clicked, this, &PhotoBooth::showCam);
    QObject::connect(m_ui->buttonExit, &QPushButton::clicked, this, &PhotoBooth::exit);

    // Get the loading gif
    m_movie = new QMovie(QString::fromUtf8("ressources/Spinner-1s-400px_white.gif"));
    m_ui->loading->setMovie(m_movie);
    m_movie->start();

    // Read the settings file
    if (!readingSettingsFile()) {
        this->close();
    }

    // Setting display
    settingDisplay();

    // Creation of all timers
    m_countDownTimer = new QTimer(this);
    m_sleepTimer = new QTimer(this);
    m_remoteTimer = new QTimer(this);
    m_cameraTimer = new QTimer(this);
    connect(m_countDownTimer, &QTimer::timeout, this, &PhotoBooth::countDown);
    connect(m_sleepTimer, &QTimer::timeout, this, &PhotoBooth::goToSleep);
    connect(m_remoteTimer, &QTimer::timeout, this, &PhotoBooth::checkRemote);
    connect(m_cameraTimer, &QTimer::timeout, this, &PhotoBooth::CameraLoop);
    m_remoteTimer->start(CHECK_REMOTE_PERIOD);

    // Creation of children objects
    m_camTrigger = new CamTrigger(this, m_secondScreen);
    m_camera = new Camera(m_ui->camView, m_cameraDevice, m_resolutionMode, m_upsideDown, m_mirror);
    m_relay = new Relay(m_relayDevice);
    m_photo = new Photo(m_photoFolder, m_isoMax, m_addWatermark);
    m_printer = new Printer(m_upsideDown);

    // Setting children
    settingRelayDevices();
    connect(this, &PhotoBooth::focusSignal, m_camTrigger, &CamTrigger::focus);
    connect(this, &PhotoBooth::triggerSignal, m_camTrigger, &CamTrigger::trigger);
}

PhotoBooth::~PhotoBooth()
{
    delete m_movie;

    delete m_camTrigger;
    delete m_countDownTimer;
    delete m_sleepTimer;
    delete m_remoteTimer;
    delete m_cameraTimer;

    delete m_camera;
    delete m_photo;
    delete m_printer;
    delete m_relay;
    delete m_pcFan;
    delete m_printerFan;
    delete m_light;
    delete m_ui;
}

void PhotoBooth::checkRemote()
{
    m_camTrigger->checkLoop();
}

void PhotoBooth::CameraLoop()
{
    m_camera->loop();
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
    settings.endArray();

    // Read photo section
    settings.beginReadArray("photo");
    m_isoMax = settings.value("isoMax").toUInt();
    m_photoFolder = settings.value("photoFolder").toString();
    if (m_photoFolder == "") {
        qDebug() << "ERROR: photoFolder path is not setin the settings file.";
        return false;
    }
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
    m_resolutionMode = settings.value("resolutionMode").toUInt();
    settings.endArray();

    // read dev section
    settings.beginReadArray("dev");
    m_modeDev = settings.value("modeDev").toBool();
    m_fullScreen = settings.value("fullScreen").toBool();
    m_secondScreen = settings.value("secondScreen").toBool();
    settings.endArray();

    // read HMI section
    settings.beginReadArray("HMI");
    m_font = settings.value("font").toString();
    m_fontSizeRatio = settings.value("fontSizeRatio").toDouble();

    QString backgroundColor1 = settings.value("backgroundColor1").toString();
    QString backgroundColor2 = settings.value("backgroundColor2").toString();
    QString fontColor = settings.value("fontColor").toString();
    settings.endArray();

    if (!generateStyleSheet(backgroundColor1, backgroundColor2, fontColor))
        return false;

    return true;
}


bool PhotoBooth::generateStyleSheet(QString backGroundColor1, QString backGroundColor2, QString fontColor)
{
    QMap<char, uint> colorRGB;

    if (!transformColor(backGroundColor1, colorRGB))
        return false;
    m_styleSheet = "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(";
    m_styleSheet += QString::number(colorRGB['R']) + ", ";
    m_styleSheet += QString::number(colorRGB['G']) + ", ";
    m_styleSheet += QString::number(colorRGB['B']) + ", 255), stop:1 rgba(";

    if (!transformColor(backGroundColor2, colorRGB))
        return false;
    m_styleSheet += QString::number(colorRGB['R']) + ", ";
    m_styleSheet += QString::number(colorRGB['G']) + ", ";
    m_styleSheet += QString::number(colorRGB['B']) + ", 255));\ncolor: rgb(";

    if (!transformColor(fontColor, colorRGB))
        return false;
    m_styleSheet += QString::number(colorRGB['R']) + ", ";
    m_styleSheet += QString::number(colorRGB['G']) + ", ";
    m_styleSheet += QString::number(colorRGB['B']) + ");";

    return true;
}


void PhotoBooth::settingDisplay()
{
    m_ui->compteur->setText(QString::number(m_printCounter));
    if (!m_modeDev)
        m_ui->warning->hide();

    if (m_secondScreen)
        move(2000, 200);

    if (m_fullScreen)
        showFullScreen();

    updateFont(m_ui->compteur);
    updateFont(m_ui->countdown);
    updateFont(m_ui->warning);
    updateFont(m_ui->lookUp);
    updateFont(m_ui->nbPrintLabel);
    updateFont(m_ui->veilleButton);
    updateFont(m_ui->buttonIncrease);
    updateFont(m_ui->buttonDecrease);

    m_ui->background->setStyleSheet(m_styleSheet);
    m_ui->veilleButton->setStyleSheet(m_styleSheet);
}

void PhotoBooth::updateFont(QLabel* label)
{
    QFont font = label->font();
    font.setFamily(m_font);
    font.setPointSize(font.pointSize() / m_fontSizeRatio);
    label->setFont(font);
}


void PhotoBooth::updateFont(QPushButton* button)
{
    QFont font = button->font();
    font.setFamily(m_font);
    font.setPointSize(font.pointSize() / m_fontSizeRatio);
    button->setFont(font);
}

void PhotoBooth::showCam()
{
    reinitSleep();

    m_ui->lookUp->hide();
    m_ui->countdown->hide();
    m_ui->buttonPhoto->show();
    m_ui->widgetPhoto->show();
    m_ui->veilleButton->hide();
    m_ui->widgetPrint->hide();

    m_camera->start();
    m_cameraTimer->start(1000/m_fps);

    m_state = SHOWING_CAM;
}

void PhotoBooth::takePhoto()
{
    m_sleepTimer->stop();
    m_ui->lookUp->hide();
    m_ui->buttonPhoto->hide();
    m_ui->lookUp->hide();

    // Countdown initialisation
    m_count = 6;
    m_countDownTimer->start(1000);
    m_ui->countdown->setText(QString::number(m_count));
    m_ui->countdown->show();
    m_ui->buttonPhoto->hide();

}

void PhotoBooth::startLoading()
{
    m_ui->loading->show();
    m_ui->veilleButton->hide();
    m_ui->widgetPhoto->hide();
    m_ui->widgetPrint->hide();
    m_ui->compteur->hide();
}

void PhotoBooth::stopLoading()
{
    QPixmap _;
    m_photo->getLast(_);
    m_ui->compteur->show();
    showCam();
}

void PhotoBooth::showPhoto()
{
    m_ui->veilleButton->hide();
    m_ui->widgetPhoto->hide();
    m_state = DISPLAY_PIC;

    m_photo->getLast(m_lastPhoto);

    if (!m_lightOn) {
        // If the iso of the last photo are higher than max, turn on the light
        if (m_photo->checkIso()) {
            qDebug() << "INFO : ISO >" << m_isoMax << "- Turning on light";
            m_lightOn = true;
            m_light->on();
        }
    }

    // Resize the photo and display it
    QPixmap photoToDisplay = m_lastPhoto.scaled(m_ui->viewer->size());
    m_ui->viewer->setPixmap(photoToDisplay);

    m_nbPrint = 1;
    updateNbPrint(0);
    m_ui->widgetPrint->show();

    m_sleepTimer->start(60000);
}

void PhotoBooth::goToSleep()
{
    m_sleepTimer->stop();
    m_ui->veilleButton->show();
}

void PhotoBooth::exit()
{
    this->close();
}

void PhotoBooth::reinitSleep()
{
    m_sleepTimer->stop();
    m_sleepTimer->start(60000);
}

void PhotoBooth::updateNbPrint(int increment)
{
    reinitSleep();

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
    reinitSleep();

    // if no photo left in the printer, no effect on the print button
    if (m_printCounter ==0)
        return;

    // send photo to printer
    m_printer->print(m_lastPhoto, m_nbPrint);

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


void PhotoBooth::countDown()
{
    m_count -= 1;
    m_ui->countdown->setText(QString::number(m_count));

    switch (m_count) {
    case 4:
        emit focusSignal();
        break;
    case 2:
        m_ui->lookUp->show();
        break;
    case 0:
        m_countDownTimer->stop();
        emit triggerSignal();
        m_camTrigger->trigger();
        showPhoto();
        break;
    }
}


void PhotoBooth::settingRelayDevices()
{
    m_relay = new Relay(m_relayDevice);
    m_pcFan = new RelayDevice(m_relay, m_relaysConfig["pcFan"]);
    m_printerFan = new RelayDevice(m_relay, m_relaysConfig["printerFan"]);
    m_light = new RelayDevice(m_relay, m_relaysConfig["light"]);
}

