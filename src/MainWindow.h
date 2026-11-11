#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QCloseEvent>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // Перехват нажатия клавиш для их переназначения
    void keyPressEvent(QKeyEvent *event) override;

    // Перехват событие закрытия окна (сворачивание в трей)
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onThrottleChanged(int value);
    void onBrakeChanged(int value);
    void onSteerChanged(int value);
    void onSteerReturnChanged(int value);
    void onProfileChanged(const QString &profile);
    void onSaveProfileClicked();
    void onLoadProfileClicked();
    void onApplyClicked();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    // Слоты для кнопок переназначения
    void onSetGasKeyClicked();
    void onSetBrakeKeyClicked();
    void onSetLeftKeyClicked();
    void onSetRightKeyClicked();

    void updateVJoy();

private:
    void setupUI();
    void setupTrayIcon();

    // === ОБЪЯВЛЕНИЯ НОВЫХ ФУНКЦИЙ для версии 1.1 ===
    void loadStartupSettings();
    void loadProfile(const QString& profileName);
    void saveProfile(const QString& profileName);
    // ============================================

    void initVJoy();
    void cleanupVJoy();
    void updateKeyLabels(); // Обновляет текст на кнопках клавиш

    // UI элементы
    QSlider *throttleSlider;
    QSlider *brakeSlider;
    QSlider *steerSlider;
    QSlider *steerReturnSlider;
    QLabel *throttleValue;
    QLabel *brakeValue;
    QLabel *steerValue;
    QLabel *steerReturnValue;
    QComboBox *profileCombo;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *applyButton;

    // Кнопки для переназначения клавиш
    QPushButton *btnSetGas;
    QPushButton *btnSetBrake;
    QPushButton *btnSetLeft;
    QPushButton *btnSetRight;

    // Системный трей
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    // Таймер для обновления vJoy
    QTimer *updateTimer;

    // Состояние переназначения клавиш
    QString waitingForKey; // "Gas", "Brake", "Left", "Right" или ""

    // Настройки (виртуальные коды Windows)
    int keyGas;
    int keyBrake;
    int keyLeft;
    int keyRight;

    QString currentProfile;

    // vJoy
    bool vJoyInitialized;
    unsigned int vJoyId;

    // Состояние педалей
    float throttle;
    float brake;
    float steer;
};

#endif // MAINWINDOW_H