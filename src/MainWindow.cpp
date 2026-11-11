#include "MainWindow.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QCloseEvent>
#include <QKeySequence>

extern "C" {
#include "public.h"
#include "vjoyinterface.h"
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("GPL Smooth Pedals v1.1");
    setMinimumSize(450, 600);

    vJoyInitialized = false;
    vJoyId = 1;
    throttle = 0.0f;
    brake = 0.0f;
    steer = 0.0f;

    // Значения по умолчанию (Виртуальные коды Windows)
    keyGas = VK_MULTIPLY;
    keyBrake = VK_F5;
    keyLeft = VK_DIVIDE;
    keyRight = VK_SUBTRACT;
    waitingForKey = "";

    setupUI();
    setupTrayIcon();
    loadStartupSettings(); // Загружаем настройки при старте
    updateKeyLabels();     // Обновляем текст кнопок клавиш
    initVJoy();

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateVJoy);
    updateTimer->start(10); // 10 мс = 100 обновлений в секунду
}

MainWindow::~MainWindow() {
    cleanupVJoy();
}

// === ПЕРЕХВАТ КЛАВИШ ДЛЯ ПЕРЕНАЗНАЧЕНИЯ ===
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (!waitingForKey.isEmpty()) {
        int vkCode = event->nativeVirtualKey();

        if (vkCode != 0) {
            if (waitingForKey == "Gas") {
                keyGas = vkCode;
            } else if (waitingForKey == "Brake") {
                keyBrake = vkCode;
            } else if (waitingForKey == "Left") {
                keyLeft = vkCode;
            } else if (waitingForKey == "Right") {
                keyRight = vkCode;
            }

            waitingForKey = "";
            updateKeyLabels();

            // Вызываем saveProfile для сохранения настроек
            saveProfile(profileCombo->currentText());

            QMessageBox::information(this, "Успех", "Клавиша успешно изменена и сохранена!");
        }
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::updateKeyLabels() {
    btnSetGas->setText(QKeySequence(keyGas).toString());
    btnSetBrake->setText(QKeySequence(keyBrake).toString());
    btnSetLeft->setText(QKeySequence(keyLeft).toString());
    btnSetRight->setText(QKeySequence(keyRight).toString());
}

void MainWindow::onSetGasKeyClicked() { waitingForKey = "Gas"; btnSetGas->setText("Нажмите клавишу..."); }
void MainWindow::onSetBrakeKeyClicked() { waitingForKey = "Brake"; btnSetBrake->setText("Нажмите клавишу..."); }
void MainWindow::onSetLeftKeyClicked() { waitingForKey = "Left"; btnSetLeft->setText("Нажмите клавишу..."); }
void MainWindow::onSetRightKeyClicked() { waitingForKey = "Right"; btnSetRight->setText("Нажмите клавишу..."); }

// === НАСТРОЙКА ИНТЕРФЕЙСА ===
void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // --- ГАЗ ---
    QGroupBox *throttleGroup = new QGroupBox("Газ /Throttle");
    QVBoxLayout *throttleLayout = new QVBoxLayout(throttleGroup);

    QHBoxLayout *gasKeyLayout = new QHBoxLayout();
    gasKeyLayout->addWidget(new QLabel("Клавиша /Key:"));
    btnSetGas = new QPushButton();
    btnSetGas->setMaximumWidth(120);
    connect(btnSetGas, &QPushButton::clicked, this, &MainWindow::onSetGasKeyClicked);
    gasKeyLayout->addWidget(btnSetGas);
    throttleLayout->addLayout(gasKeyLayout);

    throttleSlider = new QSlider(Qt::Horizontal);
    throttleSlider->setRange(10, 200);
    throttleSlider->setValue(30);
    throttleValue = new QLabel("Время разгона /Acceleration Time: 30 мс");
    throttleLayout->addWidget(throttleSlider);
    throttleLayout->addWidget(throttleValue);
    connect(throttleSlider, &QSlider::valueChanged, this, &MainWindow::onThrottleChanged);
    mainLayout->addWidget(throttleGroup);

    // --- ТОРМОЗ ---
    QGroupBox *brakeGroup = new QGroupBox("Тормоз /Brake");
    QVBoxLayout *brakeLayout = new QVBoxLayout(brakeGroup);

    QHBoxLayout *brakeKeyLayout = new QHBoxLayout();
    brakeKeyLayout->addWidget(new QLabel("Клавиша /Key:"));
    btnSetBrake = new QPushButton();
    btnSetBrake->setMaximumWidth(120);
    connect(btnSetBrake, &QPushButton::clicked, this, &MainWindow::onSetBrakeKeyClicked);
    brakeKeyLayout->addWidget(btnSetBrake);
    brakeLayout->addLayout(brakeKeyLayout);

    brakeSlider = new QSlider(Qt::Horizontal);
    brakeSlider->setRange(10, 200);
    brakeSlider->setValue(40);
    brakeValue = new QLabel("Время торможения /Braking Time: 40 мс");
    brakeLayout->addWidget(brakeSlider);
    brakeLayout->addWidget(brakeValue);
    connect(brakeSlider, &QSlider::valueChanged, this, &MainWindow::onBrakeChanged);
    mainLayout->addWidget(brakeGroup);

    // --- РУЛЬ ---
    QGroupBox *steerGroup = new QGroupBox("Руль /Steering");
    QVBoxLayout *steerLayout = new QVBoxLayout(steerGroup);

    QHBoxLayout *steerKeyLayout = new QHBoxLayout();
    steerKeyLayout->addWidget(new QLabel("Влево /Left:"));
    btnSetLeft = new QPushButton();
    btnSetLeft->setMaximumWidth(120);
    connect(btnSetLeft, &QPushButton::clicked, this, &MainWindow::onSetLeftKeyClicked);
    steerKeyLayout->addWidget(btnSetLeft);

    steerKeyLayout->addWidget(new QLabel("Вправо /Right:"));
    btnSetRight = new QPushButton();
    btnSetRight->setMaximumWidth(120);
    connect(btnSetRight, &QPushButton::clicked, this, &MainWindow::onSetRightKeyClicked);
    steerKeyLayout->addWidget(btnSetRight);
    steerLayout->addLayout(steerKeyLayout);

    steerSlider = new QSlider(Qt::Horizontal);
    steerSlider->setRange(10, 200);
    steerSlider->setValue(60);
    steerValue = new QLabel("Время поворота /Steering Turn Time: 60 мс");

    steerReturnSlider = new QSlider(Qt::Horizontal);
    steerReturnSlider->setRange(10, 200);
    steerReturnSlider->setValue(40);
    steerReturnValue = new QLabel("Время возврата /Steering Return Time: 40 мс");

    steerLayout->addWidget(new QLabel("Поворот /Turn:"));
    steerLayout->addWidget(steerSlider);
    steerLayout->addWidget(steerValue);
    steerLayout->addWidget(new QLabel("Возврат в центр /Center return:"));
    steerLayout->addWidget(steerReturnSlider);
    steerLayout->addWidget(steerReturnValue);

    connect(steerSlider, &QSlider::valueChanged, this, &MainWindow::onSteerChanged);
    connect(steerReturnSlider, &QSlider::valueChanged, this, &MainWindow::onSteerReturnChanged);
    mainLayout->addWidget(steerGroup);

    // --- ПРОФИЛИ ---
    QGroupBox *profileGroup = new QGroupBox("Профили / Profiles");
    QVBoxLayout *profileLayout = new QVBoxLayout(profileGroup);
    profileCombo = new QComboBox();
    profileCombo->addItems({"Default", "Monza", "Monaco", "Spa", "Watkins Glen", "Custom"});

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton("Сохранить / Save");
    loadButton = new QPushButton("Загрузить / Load");
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(loadButton);

    profileLayout->addWidget(profileCombo);
    profileLayout->addLayout(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveProfileClicked);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadProfileClicked);
    connect(profileCombo, &QComboBox::currentTextChanged, this, &MainWindow::onProfileChanged);
    mainLayout->addWidget(profileGroup);

    // --- ПРИМЕНИТЬ ---
    applyButton = new QPushButton("Применить настройки / Apply Settings");
    applyButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; padding: 10px;");
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::onApplyClicked);
    mainLayout->addWidget(applyButton);

    mainLayout->addStretch();
}

void MainWindow::setupTrayIcon() {
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    trayIcon->setToolTip("GPL Smooth Pedals");

    trayMenu = new QMenu();
    QAction *showAction = new QAction("Показать окно", this);
    QAction *quitAction = new QAction("Выход", this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    connect(showAction, &QAction::triggered, this, &QWidget::show);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

// === ОБРАБОТЧИКИ СЛАЙДЕРОВ ===
void MainWindow::onThrottleChanged(int value) {
    throttleValue->setText(QString("Время разгона: %1 мс").arg(value));
}
void MainWindow::onBrakeChanged(int value) {
    brakeValue->setText(QString("Время разгона: %1 мс").arg(value));
}
void MainWindow::onSteerChanged(int value) {
    steerValue->setText(QString("Время поворота: %1 мс").arg(value));
}
void MainWindow::onSteerReturnChanged(int value) {
    steerReturnValue->setText(QString("Время возврата: %1 мс").arg(value));
}

void MainWindow::onProfileChanged(const QString &profile) {
    currentProfile = profile;
}

// === СОХРАНЕНИЕ И ЗАГРУЗКА ПРОФИЛЕЙ ===
void MainWindow::loadProfile(const QString& profileName) {
    QSettings settings("GPL_Pedals", "Profiles");
    settings.beginGroup(profileName);

    throttleSlider->setValue(settings.value("throttle", 30).toInt());
    brakeSlider->setValue(settings.value("brake", 40).toInt());
    steerSlider->setValue(settings.value("steer", 60).toInt());
    steerReturnSlider->setValue(settings.value("steerReturn", 40).toInt());

    keyGas = settings.value("keyGas", VK_MULTIPLY).toInt();
    keyBrake = settings.value("keyBrake", VK_F5).toInt();
    keyLeft = settings.value("keyLeft", VK_DIVIDE).toInt();
    keyRight = settings.value("keyRight", VK_SUBTRACT).toInt();
    settings.endGroup();

    updateKeyLabels();
}

void MainWindow::saveProfile(const QString& profileName) {
    QSettings settings("GPL_Pedals", "Profiles");
    settings.beginGroup(profileName);

    settings.setValue("throttle", throttleSlider->value());
    settings.setValue("brake", brakeSlider->value());
    settings.setValue("steer", steerSlider->value());
    settings.setValue("steerReturn", steerReturnSlider->value());
    settings.setValue("keyGas", keyGas);
    settings.setValue("keyBrake", keyBrake);
    settings.setValue("keyLeft", keyLeft);
    settings.setValue("keyRight", keyRight);
    settings.endGroup();

    settings.setValue("lastProfile", profileName);
}

void MainWindow::loadStartupSettings() {
    QSettings settings("GPL_Pedals", "Profiles");
    QString lastProfile = settings.value("lastProfile", "Default").toString();
    profileCombo->setCurrentText(lastProfile);
    loadProfile(lastProfile);
}

void MainWindow::onSaveProfileClicked() {
    saveProfile(profileCombo->currentText());
    QMessageBox::information(this, "Сохранено", QString("Профиль '%1' успешно сохранён!").arg(profileCombo->currentText()));
}

void MainWindow::onLoadProfileClicked() {
    loadProfile(profileCombo->currentText());
    QMessageBox::information(this, "Загружено", QString("Профиль '%1' успешно загружен!").arg(profileCombo->currentText()));
}

void MainWindow::onApplyClicked() {
    saveProfile(profileCombo->currentText());
    QMessageBox::information(this, "Применено", "Настройки сохранены и применены!");
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isVisible()) {
            hide();
        } else {
            show();
            raise();
        }
    }
}

// === ЛОГИКА VJOY ===
void MainWindow::initVJoy() {
    if (!vJoyEnabled()) {
        QMessageBox::critical(this, "Ошибка", "vJoy не установлен или не запущен!\nУстановите vJoy и перезагрузите ПК.");
        return;
    }
    if (!AcquireVJD(vJoyId)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось захватить vJoy Device 1!\nЗакройте другие программы (JoyToKey, reWASD) или перезагрузите ПК.");
        return;
    }
    ResetVJD(vJoyId);
    vJoyInitialized = true;
}

void MainWindow::cleanupVJoy() {
    if (vJoyInitialized) {
        RelinquishVJD(vJoyId);
    }
}

void MainWindow::updateVJoy() {
    if (!vJoyInitialized) return;

    bool gasPressed = (GetAsyncKeyState(keyGas) & 0x8000) != 0;
    bool brakePressed = (GetAsyncKeyState(keyBrake) & 0x8000) != 0;
    bool leftPressed = (GetAsyncKeyState(keyLeft) & 0x8000) != 0;
    bool rightPressed = (GetAsyncKeyState(keyRight) & 0x8000) != 0;

    // Расчёт приращения на основе времени (мс)
    float throttleInc = 1.0f / (throttleSlider->value() / 10.0f);
    float brakeInc = 1.0f / (brakeSlider->value() / 10.0f);
    float steerInc = 1.0f / (steerSlider->value() / 10.0f);
    float steerRetInc = 1.0f / (steerReturnSlider->value() / 10.0f);

    if (gasPressed) {
        throttle = qMin(1.0f, throttle + throttleInc);
    } else {
        throttle = qMax(0.0f, throttle - throttleInc * 1.5f);
    }

    if (brakePressed) {
        brake = qMin(1.0f, brake + brakeInc);
    } else {
        brake = qMax(0.0f, brake - brakeInc * 1.5f);
    }

    if (leftPressed && !rightPressed) {
        steer = qMax(-1.0f, steer - steerInc);
    } else if (rightPressed && !leftPressed) {
        steer = qMin(1.0f, steer + steerInc);
    } else {
        if (steer > 0) steer = qMax(0.0f, steer - steerRetInc);
        else if (steer < 0) steer = qMin(0.0f, steer + steerRetInc);
    }

    LONG tVal = static_cast<LONG>(throttle * 32768.0f);
    LONG bVal = static_cast<LONG>(brake * 32768.0f);
    LONG sVal = static_cast<LONG>(1 + (steer + 1.0f) * 0.5f * 32767.0f);

    SetAxis(tVal, vJoyId, HID_USAGE_Y);
    SetAxis(bVal, vJoyId, HID_USAGE_Z);
    SetAxis(sVal, vJoyId, HID_USAGE_X);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    hide();
    trayIcon->showMessage("GPL Smooth Pedals", "Приложение свернуто в трей. Нажмите на иконку, чтобы вернуться.");
}