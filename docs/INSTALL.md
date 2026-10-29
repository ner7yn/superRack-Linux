# Установка зависимостей для SuperRack Linux

## Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y \
    cmake \
    g++ \
    git \
    libasound2-dev \
    liblv2-dev \
    liblilv-dev \
    libsuil-dev \
    pkg-config
```

## Fedora/RHEL

```bash
sudo dnf install \
    cmake \
    gcc-c++ \
    git \
    alsa-lib-devel \
    lv2-devel \
    lilv-devel \
    suil-devel \
    pkg-config
```

## Arch Linux

```bash
sudo pacman -S \
    cmake \
    gcc \
    git \
    alsa-lib \
    lv2 \
    lilv \
    suil
```

## Проверка установки

```bash
# Проверка CMake
cmake --version

# Проверка компилятора
g++ --version | head -1

# Проверка Lilv
pkg-config --modversion lilv-0

# Проверка ALSA
pkg-config --modversion alsa
```

## Если какие-то пакеты не найдены

```bash
# Альтернативные названия пакетов
sudo apt install -y liblilv-0-dev    # вместо liblilv-dev
sudo apt install -y lv2-dev          # иногда нужен lv2 SDK
```