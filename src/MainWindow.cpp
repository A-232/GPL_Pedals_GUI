#include "MainWindow.h"

// 1. ВАЖНО: Определяем NOMINMAX, чтобы windows.h не конфликтовал со std::min и std::max из <algorithm>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// 2. Подключаем Qt
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QCloseEvent>

// 3. ВАЖНО: Подключаем заголовки vJoy ПОСЛЕ windows.h и внутри extern "C"
extern "C" {
#include "public.h"
#include "vjoyinterface.h"
}


// Клавиши
const int KEY_GAS    = VK_MULTIPLY;   // Numpad *
const int KEY_BRAKE  = VK_F5;         // F5
const int KEY_LEFT   = VK_DIVIDE;     // Numpad /
const int KEY_RIGHT  = VK_SUBTRACT;   // Numpad -

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("GPL Smooth Pedals");
    setMinimumSize(400, 500);

    vJoyInitialized = false;
    vJoyId = 1;
    throttle = 0.0f;
    brake = 0.0f;
    steer = 0.0f;

    setupUI();
    setupTrayIcon();
    loadSettings();
    initVJoy();

    // Таймер обновления vJoy (10 мс = 100 раз/сек)
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateVJoy);
    updateTimer->start(10);
}

MainWindow::~MainWindow() {
    cleanupVJoy();
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Группа настроек газа
    QGroupBox *throttleGroup = new QGroupBox("Газ/Throttle (Клавиша/Key 'Numpad *')");
    QVBoxLayout *throttleLayout = new QVBoxLayout(throttleGroup);
    throttleSlider = new QSlider(Qt::Horizontal);
    throttleSlider->setRange(1, 50);    // Изначально было throttleSlider->setRange(1, 20);
    throttleSlider->setValue(30);    // Изначально было throttleSlider->setValue(3);
    throttleValue = new QLabel("Скорость: 0.03");
    throttleLayout->addWidget(throttleSlider);
    throttleLayout->addWidget(throttleValue);
    connect(throttleSlider, &QSlider::valueChanged, this, &MainWindow::onThrottleChanged);
    mainLayout->addWidget(throttleGroup);

    // Группа настроек тормоза
    QGroupBox *brakeGroup = new QGroupBox("Тормоз/Brake (Клавиша/Key 'F5')");
    QVBoxLayout *brakeLayout = new QVBoxLayout(brakeGroup);
    brakeSlider = new QSlider(Qt::Horizontal);
    brakeSlider->setRange(1, 20);
    brakeSlider->setValue(5);
    brakeValue = new QLabel("Скорость: 0.05");
    brakeLayout->addWidget(brakeSlider);
    brakeLayout->addWidget(brakeValue);
    connect(brakeSlider, &QSlider::valueChanged, this, &MainWindow::onBrakeChanged);
    mainLayout->addWidget(brakeGroup);

    // Группа настроек руля
    QGroupBox *steerGroup = new QGroupBox("Руль/Steering wheel (Клавиша/Key 'Numpad /' и 'Numpad -')");
    QVBoxLayout *steerLayout = new QVBoxLayout(steerGroup);
    steerSlider = new QSlider(Qt::Horizontal);
    steerSlider->setRange(1, 20);
    steerSlider->setValue(8);
    steerValue = new QLabel("Скорость: 0.08");
    steerReturnSlider = new QSlider(Qt::Horizontal);
    steerReturnSlider->setRange(1, 20);
    steerReturnSlider->setValue(10);
    steerReturnValue = new QLabel("Возврат: 0.10");
    steerLayout->addWidget(new QLabel("Поворот/Turn:"));
    steerLayout->addWidget(steerSlider);
    steerLayout->addWidget(steerValue);
    steerLayout->addWidget(new QLabel("Возврат в центр/Center return:"));
    steerLayout->addWidget(steerReturnSlider);
    steerLayout->addWidget(steerReturnValue);
    connect(steerSlider, &QSlider::valueChanged, this, &MainWindow::onSteerChanged);
    connect(steerReturnSlider, &QSlider::valueChanged, this, &MainWindow::onSteerReturnChanged);
    mainLayout->addWidget(steerGroup);

    // Группа профилей
    QGroupBox *profileGroup = new QGroupBox("Профили");
    QVBoxLayout *profileLayout = new QVBoxLayout(profileGroup);
    profileCombo = new QComboBox();
    profileCombo->addItems({"Default", "Monza", "Monaco", "Spa", "Custom"});
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton("Сохранить/Save");
    loadButton = new QPushButton("Загрузить/Download");
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(loadButton);
    profileLayout->addWidget(profileCombo);
    profileLayout->addLayout(buttonLayout);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveProfileClicked);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadProfileClicked);
    connect(profileCombo, &QComboBox::currentTextChanged, this, &MainWindow::onProfileChanged);
    mainLayout->addWidget(profileGroup);

    // Кнопка применения
    applyButton = new QPushButton("Применить настройки/Apply the settings");
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

void MainWindow::onThrottleChanged(int value) {
    throttleSpeed = value * 0.01f;
    throttleValue->setText(QString("Скорость: %1").arg(throttleSpeed, 0, 'f', 2));
}

void MainWindow::onBrakeChanged(int value) {
    brakeSpeed = value * 0.01f;
    brakeValue->setText(QString("Скорость: %1").arg(brakeSpeed, 0, 'f', 2));
}

void MainWindow::onSteerChanged(int value) {
    steerSpeed = value * 0.01f;
    steerValue->setText(QString("Скорость: %1").arg(steerSpeed, 0, 'f', 2));
}

void MainWindow::onSteerReturnChanged(int value) {
    steerReturnSpeed = value * 0.01f;
    steerReturnValue->setText(QString("Возврат: %1").arg(steerReturnSpeed, 0, 'f', 2));
}

void MainWindow::onProfileChanged(const QString &profile) {
    currentProfile = profile;
}

void MainWindow::onSaveProfileClicked() {
    saveSettings();
    QMessageBox::information(this, "Сохранено", QString("Профиль '%1' сохранён").arg(currentProfile));
}

void MainWindow::onLoadProfileClicked() {
    loadSettings();
    QMessageBox::information(this, "Загружено", QString("Профиль '%1' загружен").arg(currentProfile));
}

void MainWindow::onApplyClicked() {
    QMessageBox::information(this, "Применено", "Настройки применены!");
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

void MainWindow::loadSettings() {
    QSettings settings("GPL_Pedals", "GPL_Pedals_GUI");
    throttleSlider->setValue(settings.value("throttle", 3).toInt());
    brakeSlider->setValue(settings.value("brake", 5).toInt());
    steerSlider->setValue(settings.value("steer", 8).toInt());
    steerReturnSlider->setValue(settings.value("steerReturn", 10).toInt());
    profileCombo->setCurrentText(settings.value("profile", "Default").toString());
}

void MainWindow::saveSettings() {
    QSettings settings("GPL_Pedals", "GPL_Pedals_GUI");
    settings.setValue("throttle", throttleSlider->value());
    settings.setValue("brake", brakeSlider->value());
    settings.setValue("steer", steerSlider->value());
    settings.setValue("steerReturn", steerReturnSlider->value());
    settings.setValue("profile", currentProfile);
}

void MainWindow::initVJoy() {
    if (!vJoyEnabled()) {
        QMessageBox::critical(this, "Ошибка", "vJoy не установлен или не запущен!");
        return;
    }

    if (!AcquireVJD(vJoyId)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось захватить vJoy Device!");
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

    bool gasPressed = (GetAsyncKeyState(KEY_GAS) & 0x8000) != 0;
    bool brakePressed = (GetAsyncKeyState(KEY_BRAKE) & 0x8000) != 0;
    bool leftPressed = (GetAsyncKeyState(KEY_LEFT) & 0x8000) != 0;
    bool rightPressed = (GetAsyncKeyState(KEY_RIGHT) & 0x8000) != 0;

    // Газ
    if (gasPressed) {
        throttle = qMin(1.0f, throttle + throttleSpeed);
    } else {
        throttle = qMax(0.0f, throttle - throttleSpeed * 2);
    }

    // Тормоз
    if (brakePressed) {
        brake = qMin(1.0f, brake + brakeSpeed);
    } else {
        brake = qMax(0.0f, brake - brakeSpeed * 2);
    }

    // Руль
    if (leftPressed && !rightPressed) {
        steer = qMax(-1.0f, steer - steerSpeed);
    } else if (rightPressed && !leftPressed) {
        steer = qMin(1.0f, steer + steerSpeed);
    } else {
        if (steer > 0) steer = qMax(0.0f, steer - steerReturnSpeed);
        else if (steer < 0) steer = qMin(0.0f, steer + steerReturnSpeed);
    }

    // Отправляем в vJoy
    LONG throttleValue = static_cast<LONG>(throttle * 32768.0f);
    LONG brakeValue = static_cast<LONG>(brake * 32768.0f);
    LONG steerValue = static_cast<LONG>(1 + (steer + 1.0f) * 0.5f * 32767.0f);

    SetAxis(throttleValue, vJoyId, HID_USAGE_Y);
    SetAxis(brakeValue, vJoyId, HID_USAGE_Z);
    SetAxis(steerValue, vJoyId, HID_USAGE_X);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    hide();
    trayIcon->showMessage("GPL Smooth Pedals", "Приложение свернуто в трей. Нажмите на иконку, чтобы вернуться.");
}