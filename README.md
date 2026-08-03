# 🇬🇧 Universal Media Loader 🎬

📥 A polished, fast, native desktop app for downloading video and audio from YouTube, Twitch, and TikTok.

**Universal Media Loader** is a complete, production-ready media downloader. It fetches videos and streams from **YouTube**, **Twitch**, and **TikTok**, and extracts audio as MP3 — with a clean native Qt interface, quick startup, and a small memory footprint. Everything you need is packed into a single self-contained file.

## ⬇️ Download

Grab the latest ready-to-run **UniversalMediaLoader.exe** from the Releases page — it's a single self-contained file, no dependencies or extra files needed. Just download and launch.

👉 [**⬇️ Download UniversalMediaLoader.exe**](https://github.com/By183/UniversalMediaLoader/releases/latest/download/UniversalMediaLoader.exe)

## ✨ Features

- ⬇️ Video download in multiple qualities (1080p/720p/480p/360p)
- 🎵 Audio-only MP3 extraction with embedded cover art
- 🖼️ Standalone thumbnail/preview download
- ⏸️ Pause/resume/cancel with automatic cleanup — no leftover files
- ✂️ Time-range cropping — download only the exact section you need, with live progress
- 📋 Automatic playlist support
- 🌗 Dark/light theme with smooth transitions
- 🔄 Self-updating download engine
- 🔤 Full Unicode/Cyrillic filename support
- ✏️ Custom output filenames

## 🛠️ Tech stack

- C++17
- Qt6 (Widgets)
- CMake
- Built on top of the industry-standard [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [ffmpeg](https://ffmpeg.org/).

## 🏗️ Requirements / how to build

On Windows, with [Qt 6](https://www.qt.io/download-qt-installer) and [CMake](https://cmake.org/) installed:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/<version>/msvc2019_64
cmake --build build --config Release
```

The app targets C++17, Qt6 Widgets, and the MSVC toolchain.

## 🧪 Tests

Unit and integration tests can be enabled with:

```powershell
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Release
```

## 🙏 Credits

Built on top of [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [ffmpeg](https://ffmpeg.org/).

---

# 🇷🇺 Universal Media Loader 🎬

📥 Красивое, быстрое нативное десктоп-приложение для скачивания видео и аудио с YouTube, Twitch и TikTok.

**Universal Media Loader** — полнофункциональный, готовый к использованию медиа-загрузчик. Он извлекает видео и аудио с **YouTube**, **Twitch** и **TikTok**, а также извлекает аудио в MP3 — с современным нативным Qt-интерфейсом, быстрым запуском и малым потреблением памяти. Всё необходимое упаковано в один автономный файл.

## ⬇️ Скачать

Готовый к запуску **UniversalMediaLoader.exe** можно забрать на странице релизов — это один самодостаточный файл, без зависимостей. Просто скачайте и запустите.

👉 [**⬇️ Скачать UniversalMediaLoader.exe**](https://github.com/By183/UniversalMediaLoader/releases/latest/download/UniversalMediaLoader.exe)

## ✨ Возможности

- ⬇️ Скачивание видео в нескольких качествах (1080p/720p/480p/360p)
- 🎵 Извлечение аудио в MP3 со встроенной обложкой
- 🖼️ Отдельное скачивание превью / обложки
- ⏸️ Пауза / продолжение / отмена с автоматической очисткой — без лишних файлов
- ✂️ Обрезка по времени — скачивание только нужного фрагмента, с прогрессом в реальном времени
- 📋 Автоматическая поддержка плейлистов
- 🌗 Тёмная и светлая тема с плавными переходами
- 🔄 Самообновляемый движок загрузки
- 🔤 Полная поддержка кириллицы / Unicode в именах файлов
- ✏️ Пользовательские имена выходных файлов

## 🛠️ Технологии

- C++17
- Qt6 (Widgets)
- CMake
- Построено на отраслевых стандартах [yt-dlp](https://github.com/yt-dlp/yt-dlp) и [ffmpeg](https://ffmpeg.org/).

## 🏗️ Требования и сборка

В Windows с установленными [Qt 6](https://www.qt.io/download-qt-installer) и [CMake](https://cmake.org/):

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/<version>/msvc2019_64
cmake --build build --config Release
```

Приложение рассчитано на C++17, Qt6 Widgets и тулчейн MSVC.

## 🧪 Тесты

Для включения модульных и интеграционных тестов:

```powershell
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Release
```

## 🙏 Благодарности

Построено на основе [yt-dlp](https://github.com/yt-dlp/yt-dlp) и [ffmpeg](https://ffmpeg.org/).