<div align="center">

**[English](README.md)** · **[Русский](https://github.com/By183/UniversalMediaLoader/blob/main/README.ru.md)**

---

# Universal Media Loader

A polished, fast, native desktop app for downloading video and audio from YouTube, Twitch, and TikTok.

</div>

**Universal Media Loader** is a complete, production-ready media downloader. It fetches videos and streams from **YouTube**, **Twitch**, and **TikTok**, and extracts audio as MP3 — with a clean native Qt interface, quick startup, and a small memory footprint. Everything you need is packed into a single self-contained file.

## Download

Download the **UniversalMediaLoader-v2.0.0-win64.zip** release archive, extract it anywhere, and run **UniversalMediaLoader.exe** — no installation needed, everything required (Qt runtime, plugins, yt-dlp) is included in the folder.

[**Download UniversalMediaLoader-v2.0.0-win64.zip**](https://github.com/By183/UniversalMediaLoader/releases/latest/download/UniversalMediaLoader-v2.0.0-win64.zip)

## Features

- Video download in multiple qualities (1080p/720p/480p/360p)
- Audio-only MP3 extraction with embedded cover art
- Standalone thumbnail/preview download
- Pause/resume/cancel with automatic cleanup — no leftover files
- Time-range cropping — download only the exact section you need, with live progress
- Automatic playlist support
- Dark/light theme with smooth transitions
- Self-updating download engine
- Full Unicode/Cyrillic filename support
- Custom output filenames

## Tech stack

- C++17
- Qt6 (Widgets)
- CMake
- Built on top of the industry-standard [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [ffmpeg](https://ffmpeg.org/)

## Requirements / how to build

On Windows, with [Qt 6](https://www.qt.io/download-qt-installer) and [CMake](https://cmake.org/) installed:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/<version>/msvc2019_64
cmake --build build --config Release
```

The app targets C++17, Qt6 Widgets, and the MSVC toolchain.

## Tests

Unit and integration tests can be enabled with:

```powershell
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Release
```

## Credits

Built on the shoulders of [yt-dlp](https://github.com/yt-dlp/yt-dlp) and [ffmpeg](https://ffmpeg.org/).