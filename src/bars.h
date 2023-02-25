#ifndef BARS_H
#define BARS_H

#include <Arduino.h>
#include <stdint.h>

// на экране 50 отсчетов через пиксель - всего 100 пикселей
// отсчеты через 60 секунд, всего 50 минут
#define TOTAL_MEASURES 50

struct SensorsMeasures
{
    // позиция записи в data
    uint16_t data_index;
    // минимум в data
    uint16_t min;
    // максимум в data
    uint16_t max;

    // текущее значение
    uint16_t current;

    // для входных значений, они потом усредняются
    uint16_t input;
    uint16_t reserved1;
    // измерения от датчика
    uint16_t data[TOTAL_MEASURES];
};

#define _CO2E 0
#define _TVOC 1

// Данные измерений для главного модуля
extern SensorsMeasures measures[2];
// данные для графика
extern uint8_t samples[TOTAL_MEASURES];

// инициализация массивов и переменных
void init_measures();

// добавление в input новых отсчетов
// если прошли полностью, то вычислить среднее
// и записать в data
void add_new_measure(uint16_t measure_co2, uint16_t measure_tvoc);

// обновить данные в массиве для отображения
void update_measure_data();

// подготовить отсчеты для графика
void prepare_display_samples(uint8_t x);

// fake random values
uint16_t fake_measure(uint8_t sensor_type);

#endif
