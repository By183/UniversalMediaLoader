<div align="center">

**[🇷🇺 Русский](README.ru.md)** · **[🇬🇧 English](https://github.com/By183/UniversalMediaLoader/blob/main/README.md)**

---

# Universal Media Loader 🎬

📥 Красивое, быстрое нативное десктоп-приложение для скачивания видео и аудио с YouTube, Twitch и TikTok.

</div>

**Universal Media Loader** — полнофункциональный, готовый к использованию медиа-загрузчик. Он загружает видео и стримы с **YouTube**, **Twitch** и **TikTok**, а также извлекает аудио в MP3 — с современным нативным Qt-интерфейсом, быстрым запуском и малым потреблением памяти. Всё необходимое упаковано в один автономный файл.

## ⬇️ Скачать

Скачайте архив **UniversalMediaLoader-v2.0.0-win64.zip**, распакуйте его в любую папку и запустите **UniversalMediaLoader.exe** — установка не требуется, всё необходимое (Qt-рантайм, плагины, yt-dlp) уже находится в папке.

👉 [**⬇️ Скачать UniversalMediaLoader-v2.0.0-win64.zip**](https://github.com/By183/UniversalMediaLoader/releases/latest/download/UniversalMediaLoader-v2.0.0-win64.zip)

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
- Построено на отраслевых стандартах [yt-dlp](https://github.com/yt-dlp/yt-dlp) и [ffmpeg](https://ffmpeg.org/)

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