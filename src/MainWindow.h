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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    void updateVJoy();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupTrayIcon();
    void loadSettings();
    void saveSettings();
    void initVJoy();
    void cleanupVJoy();

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

    // Системный трей
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    // Таймер для обновления vJoy
    QTimer *updateTimer;

    // Настройки
    float throttleSpeed;
    float brakeSpeed;
    float steerSpeed;
    float steerReturnSpeed;
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