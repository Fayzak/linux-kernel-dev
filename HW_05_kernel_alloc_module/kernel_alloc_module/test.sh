#!/bin/bash
set -e

MODULE_NAME="kernel_alloc"

SYS_ALLOC="/sys/module/${MODULE_NAME}/parameters/alloc"
SYS_FREE="/sys/module/${MODULE_NAME}/parameters/free"
SYS_STATS="/sys/module/${MODULE_NAME}/parameters/stats"

get_last_alloc() {
    sudo dmesg | grep "kernel_alloc: alloc_set: allocated" | tail -n 1 | grep -oE '0x[0-9a-fA-F]+' | tail -n 1
}

print_stats() {
    echo -e "\n=== ТЕКУЩАЯ СТАТИСТИКА ==="
    sudo cat "$SYS_STATS"
    echo "============================"
}

echo "🚀 Запуск тестового сценария для ${MODULE_NAME}..."

# 1. Начальное состояние
echo -e "\n[1/5] Проверка начального состояния..."
print_stats

# 2. Выделение трех ОДИНАКОВЫХ блоков, чтобы они встали строго друг за другом
BLOCK_SIZE_BYTES=3145728 # 3072 * 1024

echo -e "\n[2/5] Выделение трех блоков по 3072 KB (чтобы они встали подряд)..."

echo "  -> Выделяем Блок 1..."
sudo sh -c "echo $BLOCK_SIZE_BYTES > $SYS_ALLOC"
sleep 0.2
PTR_1=$(get_last_alloc)
echo "     Успешно: $PTR_1"

echo "  -> Выделяем Блок 2 (будущая 'дыра')..."
sudo sh -c "echo $BLOCK_SIZE_BYTES > $SYS_ALLOC"
sleep 0.2
PTR_2=$(get_last_alloc)
echo "     Успешно: $PTR_2"

echo "  -> Выделяем Блок 3..."
sudo sh -c "echo $BLOCK_SIZE_BYTES > $SYS_ALLOC"
sleep 0.2
PTR_3=$(get_last_alloc)
echo "     Успешно: $PTR_3"

# 3. Проверка статистики после выделения
echo -e "\n[3/5] Проверка статистики после выделения..."
echo "  (Ожидается: Fragmentation: 0%, так как всё свободное место идет одним куском в конце)"
print_stats

# 4. Освобождение СРЕДНЕГО блока
echo -e "\n[4/5] Освобождение СРЕДНЕГО блока (Блок 2: $PTR_2)..."
echo "  (Это создаст 'дыру' в 3072 KB между Блоком 1 и Блоком 3)"
sudo sh -c "echo $PTR_2 > $SYS_FREE"
sleep 0.2

# 5. Проверка статистики с фрагментацией
echo -e "\n[5/5] Проверка статистики с фрагментацией..."
print_stats

# 6. Финальное освобождение
echo -e "\n[ФИНАЛ] Освобождение Блока 1 и Блока 3..."
sudo sh -c "echo $PTR_1 > $SYS_FREE"
sleep 0.2
sudo sh -c "echo $PTR_3 > $SYS_FREE"
sleep 0.2

echo -e "\n[ПРОВЕРКА] Статистика после полного освобождения..."
print_stats

echo -e "\n✅ Тест завершен!"