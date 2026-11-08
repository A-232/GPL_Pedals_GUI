# GPL Smooth Pedals

An application for smooth keyboard control of pedals and steering in Grand Prix Legends.

## Requirements

**Required:** Install [vJoy](https://sourceforge.net/projects/vjoystick/files/) before running the program!

1. Download `vJoySetup.exe` from the official website
2. Install vJoy (keep all components at their defaults)
3. Restart your computer after installation
4. Run `GPL_Pedals_GUI.exe`

## Usage

1. Run `GPL_Pedals_GUI.exe`
2. Adjust pedal and steering speeds with the sliders
3. Click "Apply Settings"
4. Launch Grand Prix Legends
5. In the control settings, select the **vJoy Device**
6. Uncheck "Throttle Help" and "Brake Help"
7. Enjoy the game!

## Controls

- **Throttle:** Numpad *
- **Brake:** F5
- **Steer left:** Numpad /
- **Steer right:** Numpad -

## Profiles

You can save settings for different tracks:
- **Monza:** sharper throttle (speed: 0.05)
- **Monaco:** smoother steering (speed: 0.05)
- **Spa:** balanced settings

## Building from Source

If you want to build the application yourself:

### Requirements
- Qt 6.x with Qt Creator
- MinGW UCRT64 or MSVC
- vJoy SDK

### Build Steps
1. Download the vJoy SDK from the official website
2. Copy the files:
   - `SDK\c\include\public.h` → `vjoy\include\`
   - `SDK\c\include\vjoyinterface.h` → `vjoy\include\`
   - `SDK\c\Lib\amd64\vJoyInterface.lib` → `vjoy\lib\`
   - `SDK\c\Lib\amd64\vJoyInterface.dll` → `vjoy\dll\`
3. Open `CMakeLists.txt` in Qt Creator
4. Select the MinGW UCRT64 kit
5. Click Build → Rebuild Project
6. Done! `GPL_Pedals_GUI.exe` will be in the `build\...` folder

## License

MIT License. Use freely!

## Author

[tipamandarin@gmail.com]