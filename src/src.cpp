#include "SparkFun_SGP30_Arduino_Library.h"
// Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "everytime.h"

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "bars.h"

SGP30 mySensor; // create an object of the SGP30 class

bool sgp30_connected = false;

uint16_t current_co2 = 0;
uint16_t current_tvoc = 0;

/// @brief Read measures. Call every 1 second (exact)
void update_current_measures()
{
    if (sgp30_connected)
    {
        mySensor.measureAirQuality();
        current_co2 = mySensor.CO2;
        current_tvoc = mySensor.TVOC;
    }
    else
    {
        current_co2 = fake_measure(0);
        current_tvoc = fake_measure(1);
    }
}

void setup()
{
    delay(100);
    Serial.begin(9600);
    Wire.begin();

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();

    // Initialize sensor
    if (mySensor.begin() == false)
    {
        display.clearDisplay();
        display.setCursor(10, 10); // oled display
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.print(F("NO SGP30"));
        display.display();
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
        update_current_measures();
        co2_max = current_co2;
    }
    else
    {
        co2_max = fake_measure(0);
    }

    init_measures();
}

void print_big_measures()
{
    display.setCursor(0, 10); // oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("CO2:");
    display.print(current_co2);
    display.setTextSize(1);
    display.print("ppm");

    display.setCursor(0, 40); // oled display
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("TVOC:");
    display.print(current_tvoc);
    display.setTextSize(1);
    display.print("ppb");
}

void print_small_measures()
{
    display.setTextSize(1);
    display.setCursor(102, 3); // oled display
    display.setTextColor(BLACK, WHITE);
    display.print("CO2e");
    display.setTextColor(WHITE, BLACK);
    display.setCursor(102, 3 + 9); // oled display
    display.print(current_co2);
    display.setCursor(102, 3 + 9 + 9); // oled display
    display.print("ppm");

    display.setCursor(0, 40);   // oled display
    display.setCursor(102, 34); // oled display
    display.setTextColor(WHITE, BLACK);
    display.print("TVOC");
    display.setCursor(102, 34 + 9); // oled display
    display.print(current_tvoc);
    display.setCursor(102, 34 + 9 + 9); // oled display
    display.print("ppb");
}

void draw_bars()
{
    uint16_t h = display.height() - 1;
    for (uint8_t i = 0; i < MAX_SAMPLES; i++)
    {
        if (samples[i] > 0)
        {
            uint16_t x = i * 2;
            uint16_t y = h - samples[i];
            display.drawLine(x, y, x, h, SSD1306_WHITE);
        }
    }
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(2, 2); // oled display
    display.print(measure_max);
    display.setCursor(2, display.height() - 1 - 9); // oled display
    display.print(measure_min);

    display.drawRect(0, 0, 101, display.height(), SSD1306_WHITE);
    display.drawRect(100, 0, 27, display.height(), SSD1306_WHITE);
}

void loop()
{
    EVERY_N_SECONDS(1)
    {
        // First fifteen readings will be
        // CO2: 400 ppm  TVOC: 0 ppb
        update_current_measures();
        Serial.print("CO2: ");
        Serial.print(current_co2);
        Serial.print(" ppm\tTVOC: ");
        Serial.print(current_tvoc);
        Serial.println(" ppb");
        print_small_measures();
        display.display();
        co2_max = (current_co2 > co2_max) ? current_co2 : co2_max;
    }

    EVERY_N_SECONDS(3)
    {
        add_new_measure(co2_max);
        co2_max = current_co2;
        //    add_new_measure(mySensor.CO2);
        display.clearDisplay();
        print_small_measures();
        draw_bars();
        display.display();
    }
}
