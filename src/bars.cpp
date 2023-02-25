#include "bars.h"

SensorsMeasures measures[2];

// данные для графика
uint8_t samples[TOTAL_MEASURES];

void init_measures()
{
    for (uint8_t x = 0; x < 2; x++)
    {
        for (uint8_t i = 0; i < TOTAL_MEASURES; i++)
        {
            measures[x].data[i] = 0;
        }
        measures[x].input = 0;
        measures[x].data_index = 0;
        measures[x].min = 0;
        measures[x].max = 0;
        measures[x].current = 0;
    }

    for (uint8_t i = 0; i < TOTAL_MEASURES; i++)
    {
        samples[i] = 0;
    }
}

void update_measure_data()
{
    for (uint8_t x = 0; x < 2; x++)
    {
        uint8_t i = measures[x].data_index;
        measures[x].data[i] = measures[x].input;
        measures[x].data_index = (i + 1) % TOTAL_MEASURES;
    }
}

void add_new_measure(uint16_t measure_co2, uint16_t measure_tvoc)
{
    // от датчика co2e должны быть >=400
    uint16_t c = (measure_co2 < 400) ? 400 : measure_co2;
    // от датчика tvoc могут быть 0 и больше, но 0 выкидываем
    uint16_t t = (measure_tvoc == 0) ? 1 : measure_tvoc;

    c = (c > 9999) ? 9999 : c;
    t = (t > 9999) ? 9999 : t;

    measures[_CO2E].current = c;
    measures[_TVOC].current = t;

    measures[_CO2E].input = (measures[_CO2E].input >> 1) + (c >> 1);
    measures[_TVOC].input = (measures[_TVOC].input >> 1) + (t >> 1);

    // measures[_CO2E].input = (c > 9999) ? 9999 : c;
    // measures[_TVOC].input = (t > 9999) ? 9999 : t;
}

void prepare_display_samples(uint8_t x)
{
    uint16_t mn = measures[x].data[0];
    uint16_t mx = measures[x].data[0];
    uint16_t H = 60;
    for (uint8_t i = 0; i < TOTAL_MEASURES; i++)
    {
        uint16_t a = measures[x].data[i];
        if (a > 0)
        {
            mx = (a > mx) ? a : mx;
            mn = (a < mn) ? a : mn;
        }
    }
    if (x == _CO2E)
    {
        // в co2 нет значений < 400

        mn = (mn < 600) ? 400 : mn;
        // окгруление до большей сотни
        mx = ((mx / 100) + 1) * 100;
    }

    if (x == _TVOC)
    {
        // окгруление до большей сотни
        mx = ((mx / 100) + 1) * 100;
        mn = (mn < 200) ? 0 : 100;
    }

    measures[x].min = mn;
    measures[x].max = mx;

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

    for (uint8_t i = measures[x].data_index; i < TOTAL_MEASURES; i++)
    {
        if (measures[x].data[i] == 0)
        {
            samples[sample_index] = 0;
        }
        else
        {
            uint8_t v = (uint8_t)(float((measures[x].data[i] - mn) * k));
            samples[sample_index] = (v == 0) ? 1 : v;
        }
        sample_index++;
    }
    for (uint8_t i = 0; i < measures[x].data_index; i++)
    {
        if (measures[x].data[i] == 0)
        {
            samples[sample_index] = 0;
        }
        else
        {
            uint8_t v = (uint8_t)(float((measures[x].data[i] - mn) * k));
            samples[sample_index] = (v == 0) ? 1 : v;
        }
        sample_index++;
    }
}

uint16_t fake_measure(uint8_t sensor_type)
{
    if (sensor_type == _CO2E)
    {
        return random(400, 1600);
    }
    if (sensor_type == _TVOC)
    {
        return random(0, 1200);
    }
    return 0;
}