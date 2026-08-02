# 🇬🇧 Universal Media Loader 🎬

📥 A lightweight desktop app for downloading videos and streams, and extracting audio as MP3.

**Universal Media Loader** is a fast, low-overhead C++/Qt6 rewrite of an earlier Python/CustomTkinter tool. It downloads videos and streams from **YouTube**, **Twitch**, and **TikTok**, and extracts audio as MP3 — with quick startup and a small memory footprint.

## ⬇️ Download

Grab the latest ready-to-run UniversalMediaLoader.exe from the Releases page — it's a single self-contained file, no dependencies or extra files needed. Just download and launch.

[![Download Universal Media Loader](https://img.shields.io/badge/Download-Universal_Media_Loader-v2.0.0-blue?style=for-the-badge&logo=download&logoColor=white)](https://github.com/By183/UniversalMediaLoader/releases/latest/download/UniversalMediaLoader.exe)

## ✨ Features

- ⬇️ Video download at multiple qualities
- 🎵 Audio-only MP3 extraction with embedded cover art
- 🖼️ Standalone thumbnail/preview download
- ⏸️ Pause / resume / cancel with proper cleanup
- ✂️ Time-range cropping (download only a specific section of a video/stream) with live progress
- 📋 Automatic playlist support
- 🌗 Dark and light theme with smooth transitions
- 🔄 Self-updating download engine (yt-dlp)
- 🌐 Cyrillic / Unicode filename support
- ✏️ Custom output filenames

## 🛠️ Tech stack

- C++17
- Qt6 (Widgets)
- CMake
- Wraps [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [ffmpeg](https://ffmpeg.org/) as external tools — it does not reimplement extraction logic

## 🏗️ Requirements / how to build

On Windows, with [Qt 6](https://www.qt.io/download-qt-installer) and [CMake](https://cmake.org/) installed:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/<version>/msvc2019_64
cmake --build build --config Release
```

The C++ rewrite targets C++17, Qt6 Widgets, and the MSVC toolchain.

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

📥 Легковесное приложение для скачивания видео и стримов, а также извлечения аудио в MP3.

**Universal Media Loader** — быстрый C++/Qt6 клиент с низким уровнем накладных расходов, созданный как полный перенос более ранней версии на Python/CustomTkinter. Приложение скачивает видео и стримы с **YouTube**, **Twitch** и **TikTok**, а также извлекает аудио в MP3 — с быстрым запуском и малым потреблением памяти.

## ⬇️ Скачать

Готовый к запуску UniversalMediaLoader.exe можно забрать на странице релизов — это один самодостаточный файл, без зависимостей. Просто скачайте и запустите.

[![Скачать Universal Media Loader](https://img.shields.io/badge/%D0%A1%D0%BA%D0%B0%D1%87%D0%B0%D1%82%D1%8C-Universal_Media_Loader-v2.0.0-blue?style=for-the-badge&logo=download&logoColor=white)](https://github.com/By183/UniversalMediaLoader/releases/latest/download/UniversalMediaLoader.exe)

## ✨ Возможности

- ⬇️ Скачивание видео в нескольких качествах
- 🎵 Извлечение аудио в MP3 со встроенной обложкой
- 🖼️ Отдельное скачивание превью / обложки
- ⏸️ Пауза / продолжение / отмена с правильной очисткой
- ✂️ Обрезка по времени (скачивание только нужного фрагмента видео/стрима) с прогрессом в реальном времени
- 📋 Автоматическая поддержка плейлистов
- 🌗 Тёмная и светлая тема с плавными переходами
- 🔄 Самообновляемый движок загрузки (yt-dlp)
- 🌐 Поддержка кириллицы / Unicode в именах файлов
- ✏️ Пользовательские имена выходных файлов

## 🛠️ Технологии

- C++17
- Qt6 (Widgets)
- CMake
- Использует [yt-dlp](https://github.com/yt-dlp/yt-dlp) и [ffmpeg](https://ffmpeg.org/) как внешние инструменты — не переписывает логику извлечения

## 🏗️ Требования и сборка

В Windows с установленными [Qt 6](https://www.qt.io/download-qt-installer) и [CMake](https://cmake.org/):

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/<version>/msvc2019_64
cmake --build build --config Release
```

Проект рассчитан на C++17, Qt6 Widgets и тулчейн MSVC.

## 🧪 Тесты

Для включения модульных и интеграционных тестов:

```powershell
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Release
```

## 🙏 Благодарности
