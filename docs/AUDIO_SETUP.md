# Настройка аудио — SuperRack Linux

## Оборудование

- Behringer X32 (USB connected)
- Linux с ALSA/JACK/PipeWire

## X32 USB Audio

X32 предоставляет 32 входа / 32 выхода через USB.
Имя устройства ALSA: `X32`, `X32 Mixer`, или похожее.

### Проверка устройства

```bash
# ALSA
cat /proc/asound/cards
# Должно показать X32

# или
aplay -l
# или
pw-cli list-objects node
```

## Рекомендуемые настройки

### Частота дискретизации
- **48 кГц** (основная для X32)

### Размер буфера
- **64 сэмпла** — цель (задержка ~1.3 мс при 48 кГц)
- **128 сэмплов** — fallback при xruns
- **256 сэмплов** — для тестирования стабильности
- **512 сэмплов** — максимальный стабильный

### Формат
- 32-bit float внутри движка
- S32_LE на интерфейсе X32 (если доступно)

## PipeWire

Рекомендуется PipeWire вместо чистого JACK:
```bash
# PipeWire обычно работает "из коробки"
# Проверить:
pw-cli list-objects node
# Должен показать X32
```

## JACK (alternative)

```bash
# Запуск JACK с X32
jackd -d alsa -d X32 -r 48000 -p 64 -n 4 &
```

## ALSA (direct)

Для ALSA-только окружения:
```bash
# Проверить устройство
cat /proc/asound/cards
# Запустить с ALSA backend в JUCE
```

## Измерение задержки

### Round-trip задержка
```
Задержка = (buffer_size / sample_rate) * 2 + interface_latency

Пример (64 samples, 48kHz, X32):
Задержка = (64/48000)*2 + ~0.5ms ≈ 2.6ms + interface
```

### Проверка xruns
```bash
# Watch for xruns
# В приложении: индикатор xrun counter в status bar
```

## Troubleshooting

### X32 не определяется
1. Проверить кабель USB
2. Проверить `lsusb | grep -i x32`
3. Проверить права доступа `/dev/snd/*`
4. Добавить пользователя в `audio` group

### Xruns при 64 сэмплах
1. Увеличить буфер до 128
2. Проверить загрузку CPU (top/htop)
3. Отключить compositor (если desktop Linux)
4. Установить realtime priority для приложения

### High CPU load
1. Меньше плагинов на канал
2. Более лёгкие плагины
3. Проверить, что X32 работает в USB Class Compliant mode

## Права доступа

```bash
# Добавить пользователя в audio group
sudo usermod -aG audio $USER
# Перелогиниться
```

Для realtime priority (опционально):
```bash
# /etc/security/limits.d/audio.conf
@audio - rtprio 95
@audio - memlock unlimited
```