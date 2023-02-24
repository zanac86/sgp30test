#include "bars.h"

SensorsMeasures measures[2];

// данные для графика
uint8_t samples[MAX_MEASURES];

void init_measures()
{
    for (uint8_t x = 0; x < 2; x++)
    {
        for (uint8_t i = 0; i < MAX_MEASURES; i++)
        {
            measures[x].data[i] = 0;
        }
        measures[x].input = 0;
        measures[x].data_index = 0;
        measures[x].min = 0;
        measures[x].max = 0;
        measures[x].current = 0;
    }

    for (uint8_t i = 0; i < MAX_MEASURES; i++)
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
        measures[x].data_index = (i + 1) % MAX_MEASURES;
    }
}

void add_new_measure(uint16_t measure_co2, uint16_t measure_tvoc)
{
    measures[_CO2E].current = (measure_co2 > 9999) ? 9999 : measure_co2;
    measures[_TVOC].current = (measure_tvoc > 9999) ? 9999 : measure_tvoc;
    // uint16_t c = (measures[_CO2E].input >> 1) + (measures[_CO2E].current >> 1);
    // uint16_t t = (measures[_TVOC].input >> 1) + (measures[_TVOC].current >> 1);

    uint16_t c = measure_co2;
    uint16_t t = measure_tvoc;

    measures[_CO2E].input = (c > 9999) ? 9999 : c;
    measures[_TVOC].input = (t > 9999) ? 9999 : t;
}

void prepare_display_samples(uint8_t x)
{
    uint16_t mn = measures[x].data[0];
    uint16_t mx = measures[x].data[0];
    uint16_t H = 60;
    for (uint8_t i = 0; i < MAX_MEASURES; i++)
    {
        uint16_t a = measures[x].data[i];
        if (a > 0)
        {
            mx = (a > mx) ? a : mx;
            mn = (a < mn) ? a : mn;
        }
    }

    mx = ((mx / 100) + 1) * 100;
    if (mn < 500)
    {
        mn = 400;
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

    for (uint8_t i = measures[x].data_index; i < MAX_MEASURES; i++)
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
        return random(400, 600);
    }
    if (sensor_type == _TVOC)
    {
        return random(0, 100);
    }
    return 0;
}