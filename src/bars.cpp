#include "bars.h"

// данные от датчика
uint16_t measures[MAX_SAMPLES];
uint16_t measure_min = 0;
uint16_t measure_max = 0;
// данные для графика
uint8_t samples[MAX_SAMPLES];
// какие данные рисовать на графике
uint8_t index = 0;
uint16_t co2_max = 0;

void init_measures()
{
    index = 0;
    for (uint8_t i = 0; i < MAX_SAMPLES; i++)
    {
        measures[i] = 0;
        measures[i] = 0;
        samples[i] = 0;
    }
}

void add_new_measure(uint16_t measure_co2)
{
    if (measure_co2 > 9999)
    {
        measures[index] = 9999;
    }
    else
    {
        measures[index] = measure_co2;
    }
    index = (index + 1) % MAX_SAMPLES;
    normalize_samples();
}

void normalize_samples()
{
    uint16_t mn = measures[0];
    uint16_t mx = measures[1];
    uint16_t H = 60;
    for (uint8_t i = 0; i < MAX_SAMPLES; i++)
    {
        uint16_t a = measures[i];
        if (a > 0)
        {
            mx = (a > mx) ? a : mx;
            mn = (a < mn) ? a : mn;
        }
    }

    mx = ((mx / 100) + 1) * 100;

    measure_min = mn;
    measure_max = mx;

    float k = 0;
    if ((mx - mn) < 10)
    {
        k = float(H) / float(mx - mn + 20);
    }
    else
    {
        k = float(H) / float(mx - mn);
    }

    uint8_t sample_index = 0;
    for (uint8_t i = index; i < MAX_SAMPLES; i++)
    {
        if (measures[i] == 0)
        {
            samples[sample_index] = 0;
            sample_index++;
            continue;
        }
        uint8_t x = (uint8_t)(float((measures[i] - mn) * k));
        samples[sample_index] = (x == 0) ? 1 : x;
        sample_index++;
    }

    for (uint8_t i = 0; i < index; i++)
    {
        if (measures[i] == 0)
        {
            samples[sample_index] = 0;
            sample_index++;
            continue;
        }
        uint8_t x = (uint8_t)(float((measures[i] - mn) * k));
        samples[sample_index] = (x == 0) ? 1 : x;
        sample_index++;
    }
}

uint16_t fake_measure(uint8_t sensor_type)
{
    if (sensor_type == 0)
    {
        return random(400, 600);
    }
    if (sensor_type == 1)
    {
        return random(0, 100);
    }
    return 0;
}