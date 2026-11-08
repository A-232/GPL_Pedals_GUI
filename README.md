# GPL Smooth Pedals

Приложение для плавного управления педалями и рулём в Grand Prix Legends с клавиатуры.

## Требования

**Обязательно:** Установите [vJoy](https://sourceforge.net/projects/vjoystick/files/) перед запуском программы!

1. Скачайте `vJoySetup.exe` с официального сайта
2. Установите vJoy (выберите все компоненты по умолчанию)
3. После установки перезагрузите компьютер
4. Запустите `GPL_Pedals_GUI.exe`

## Использование

1. Запустите `GPL_Pedals_GUI.exe`
2. Настройте скорости педалей и руля через слайдеры
3. Нажмите "Применить настройки"
4. Запустите Grand Prix Legends
5. В настройках управления выберите устройство **vJoy Device**
6. Снимите галочки с "Throttle Help" и "Brake Help"
7. Играйте!

## Управление

- **Газ:** Numpad *
- **Тормоз:** F5
- **Руль влево:** Numpad /
- **Руль вправо:** Numpad -

## Профили

Вы можете сохранять настройки для разных трасс:
- **Monza:** более резкий газ (скорость: 0.05)
- **Monaco:** более плавный руль (скорость: 0.05)
- **Spa:** сбалансированные настройки

## Сборка из исходников

Если вы хотите собрать приложение самостоятельно:

### Требования
- Qt 6.x с Qt Creator
- MinGW UCRT64 или MSVC
- vJoy SDK

### Шаги сборки
1. Скачайте vJoy SDK с официального сайта
2. Скопируйте файлы:
   - `SDK\c\include\public.h` → `vjoy\include\`
   - `SDK\c\include\vjoyinterface.h` → `vjoy\include\`
   - `SDK\c\Lib\amd64\vJoyInterface.lib` → `vjoy\lib\`
   - `SDK\c\Lib\amd64\vJoyInterface.dll` → `vjoy\dll\`
3. Откройте `CMakeLists.txt` в Qt Creator
4. Выберите Kit MinGW UCRT64
5. Нажмите Build → Rebuild Project
6. Готово! `GPL_Pedals_GUI.exe` будет в папке `build\...`

## Лицензия

MIT License. Используйте свободно!

## Автор

[tipamandarin@gmail.com]