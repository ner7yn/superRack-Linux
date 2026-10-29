# SuperRack Linux

Linux-аналог Waves SuperRack Performer для живой обработки звука с Behringer X32 по USB.

**Статус:** MVP — базовая функциональность реализована, тесты проходят.

## Возможности (MVP)

- 32 моно-канала с цепочками эффектов
- Маршрутизация входов/выходов через Behringer X32 USB
- Metering (peak, RMS, clipping detection)
- Mute/Bypass для каждого рэка
- Snapshots с Recall Safe
- MIDI Control Change и Program Change
- Группировка рэков
- LV2-плагины (Lilv host)
- Сохранение/загрузка сессий в JSON

## Требования

### Система
- Linux (x86-64)
- CMake 3.20+
- GCC 10+ или Clang 12+ (C++20)

### Зависимости
- **JUCE 8.x** — Audio processing + GUI
- **Lilv 0.24+** — LV2 plugin host
- **ALSA / JACK / PipeWire** — Audio backend
- **nlohmann/json** — Session storage

### Оборудование
- Behringer X32 (USB)
- 32-channel audio interface (совместимый с ALSA/JACK)

## Сборка

```bash
# 1. Клонировать JUCE
git clone --recurse-submodules https://github.com/juce-framework/JUCE.git
cd JUCE && git checkout 8.0.4 && cd ..

# 2. Настроить и собрать
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DJUCE_DIR=$PWD/../JUCE \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra"

# 3. Собрать
cmake --build . --config Release -j$(nproc)

# 4. Запустить
./superRackLinux

# 5. Тесты
cd tests
./superRackTests
```

## Структура проекта

```
superRack-Linux/
├── CMakeLists.txt           # Главный CMake
├── docs/
│   ├── ARCHITECTURE.md      # Архитектура
│   ├── DEVELOPMENT_PLAN.md  # План разработки
│   ├── AUDIO_SETUP.md       # Настройка X32
│   └── LATENCY_TESTS.md     # Тесты задержки
├── src/
│   ├── app/                 # Main, App
│   ├── audio/               # AudioEngine, DeviceManager
│   ├── routing/             # RoutingMatrix
│   ├── racks/               # RackProcessor
│   ├── plugins/             # LV2PluginHost (Lilv)
│   ├── session/             # SessionManager
│   ├── snapshots/           # SnapshotManager
│   ├── midi/                # MIDIProcessor
│   ├── groups/              # GroupManager
│   ├── metering/            # Metering
│   ├── ui/                  # JUCE GUI
│   └── common/              # Types, Utils
└── tests/                   # Catch2 тесты
```

## Архитектура

Проект разделён на независимые модули:
- **Audio Engine** — real-time обработка, без malloc, без блокировок
- **Routing Matrix** — маршрутизация 32 каналов (mono/stereo)
- **Rack Processor** — цепочка плагинов с gain/mute/bypass
- **LV2 Plugin Host** — загрузка плагинов через Lilv
- **Snapshot Manager** — сохранение/восстановление состояний
- **MIDI Processor** — CC/PC команды
- **Group Manager** — групповое управление
- **Metering** — пиковые/RMS индикаторы + clipping

## Критерии MVP

1. ✅ Сборка из исходного кода
2. ✅ 32 канала обработки
3. ✅ Routing matrix
4. ✅ Metering (peak, RMS, clipping)
5. ✅ Gain/Mute/Bypass
6. ✅ LV2 плагин хост (Lilv)
7. ✅ Snapshots + Recall Safe
8. ✅ MIDI PC/CC
9. ✅ Группы рэков
10. ✅ Session save/load (JSON)
11. ❌ Измерение задержки (требуется X32 + Linux)
12. ❌ Стабильные тесты с реальным аудио

## Известные ограничения

1. **LV2 GUI** — не реализовано в MVP (требуется дополнительная интеграция)
2. **X32 тестирование** — требуется реальное оборудование
3. **Задержка** — целевая 3-5 мс, требует измерения
4. **Stereo routing** — архитектурно заложен, но не полностью протестирован
5. **JUCE LV2 хост** — ограниченная поддержка, используется Lilv напрямую

## Разработка

См. `docs/DEVELOPMENT_PLAN.md` для деталей реализации.

## Лицензия

MIT (по умолчанию, укажите при необходимости).