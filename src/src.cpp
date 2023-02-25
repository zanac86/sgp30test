#include <SparkFun_SGP30_Arduino_Library.h>
// Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>
#include "everytime.h"
#include "bars.h"

// дефайн перед подключением либы - использовать microWire (лёгкая либа для I2C)
// #define USE_MICRO_WIRE
#include <GyverOLED.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

SGP30 mySensor; // create an object of the SGP30 class

// если нашелся датчик
bool sgp30_connected = false;

// Тип выводимых данных
uint8_t display_sensor_type = _CO2E;

uint8_t show_count = 0;

#define DISPLAY_PLOT 0
#define DISPLAY_SENS 1
uint8_t display_mode = DISPLAY_PLOT;

// Read measures. Call every 1 second (exact)
void update_current_measures_every_1s()
{
    if (sgp30_connected)
    {
        mySensor.measureAirQuality();
        add_new_measure(mySensor.CO2, mySensor.TVOC);
    }
    else
    {
        add_new_measure(fake_measure(_CO2E), fake_measure(_TVOC));
    }
}

void setup()
{
    delay(100);
    Serial.begin(9600);
    Wire.begin();

    oled.init();
    oled.clear();
    oled.update();

    init_measures();

    // Initialize sensor
    if (mySensor.begin() == false)
    {
        oled.clear();
        oled.setCursor(10, 10); // oled display
        oled.setScale(2);
        oled.print(F("NO SGP30"));
        oled.update();
        Serial.println(F("No SGP30 Detected. Check connections."));
        sgp30_connected = false;
        delay(1000);
    }
    else
    {
        sgp30_connected = true;
    }
    // Initializes sensor for air quality readings
    // measureAirQuality should be called in one second increments after a call to initAirQuality
    if (sgp30_connected)
    {
        mySensor.initAirQuality();
        delay(1000); // Wait 1 second
    }

    update_current_measures_every_1s();
}

void print_big_measures()
{
    char s[6];
    oled.setCursorXY(0, 10); // oled display
    oled.setScale(2);
    oled.invertText(false);
    oled.print("CO2:");
    sprintf(s, "%4u", measures[_CO2E].current);
    oled.print(s);
    oled.setScale(1);
    oled.print("ppm");

    oled.setCursorXY(0, 40); // oled display
    oled.setScale(2);
    oled.invertText(false);
    oled.print("TVOC:");
    sprintf(s, "%4u", measures[_TVOC].current);
    oled.print(s);
    oled.setScale(1);
    oled.print("ppb");
}

void print_small_measures()
{
    int x = 104;
    char s[6];
    oled.setScale(1);
    oled.setCursorXY(x, 0); // oled display
    oled.invertText(display_sensor_type == _CO2E);
    oled.print("CO2e");
    oled.invertText(false);
    oled.setCursorXY(x, 0 + 9); // oled display
    sprintf(s, "%4u", measures[_CO2E].current);
    oled.print(s);
    oled.setCursorXY(x, 0 + 9 + 9); // oled display
    oled.print("ppm");

    oled.setCursorXY(x, 32); // oled display
    oled.invertText(display_sensor_type == _TVOC);
    oled.print("TVOC");
    oled.invertText(false);
    oled.setCursorXY(x, 32 + 9); // oled display
    sprintf(s, "%4u", measures[_TVOC].current);
    oled.print(s);
    oled.setCursorXY(x, 32 + 9 + 9); // oled display
    oled.print("ppb");
}

void draw_bars()
{
    uint16_t h = 64 - 1;
    for (uint8_t i = 0; i < TOTAL_MEASURES; i++)
    {
        if (samples[i] > 0)
        {
            uint16_t x = i * 2;
            uint16_t y = h - samples[i];
            oled.fastLineV(x, y, h);
        }
    }
    oled.rect(0, 0, 100, 63, OLED_STROKE);

    oled.setScale(1);
    oled.invertText(false);
    oled.setCursorXY(0, 0); // oled display
    oled.print(measures[display_sensor_type].max);
    oled.setCursorXY(0, 64 - 8); // oled display
    oled.print(measures[display_sensor_type].min);

    // oled.rect(100, 0, 127, 63, OLED_STROKE);
}

void loop()
{
    EVERY_N_SECONDS(1)
    {
        // First fifteen readings will be
        // CO2: 400 ppm  TVOC: 0 ppb
        update_current_measures_every_1s();
        if (display_mode == DISPLAY_PLOT)
        {
            print_small_measures();
        }
        if (display_mode == DISPLAY_SENS)
        {
            oled.clear();
            print_big_measures();
            oled.update();
        }
        oled.update();
    }

    EVERY_N_SECONDS(10)
    {
        display_mode = (display_mode == DISPLAY_PLOT) ? DISPLAY_SENS : DISPLAY_PLOT;
    }

    EVERY_N_SECONDS(3)
    {
        if (show_count == 4)
        {
            display_sensor_type = (display_sensor_type == _CO2E) ? _TVOC : _CO2E;
            show_count = 0;
        }
        else
        {
            show_count++;
        }

        if (display_mode == DISPLAY_PLOT)
        {
            update_measure_data();
            prepare_display_samples(display_sensor_type);
            oled.clear();
            print_small_measures();
            draw_bars();
            oled.update();
        }
    }

}
