# План разработки SuperRack Linux

## Статус: Этап 0 — Инициализация проекта

## Этапы

### Этап 0: Скелет проекта (текущий)
**Цель:** Создать структуру, CMakeLists.txt, базовые документы.

**Что сделано:**
- Структура директорий `src/{app,audio,routing,racks,plugins,session,snapshots,midi,groups,metering,ui}`
- `CMakeLists.txt` с JUCE, Lilv, nlohmann/json
- `docs/ARCHITECTURE.md`
- `docs/DEVELOPMENT_PLAN.md` (этот файл)
- `docs/AUDIO_SETUP.md` (TODO: после проверки X32)
- `docs/LATENCY_TESTS.md` (TODO: после тестов)

**Следующий шаг:** Создать минимальный Main.cpp и App.cpp, проверить сборку на Linux.

---

### Этап 1: Минимальный аудиопрототип
**Цель:** Приложение запускается, обнаруживает X32, передаёт аудио.

**Задачи:**
1. `Main.cpp` + `App.cpp` — минимальное JUCE приложение
2. `AudioEngine` — базовый JUCE audio callback
3. 32 канала без обработки
4. Проверка сборки на Linux
5. Проверка обнаружения X32

**Критерий завершения:** Реальный звук проходит через app без xruns.

---

### Этап 2: Audio Engine + Routing
**Цель:** Gain, mute, bypass, routing, metering.

**Задачи:**
1. `RoutingMatrix` — маршрутизация входов на каналы
2. `DeviceManager` — обнаружение X32, настройка SR/buffer
3. Gain, mute, bypass для каждого канала
4. Peak metering (атомарные переменные для GUI)
5. Xrun counter
6. Graceful recovery при disconnect X32

**Критерий завершения:** 32 канала с реальным routing и meters.

---

### Этап 3: LV2 Host (Lilv)
**Цель:** Загрузка, обработка, GUI плагинов.

**Задачи:**
1. `LV2PluginHost` — сканирование плагинов через Lilv
2. Загрузка плагина (фоновый thread)
3. Обработка в audio callback (pre-allocated)
4. Открытие GUI плагина (JUCE wrapper)
5. Управление параметрами
6. Bypass плагина
7. Сохранение/восстановление state
8. Обработка отсутствующего плагина (не падать)

**Критерий завершения:** EQ + Compressor + Limiter загружаются и обрабатывают звук.

---

### Этап 4: Интерфейс
**Цель:** Overview, Rack View, Plugin Browser.

**Задачи:**
1. `MainComponent` сJUCE
2. Overview — 32 рэка в виде сетки
3. Rack View — детальный вид одного рэка
4. Plugin Browser — список LV2 плагинов
5. Gain/Mute/Bypass controls
6. Input/Output meters (peak)
7. Routing window
8. Status bar (CPU, xruns, device)
9. Группы — цветовая маркировка

**Критерий завершения:** UI управляет реальным audio engine.

---

### Этап 5: Сессии, Snapshots, MIDI, Presets
**Цель:** Сохранение, восстановление, MIDI-управление.

**Задачи:**
1. `SessionManager` — JSON сессия (atomic save)
2. `SnapshotManager` — snapshot с Recall Safe
3. Плавный переход между snapshots (ramp)
4. `MIDIProcessor` — CC и PC
5. MIDI Learn
6. Rack presets
7. Plugin state presets

**Критерий завершения:** Всё восстанавливается после перезапуска, MIDI работает.

---

### Этап 6: Стабилизация
**Цель:** Тесты, оптимизация, документация.

**Задачи:**
1. Тест 32 канала без плагинов
2. Тест 32 канала с одним плагином
3. Длительный прогон 60+
4. Измерение задержки
5. Обработка disconnect X32
6. Thread safety проверка
7. `docs/AUDIO_SETUP.md` — руководство по настройке
8. `docs/LATENCY_TESTS.md` — результаты тестов

**Критерий завершения:** Стабильная работа без xruns.

---

## График рисков

| Риск | Вероятность | Влияние | Mitigation |
|---|---|---|---|
| JUCE LV2 хостинг ограничен | Средняя | Высокое | Lilv напрямую, не JUCE wrapper |
| X32 не определяется в ALSA | Низкая | Высокое | JACK fallback, проверить на реальном X32 |
| Много плагинов = CPU spike | Средняя | Среднее | Background loading, pre-allocated buffers |
| Snapshot переход = click | Средняя | Среднее | Interpolation + crossfade в audio thread |

## Версии ПО

| Компонент | Версия | Примечание |
|---|---|---|
| JUCE | 8.x (latest) | Module-based, CMake |
| Lilv | 0.24+ | LV2 host |
| nlohmann/json | 3.11.3 | JSON serialization |
| CMake | 3.20+ | Build system |
| C++ | 20 | Standard |
| GCC/Clang | any C++20 | Linux toolchain |