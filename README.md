# System Monitoring Agent

Проект написан на C++. Программа запускается как консольный агент и собирает метрики активности пользователя.

Агент:

- раз в 5 секунд проверяет активное окно;
- сохраняет имя процесса и заголовок окна;
- проверяет, была ли активность пользователя за последние 5 секунд;
- складывает данные во внутренний потокобезопасный буфер;
- отправляет накопленные данные каждые 30 секунд или при накоплении 10 записей;
- при ошибке отправки не удаляет данные и повторяет попытку позже;
- хранит в памяти не более 100 записей;
- при корректной остановке сохраняет оставшиеся неотправленные данные в `backup.json`.

## Архитектура

Диаграмма классов находится здесь:

[docs/architecture.puml](docs/architecture.puml)

Её можно открыть в VS Code через расширение PlantUML или через любой другой PlantUML viewer.

Основные компоненты проекта:

- `Agent` — главный класс агента. Запускает два потока: сбор метрик и отправку данных.
- `WindowsActivityProvider` — получает данные через Win32 API: активное окно, имя процесса, заголовок окна и факт активности пользователя.
- `MetricBuffer` — потокобезопасный буфер. В нём хранится максимум 100 записей.
- `JsonSerializer` — преобразует метрики в JSON нужного формата.
- `HttpClient` — отправляет JSON методом POST.

## Требования

Для сборки проекта нужны:

- Windows;
- CMake 3.20 или новее;
- компилятор с поддержкой C++20, например MSVC из Visual Studio Build Tools 2022;
- интернет при первом configure, потому что CMake скачивает `nlohmann/json` и `cpp-httplib`.

Проект рассчитан на Windows, так как сбор информации об активном окне реализован через Win32 API.

## Сборка

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

После сборки исполняемый файл будет находиться здесь:

```text
build/Debug/SystemMonitoringAgent.exe
```

Также можно использовать скрипт:

```powershell
.\scripts\run.cmd --version
```

Скрипт выполняет configure, build и запускает программу.

## Запуск

Обычный запуск:

```powershell
.\scripts\run.cmd
```

По умолчанию агент отправляет данные на:

```text
http://localhost:8080
```

Можно указать другой endpoint:

```powershell
.\scripts\run.cmd --endpoint http://localhost:8080
```

Дополнительные команды:

```powershell
.\scripts\run.cmd --help
.\scripts\run.cmd --version
```

Остановка выполняется через `Ctrl+C`.

При корректной остановке оставшиеся неотправленные метрики сохраняются в `backup.json` в текущем рабочем каталоге.

## Формат JSON

Агент отправляет POST-запрос с `Content-Type: application/json`.

Пример:

```json
{
  "agent_id": "DESKTOP-NAME",
  "timestamp": 1792147320,
  "payload": [
    {
      "time": "2026-09-15 13:55:00",
      "process_name": "chrome.exe",
      "window_title": "Some page",
      "user_active": true
    }
  ]
}
```

Если сервер недоступен или возвращает статус, отличный от `2xx`, данные возвращаются в буфер и будут отправлены позже.

## Тесты

Запуск всех тестов:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

В проекте используются:

- unit-тесты для `MetricBuffer` и `JsonSerializer`;
- smoke-тесты для `WindowsActivityProvider` и `HttpClient`;
- интеграционные тесты `Agent`, проверяющие отправку данных, повторную отправку после ошибки и сохранение `backup.json`.

Smoke-тест `WindowsActivityProvider` зависит от активного окна в текущей Windows-сессии, поэтому в некоторых окружениях он может завершиться неуспешно.

Для проверки основной логики без него:

```powershell
ctest --test-dir build -C Debug -E WindowsActivityProviderSmokeTest --output-on-failure
```

## Структура проекта

```text
include/     заголовочные файлы
src/         реализация агента
tests/       тесты
docs/        диаграмма архитектуры
scripts/     скрипты запуска
.vscode/     задачи сборки и конфигурация отладки VS Code
```
