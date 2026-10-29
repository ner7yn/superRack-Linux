# Результаты тестов задержки и стабильности

## TODO: Заполнить после тестирования на Linux + X32

## Профили тестирования

### Профиль 1: 32 канала, без плагинов
- Sample rate: 48 кГц
- Buffer size: 64
- Плагины: none
- Цель: базовая нагрузка движка

### Профиль 2: 32 канала, 1 плагин на канал
- Sample rate: 48 кГц
- Buffer size: 64
- Плагины: 1 лёгкий EQ на канал (напр. lsp-plugins-para-eq)
- Цель: реалистичная нагрузка

### Профиль 3: 32 канала, 3 плагина на канал
- Sample rate: 48 кГц
- Buffer size: 64
- Плагины: EQ + Compressor + Limiter
- Цель: максимальная нагрузка

### Профиль 4: Stress test
- Sample rate: 48 кГц
- Buffer size: 64
- Длительность: 60 минут
- Цель: стабильность

## Метрики

| Метрика | Описание | Целевое значение |
|---|---|---|
| CPU avg | Средняя загрузка CPU за период | < 30% |
| CPU peak | Пиковая загрузка CPU | < 80% |
| Xruns | Количество за период | 0 |
| Latency | Round-trip задержка | 3-5 мс (цель) |
| Snapshot transition | Время переключения | < buffer period |
| MIDI latency | Задержка MIDI → действие | < 5 мс |

## Формат отчёта

```json
{
  "profile": "32ch-1plugin",
  "date": "2025-01-01",
  "system": "...",
  "sampleRate": 48000,
  "bufferSize": 64,
  "results": {
    "cpu": { "avg": 15.2, "peak": 38.1, "unit": "%" },
    "xruns": { "count": 0, "perHour": 0 },
    "latency": { "measured": 3.2, "unit": "ms" },
    "memory": { "rss": 120, "unit": "MB" }
  }
}
```