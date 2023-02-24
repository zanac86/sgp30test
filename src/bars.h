#ifndef BARS_H
#define BARS_H

#include <Arduino.h>
#include <stdint.h>

#define MAX_SAMPLES 50

// данные от датчика
extern uint16_t measures[MAX_SAMPLES];
extern uint16_t measure_min;
extern uint16_t measure_max;
// данные для графика
extern uint8_t samples[MAX_SAMPLES];
// какие данные рисовать на графике
extern uint8_t index;
extern uint16_t co2_max;

void init_measures();
void add_new_measure(uint16_t measure_co2);
void normalize_samples();

uint16_t fake_measure(uint8_t sensor_type=0);

#endif
