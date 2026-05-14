#!/bin/bash

function disable() {
    echo -e "ОТКЛЮЧЕНИЕ модуля $1"
    ./scripts/config --disable "$1"
}

function enable() {
    echo -e "\nВКЛЮЧЕНИЕ модуля $1\n"
    ./scripts/config --enable "$1"
}

function makeCONFIG() {
    echo "Создание конфига для ядра 7.0.3..."
    rm -f .config
    # Используем базовую конфигурацию x86_64
    make x86_64_defconfig
    # ИЛИ make defconfig (для архитектуры по умолчанию)
}

### START ###
makeCONFIG

disable "SECURITY_SELINUX"
disable "SECURITY_SMACK"
disable "SECURITY_TOMOYO"
disable "SECURITY_APPARMOR"
disable "SECURITY_YAMA"
disable "RANDOMIZE_BASE"
disable "RANDOMIZE_KSTACK_OFFSET"
disable "RANDOMIZE_MEMORY"
disable "CPU_MITIGATIONS"
disable "MITIGATION_SPECTRE_BHI"
disable "MITIGATION_RFDS"
disable "PAGE_TABLE_ISOLATION"
disable "ZSWAP"
disable "ZRAM" # Аналог ZSWAP в 7.x версии ядра

enable "DEBUG_FS"
enable "FTRACE"
enable "FUNCTION_TRACER"
enable "DYNAMIC_FTRACE"
enable "FUNCTION_GRAPH_TRACER"
enable "STACK_TRACER"
enable "KUNIT"
enable "KUNIT_TEST"
enable "KASAN"
enable "STACKTRACE"
enable "KASAN_GENERIC"
enable "KASAN_INLINE"
enable "KASAN_EXTRA_INFO"
enable "KGDB"
enable "KGDB_SERIAL_CONSOLE"
enable "CONSOLE_POLL"
enable "KPROBES"
enable "KPROBE_EVENT"

enable "DEBUG_KMEMLEAK"
enable "DEBUG_STACK_USAGE"
enable "PROVE_LOCKING"

./scripts/config --set-val CONFIG_DEBUG_INFO y
./scripts/config --set-val CONFIG_DEBUG_INFO_NONE n
#./scripts/config --set-val CONFIG_DEBUG_INFO_DWARF4 y
./scripts/config --set-val CONFIG_DEBUG_INFO_DWARF5 y # В ядре 7.x лучше DWARF5 вместо DWARF4

make olddefconfig
make localmodconfig

echo -e "olddefconfig ok"

# Проверка конфига
echo -e "\nПроверка конфига:"
grep "CONFIG_DEBUG_INFO" .config
grep "CONFIG_DEBUG_INFO_NONE" .config
grep "CONFIG_DEBUG_INFO_DWARF5" .config

echo -e "\nПодготовка конфига завершена!"
echo "Запустите компиляцию командой:"
echo "time make -j$(nproc) bindeb-pkg"
