# Архитектура SuperRack Linux

## Обзор

Приложение состоит из независимых слоёв с чётким разделением ответственности.

```
┌─────────────────────────────────────────────────────────┐
│                      UI Layer (JUCE)                     │
│  Overview, Rack View, Plugin Browser, Routing, MIDI     │
└──────────────────────┬──────────────────────────────────┘
                       │ commands / state
┌──────────────────────▼──────────────────────────────────┐
│                 Application Layer                         │
│  Main component, window management, command handling     │
└──────┬─────────────┬──────────────┬──────────────────────┘
       │             │              │
┌──────▼──┐  ┌───────▼────┐ ┌──────▼──────────┐
│ Session │  │  Snapshot  │ │   MIDI          │
│ Manager │  │  Manager   │ │   Processor     │
└─────────┘  └────────────┘ └─────────────────┘
       │             │              │
┌──────▼─────────────▼──────────────▼──────────────────────┐
│                 Audio Engine (real-time)                  │
│  Audio processing, routing, metering, plugin host         │
│  AUDIO THREAD — no blocking, no malloc, no I/O            │
└──────┬──────────────┬──────────────┬──────────────────────┘
       │              │              │
┌──────▼──┐  ┌────────▼─────┐ ┌─────▼────────┐
│ Routing │  │   Rack        │ │ LV2 Plugin   │
│ Matrix  │  │   Processor   │ │ Host (Lilv)  │
└─────────┘  └──────────────┘ └──────────────┘
       │              │              │
┌──────▼──────────────▼──────────────▼──────────────────────┐
│              Audio Device Layer (ALSA/JACK/PW)            │
│       Behringer X32 USB detection and I/O                │
└──────────────────────────────────────────────────────────┘
```

## Thread Model

### Audio Thread (highest priority)
- Обработка буферов в реальном времени
- Запрещено: blocking, malloc/free, file I/O, lock, UI calls
- Взаимодействие с другими потоками — через lock-free очереди

### Message Thread (JUCE MessageManager)
- GUI обновления
- Пользовательский ввод
- Команды от MIDI

### Background Thread
- Загрузка плагинов (не в audio thread)
- Сохранение сессий
- Сканирование LV2 плагинов

## Каналы и маршрутизация

### Физические входы X32 (до 32)
```
X32 Input 1 ──► Routing Input 1 ──► Rack 1 ──► Routing Output 1 ──► X32 Output 1
X32 Input 2 ──► Routing Input 2 ──► Rack 2 ──► Routing Output 2 ──► X32 Output 2
...
X32 Input 32 ──► Routing Input 32 ──► Rack 32 ──► Routing Output 32 ──► X32 Output 32
```

### Routing Matrix
- 32 физических входа → 32 виртуальных канала
- 32 виртуальных канала → 32 рэка (или стерео-пары)
- 32 рэка → 32 физических выхода

## Rack Processor

Каждый рэк — цепочка плагинов:
```
Input Gain → [Plugin 1] → [Plugin 2] → ... → [Plugin N] → Output Gain → Output
    ↓              ↓            ↓                        ↓
  Meter          Bypass      Bypass                   Meter
```

## Поток данных

1. `AudioDeviceManager` получает буферы от ALSA/JACK
2. `AudioEngine::audioCallback()` копирует в входные буферы
3. `RoutingMatrix` распределяет входы по каналам
4. Каждый `RackProcessor` обрабатывает свой канал
5. `RoutingMatrix` собирает выходы
6. `Metering` собирает peak/RMS данные
7. Данные передаются в UI через атомарные переменные

## Потокобезопасность

- Audio thread НЕ использует mutex
- GUI thread обновляется из audio thread через атомарные переменные и timed callbacks
- Plugin loading/unloading — только в background thread
- Snapshot применение — в audio thread через lock-free queue

## Ключевые классы

| Класс | Файл | Назначение |
|---|---|---|
| `AudioEngine` | `src/audio/AudioEngine.cpp` | Главный обработчик аудио |
| `DeviceManager` | `src/audio/DeviceManager.cpp` | Управление X32 |
| `RoutingMatrix` | `src/routing/RoutingMatrix.cpp` | Маршрутизация сигналов |
| `RackProcessor` | `src/racks/RackProcessor.cpp` | Цепочка плагинов одного канала |
| `LV2PluginHost` | `src/plugins/LV2PluginHost.cpp` | Загрузка и управление LV2 |
| `MIDIProcessor` | `src/midi/MIDIProcessor.cpp` | MIDI CC/PC обработка |
| `SessionManager` | `src/session/SessionManager.cpp` | Сохранение/загрузка сессий |
| `SnapshotManager` | `src/snapshots/SnapshotManager.cpp` | Snapshots + Recall Safe |

## Формат данных

Сессия — JSON (nlohmann/json):
```json
{
  "schemaVersion": 1,
  "session": { "name": "...", "sampleRate": 48000, "bufferSize": 64 },
  "racks": [...],
  "routing": { ... },
  "groups": [...],
  "snapshots": [...],
  "midiMappings": [...]
}
```

Все ID — стабильные UUID. Файл сохраняется атомарно (временный файл + rename).