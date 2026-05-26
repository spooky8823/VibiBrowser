# VibiBrowser 🌐

A lightweight, custom browser built with **C++ + Qt6 WebEngine** by [Vibifiy](https://vibifiy.dev).

## Why Qt instead of Electron?
- **Much lower RAM** — Qt doesn't bundle Node.js + a second Chromium
- **Faster startup** on older hardware (1st gen i5, 4GB RAM, HDD)
- **Native performance** — compiled C++, no JavaScript runtime overhead

## Features
- Multi-tab browsing with drag-to-reorder
- Custom frameless titlebar
- Dark/Light mode
- Accent color picker
- Download manager
- History manager (stored locally as JSON)
- Address bar with smart search detection

## Building locally (Linux)
```bash
sudo apt install qt6-base-dev qt6-webengine-dev
mkdir build && cd build
qmake6 ../VibiBrowser.pro
make -j$(nproc)
./VibiBrowser
```

## Building locally (Windows)
Install Qt 6.6+ with WebEngine module from https://www.qt.io/download
Then:
```
mkdir build && cd build
qmake ..\VibiBrowser.pro
nmake
```

## Automated builds (GitHub Actions)
Every push to `main` automatically builds:
- `VibiBrowser-linux.deb` — for Linux Mint / Ubuntu / Debian
- `VibiBrowser-windows.zip` — portable Windows build

Download from the **Actions** tab → latest run → **Artifacts**.

To make a full release with download links:
```bash
git tag v1.1.0
git push origin v1.1.0
```
This creates a GitHub Release with both files attached.

## Project structure
```
VibiBrowser/
├── src/
│   ├── main.cpp
│   ├── mainwindow.cpp / .h     ← main UI
│   ├── webview.cpp / .h        ← browser tab widget
│   ├── downloadmanager.cpp/.h  ← download handling
│   ├── historymanager.cpp/.h   ← history (JSON)
│   └── settings.cpp / .h      ← settings page (WIP)
├── resources/
│   └── resources.qrc
├── .github/workflows/
│   └── build.yml               ← CI/CD
└── VibiBrowser.pro             ← Qt project file
```

## Credits
Built by spooky8823 / Vibifiy
