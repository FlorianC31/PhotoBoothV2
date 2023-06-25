#include "PhotoBooth.h"
#include "ui_PhotoBooth.h"
#include "lib/utils.h"

#define CHECK_REMOTE_PERIOD 1000 //ms
#define CHECK_CPU_TEMP_PERIOD 1000 //ms

/**
 * @brief PhotoBooth::PhotoBooth Main photobooth window class
 * @param parent parent QApplication
 */
PhotoBooth::PhotoBooth(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::PhotoBooth),
    m_camera(nullptr),
    m_camTrigger(nullptr),
    m_photo(nullptr),
    m_printer(nullptr),
    m_cpuTemp(nullptr),
    m_movie(nullptr),
    m_settingFile("settings.ini"),
    m_state(INIT),
    m_countDownTimer(nullptr),
    m_sleepTimer(nullptr),
    m_remoteTimer(nullptr),
    m_cameraTimer(nullptr),
    m_cpuTimer(nullptr),
    m_lightOn(false),
    m_relay(nullptr),
    m_pcFan(nullptr),
    m_printerFan(nullptr),
    m_light(nullptr)
{
    m_ui->setupUi(this);

    qDebug(" ");
    qDebug(" ");
    qDebug("########################## NEW SESSION #############################");

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

    startLoading();

    // Read the settings file
    qDebug() << "PHOTOBOOTH - READING SETTINGS FILE";
    if (!readingSettingsFile()) {
        this->close();
    }
    qDebug() << "PHOTOBOOTH -  -> Done";

    // Setting display
    qDebug() << "PHOTOBOOTH - SETTING DISPLAY";
    settingDisplay();
    qDebug() << "PHOTOBOOTH -  -> Done";

    // Creation of all timers
    qDebug() << "PHOTOBOOTH - CREATION OF TIMERS";
    m_countDownTimer = new QTimer(this);
    m_sleepTimer = new QTimer(this);
    m_remoteTimer = new QTimer(this);
    m_cameraTimer = new QTimer(this);
    m_cpuTimer = new QTimer(this);
    connect(m_countDownTimer, &QTimer::timeout, this, &PhotoBooth::countDown);
    connect(m_sleepTimer, &QTimer::timeout, this, &PhotoBooth::goToSleep);
    qDebug() << "PHOTOBOOTH -  -> Done";

    // Creation of children objects    
    qDebug() << "PHOTOBOOTH - Loading of Photo";
    m_photo = new Photo(this, m_photoFolder, m_isoMax, m_addWatermark, m_ui->viewer->size());
    connect(this, &PhotoBooth::loadLastPhoto, m_photo, &Photo::loadLast);

    qDebug() << "PHOTOBOOTH - Loading of CamTrigger";
    m_camTrigger = new CamTrigger(this, m_secondScreen);
    connect(m_remoteTimer, &QTimer::timeout, m_camTrigger, &CamTrigger::checkLoop);
    m_remoteTimer->start(CHECK_REMOTE_PERIOD);

    qDebug() << "PHOTOBOOTH - Loading of Camera";
    m_camera = new Camera(this, m_ui->camView, m_cameraDevice, m_resolutionMode, m_upsideDown, m_mirror);
    connect(m_cameraTimer, &QTimer::timeout, m_camera, &Camera::loop);
    m_camera->connection();

    qDebug() << "PHOTOBOOTH - Loading of Relay";
    settingRelayDevices();
    qDebug() << "PHOTOBOOTH -  -> Relay loaded";

    qDebug() << "PHOTOBOOTH - Loading of Printer";
    m_printer = new Printer(m_upsideDown);
    qDebug() << "PHOTOBOOTH -  -> Printer loaded";

    qDebug() << "PHOTOBOOTH - Loading of CpuTemp";
    m_cpuTemp = new CpuTemp(m_pcFan);
    connect(m_cpuTimer, &QTimer::timeout, m_cpuTemp, &CpuTemp::loop);
    m_cpuTimer->start(CHECK_CPU_TEMP_PERIOD);
    qDebug() << "PHOTOBOOTH -  -> CpuTemp loaded";

    // Setting children    
    qDebug() << "PHOTOBOOTH - SETTING CHILDREN OBJECTS";
    connect(this, &PhotoBooth::focusSignal, m_camTrigger, &CamTrigger::focus);
    connect(this, &PhotoBooth::triggerSignal, m_camTrigger, &CamTrigger::trigger);
    qDebug() << "PHOTOBOOTH -  -> Children objects set";

}

/**
 * @brief PhotoBooth::~PhotoBooth PhotoBooth destuctor
 */
PhotoBooth::~PhotoBooth()
{
    delete m_movie;

    delete m_camTrigger;
    delete m_countDownTimer;
    delete m_sleepTimer;
    delete m_remoteTimer;
    delete m_cameraTimer;
    delete m_cpuTimer;

    delete m_camera;
    delete m_photo;
    delete m_printer;
    delete m_cpuTemp;
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
        qDebug() << "PHOTOBOOTH - Error: The file " << QDir::currentPath() + "/" + m_settingFile << "doesn't exist.";
        return false;
    }
    qDebug() << "PHOTOBOOTH - Reading file " << QDir::currentPath() + "/" + m_settingFile;
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
        qDebug() << "PHOTOBOOTH - ERROR: photoFolder path is not setin the settings file.";
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


/**
 * @brief PhotoBooth::generateStyleSheet generate styleSheet from colors
 * @param backGroundColor1 Color1 for background shading color
 * @param backGroundColor2 Color2 for background shading color
 * @param fontColor color for the text
 * @return true if the input color are correct
 */
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


/**
 * @brief PhotoBooth::settingDisplay Setting up the display
 */
void PhotoBooth::settingDisplay()
{
    m_ui->flash->hide();

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


/**
 * @brief PhotoBooth::updateFont update the font familly and size in a QLabel
 * @param label label where update the font
 */
void PhotoBooth::updateFont(QLabel* label)
{
    QFont font = label->font();
    font.setFamily(m_font);
    font.setPointSize(font.pointSize() / m_fontSizeRatio);
    label->setFont(font);
}


/**
 * @brief PhotoBooth::updateFont update the font familly and size in a QPushButton
 * @param button button where update the font
 */
void PhotoBooth::updateFont(QPushButton* button)
{
    QFont font = button->font();
    font.setFamily(m_font);
    font.setPointSize(font.pointSize() / m_fontSizeRatio);
    button->setFont(font);
}


/**
 * @brief PhotoBooth::showCam show the camera in the UI
 */
void PhotoBooth::showCam()
{
    reinitSleep();

    m_remoteTimer->start(CHECK_REMOTE_PERIOD);

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


/**
 * @brief PhotoBooth::takePhoto take the photo
 */
void PhotoBooth::takePhoto()
{
    m_sleepTimer->stop();
    m_ui->lookUp->hide();
    m_ui->buttonPhoto->hide();
    m_ui->lookUp->hide();

    // Countdown initialisation
    m_count = 60;
    m_sleepTimer->stop();
    m_remoteTimer->stop();
    m_countDownTimer->start(100);

    m_ui->countdown->setText(QString::number(ceil(0.1*m_count)));
    m_ui->countdown->show();
    m_ui->buttonPhoto->hide();

}


/**
 * @brief PhotoBooth::startLoading start the loading of the photobooth
 */
void PhotoBooth::startLoading()
{
    qDebug() << "PHOTOBOOTH - Start loading";
    m_ui->loading->show();
    m_ui->veilleButton->hide();
    m_ui->widgetPhoto->hide();
    m_ui->widgetPrint->hide();
    m_ui->compteur->hide();
}


/**
 * @brief PhotoBooth::stopLoading stop the loading of the photobooth
 */
void PhotoBooth::stopLoading()
{
    qDebug() << "PHOTOBOOTH - Stop loading";
    m_ui->loading->hide();
    m_ui->compteur->show();
    showCam();
}


/**
 * @brief PhotoBooth::showPhoto show the last photo in the UI
 */
void PhotoBooth::loadNewPhoto(QPixmap* lastPhoto, QPixmap* lastPhoto2Print)
{
    if (m_state == INIT){
        endOfModuleLoading(PHOTO);
        return;
    }

    m_lastPhoto = lastPhoto;
    m_lastPhoto2Print = lastPhoto2Print;

    m_ui->veilleButton->hide();
    m_ui->widgetPhoto->hide();
    m_state = DISPLAY_PIC;

    if (!m_lightOn) {
        // If the iso of the last photo are higher than max, turn on the light
        if (m_photo->checkIso()) {
            qDebug() << "PHOTOBOOTH - INFO : ISO >" << m_isoMax << "- Turning on light";
            m_lightOn = true;
            m_light->on();
        }
    }

    // Resize the photo and display it
    m_ui->viewer->setPixmap(*m_lastPhoto);

    m_nbPrint = 1;
    updateNbPrint(0);
    m_ui->widgetPrint->show();
    m_ui->loading->hide();

    m_sleepTimer->start(60000);
}


/**
 * @brief PhotoBooth::goToSleep enter in sleep mode
 */
void PhotoBooth::goToSleep()
{
    m_sleepTimer->stop();
    m_ui->veilleButton->show();
}


/**
 * @brief PhotoBooth::exit close the application
 */
void PhotoBooth::exit()
{
    this->close();
}


/**
 * @brief PhotoBooth::reinitSleep reinitalize the "GoToSleep" timer
 */
void PhotoBooth::reinitSleep()
{
    m_sleepTimer->stop();
    m_sleepTimer->start(60000);
}


/**
 * @brief PhotoBooth::updateNbPrint slot called by the de/increase printer buttons to adjust the number of copies to print
 * @param increment +1 or -1 to increase or decrease the number of copies to print
 */
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


/**
 * @brief PhotoBooth::btnActivate activate a button in the UI
 * @param button button to activate
 */
void PhotoBooth::btnActivate(QPushButton* button)
{
    button->setStyleSheet("background-color: transparent;\ncolor: rgb(255, 255, 255);");
}


/**
 * @brief PhotoBooth::btnDisable disable a button in the UI
 * @param button button to disable
 */
void PhotoBooth::btnDisable(QPushButton* button)
{
    button->setStyleSheet("background-color: transparent;\ncolor: rgb(127, 127, 127);");
}


/**
 * @brief PhotoBooth::print print the last photo
 */
void PhotoBooth::print()
{
    reinitSleep();

    // if no photo left in the printer, no effect on the print button
    if (m_printCounter ==0)
        return;

    // send photo to printer
    m_printer->print(m_lastPhoto2Print, m_nbPrint);

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


/**
 * @brief PhotoBooth::countDown slot periodically called by the countdown timer
 */
void PhotoBooth::countDown()
{
    m_count -= 1;
    if (m_count>0) {
        m_ui->countdown->setText(QString::number(ceil(0.1*m_count)));
    }

    switch (m_count) {
    case 40:
        emit focusSignal();
        break;
    case 20:
        m_ui->lookUp->show();
        break;
    case 0:
        m_ui->flash->show();
        emit triggerSignal();
        break;
    case -4:
        startLoading();
        m_ui->flash->hide();        
        emit loadLastPhoto();
        m_countDownTimer->stop();
        break;
    }
}


/**
 * @brief PhotoBooth::settingRelayDevices initialize the relay devices
 */
void PhotoBooth::settingRelayDevices()
{
    m_relay = new Relay(this, m_relayDevice);
    m_pcFan = new RelayDevice(m_relay, "pcFan", m_relaysConfig["pcFan"]);
    m_printerFan = new RelayDevice(m_relay, "printerFan", m_relaysConfig["printerFan"]);
    m_light = new RelayDevice(m_relay, "light", m_relaysConfig["light"], true);
    m_pcFan->on();
    m_printerFan->on();
}

void PhotoBooth::showFlash(bool show)
{
    if(show){
        m_ui->flash->show();
    }
    else{
        m_ui->flash->hide();
    }
}

void PhotoBooth::endOfModuleLoading(PhotoBooth::Module module)
{
    switch (module){
    case CAMERA:
        qDebug() << "PHOTOBOOTH -  -> Camera loaded";
        break;
    case CAM_TRIGGER:
        qDebug() << "PHOTOBOOTH -  -> CamTrigger loaded";
        emit loadLastPhoto();
        break;
    case PHOTO:
        qDebug() << "PHOTOBOOTH -  -> Photo loaded";
        break;
    }

    if(m_camera != nullptr && m_camTrigger != nullptr && m_photo != nullptr) {
        qDebug() << "PHOTOBOOTH - Camera loaded:" << m_camera->isLoaded();
        qDebug() << "PHOTOBOOTH - CamTrigger loaded:" << m_camTrigger->isLoaded();

        if (m_camera->isLoaded() && m_camTrigger->isLoaded() && m_photo->isInitialized()) {
            stopLoading();
        }
    }
}
