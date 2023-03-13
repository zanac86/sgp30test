// https://github.com/sparkfun/SparkFun_SGP30_Arduino_Library
#include <SparkFun_SGP30_Arduino_Library.h>

#include <Wire.h>
#include "everytime.h"
#include "bars.h"

// https://github.com/GyverLibs/GyverOLED
// дефайн перед подключением либы - использовать microWire (лёгкая либа для I2C)
//  #define USE_MICRO_WIRE
#include <GyverOLED.h>

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

SGP30 mySensor; // create an object of the SGP30 class

// если нашелся датчик
bool sgp30_connected = false;

uint8_t display_sensor_type = _CO2E;
// Тип выводимых данных

// Нога для пищалки
int buzzer = 10;

// признак мерцания надписи, если уровень превышен
uint8_t blink = 0;
uint8_t alarm = 0;
uint8_t alarm_active = 0;
// следующий бзз будет через 10 минут
#define ALARM_DELAY_MS 600000
uint32_t alarm_time_delay_ms = 0;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) ///< Generic macro for obtaining number of elements of an array

// Вывод графика co2, tvoc или крупных цифр
#define DISPLAY_PLOT_CO2 0
#define DISPLAY_PLOT_TVOC 1

// список режимов по порядку
uint8_t display_mode_list[] =
{
    DISPLAY_PLOT_CO2,
    DISPLAY_PLOT_TVOC,
};

// индекс текущего режима в списке
uint8_t display_mode_index = 0;

// текущий режим
uint8_t display_mode = DISPLAY_PLOT_CO2;

// количество режимов отображения
const uint8_t display_mode_count = ARRAY_SIZE(display_mode_list);

// читать датчик каждую секунду (написано, что так ему лучше)
#define INTERVAL_READ_SENSOR 1
// обновить данные для графиков, там 50 значений
// если записывать через 30 (60) секунд, то на экране будет 25 минут (50 минут)
// сетка на экране через 5 минут или 10 минут
#define INTERVAL_UPDATE_MEASURES 30
// Выводить графики или большие цифры через 10 секунд
#define INTERVAL_CHANGE_DISPLAY 5

// обновить значения маленьких цифр
bool need_update_plot_numbers = true;

// Read measures. Call every 1 second (exact)
void update_current_measures_every_1s()
{
    if (sgp30_connected)
    {
        // если датчик есть, то измерить
        mySensor.measureAirQuality();
        add_new_measure(mySensor.CO2, mySensor.TVOC);
    }
    else
    {
        // случайные значения
        add_new_measure(fake_measure(_CO2E), fake_measure(_TVOC));
    }
    /*
    В uart будует отправляться строки с текущими измерениями
    437012 CO2=419 TVOC=96
    438012 CO2=429 TVOC=100
    439012 CO2=424 TVOC=91
    */
    char s[50];
    sprintf(s, "%lu CO2=%u TVOC=%u",
            millis(),
            measures[_CO2E].current,
            measures[_TVOC].current);
    Serial.println(s);
}

void setup()
{
    delay(100);
    Serial.begin(9600);

    // инициализация i2c
    Wire.begin();

    // экран
    oled.init();
    oled.clear();
    oled.setScale(1);
    oled.setCursorXY(10, 8);
    oled.print(__DATE__);
    oled.setCursorXY(10, 16);
    oled.print(__TIME__);
    oled.update();
    delay(500);
    oled.clear();
    oled.update();

    // стартовый бзз
    pinMode(buzzer, OUTPUT);
    tone(buzzer, 400, 40);
    delay(50);
    tone(buzzer, 1000, 40);
    delay(50);
    tone(buzzer, 400, 40);
    delay(50);

    // подготовка истории измерений
    init_measures();

    // Initialize sensor
    sgp30_connected = mySensor.begin();
    if (!sgp30_connected)
    {
        oled.clear();
        oled.setCursorXY(10, 10); // oled display
        oled.setScale(2);
        oled.print(F("NO SGP30"));
        oled.update();
        Serial.println(F("No SGP30 Detected. Check connections."));
        sgp30_connected = false;
        delay(5000);
    }

    // Initializes sensor for air quality readings
    // measureAirQuality should be called in one second increments after a call to initAirQuality
    if (sgp30_connected)
    {
        // если датчик есть, то начать измерения,
        // следующие чтения должны быть через 1 секунду
        mySensor.initAirQuality();
        delay(1000); // Wait 1 second
    }
    // для имитации измерений
    // sgp30_connected = false;
    // чтение измерений
    update_current_measures_every_1s();
}

// Пищит, если плохо
void sound_buzzer_alarm()
{
    // если тревога уже была,
    if (alarm_active)
    {
        // если прошло много времени, то сбросить флаг тревоги
        if (millis() > alarm_time_delay_ms)
        {
            alarm_active = 0;
        }
        // то выйти
        return;
    }

    // если тревоги нет, то выйти
    if (!alarm)
    {
        return;
    }

    // значит тревога
    alarm_active = 1;
    // через время тревога отключится
    alarm_time_delay_ms = millis() + ALARM_DELAY_MS;

    pinMode(buzzer, OUTPUT);
    tone(buzzer, 1000, 40);
    delay(50);
    tone(buzzer, 1600, 40);
    delay(50);
    tone(buzzer, 1800, 40);
    delay(50);
}

// выводит большие цифры желтым цветов на графиком
void print_small_measures()
{
    char s1[12];
    if (display_sensor_type == _CO2E)
    {
        sprintf(s1, "CO2 %4u", measures[_CO2E].current);
    }
    else
    {
        sprintf(s1, "TVOC %4u", measures[_TVOC].current);
    }
    oled.setScale(2);
    oled.setCursorXY(0, 0);
    oled.invertText(blink != 0);
    oled.print(s1);
    oled.setScale(1);
    if (display_sensor_type == _CO2E)
    {
        oled.setCursorXY(98, 0);
        oled.print(F("ppm"));
    }
    else
    {
        oled.setCursorXY(110, 0);
        oled.print(F("ppb"));
    }
    oled.invertText(false);
}

// рисует графики
void draw_bars()
{
    uint16_t x0 = 26;
    // столбики графика
    const uint16_t H = 64;
    for (uint8_t i = 0; i < TOTAL_MEASURES; i++)
    {
        if (samples[i] > 0)
        {
            uint16_t x = i * 2;
            uint16_t y = H - 1 - samples[i];
            oled.fastLineV(x0 + x, y, H - 1);
        }
    }

    // вертикальные линии пунктиром - через 15 минут
    for (uint8_t i = 0; i < 9; i++)
    {
        oled.dot(x0 + 21, 19 + i * 5);
        oled.dot(x0 + 41, 19 + i * 5);
        oled.dot(x0 + 61, 19 + i * 5);
        oled.dot(x0 + 81, 19 + i * 5);
    }

    // рамка вокруг графика
    oled.rect(x0 + 0, 16, x0 + 100, 63, OLED_STROKE);

    // слева на графике минимум и максимум
    oled.setScale(1);
    oled.invertText(false);
    oled.setCursorXY(0, 16); // oled display
    char s[6];
    sprintf(s, "%4u", measures[display_sensor_type].max);
    oled.print(s);
    oled.setCursorXY(0, 64 - 8); // oled display
    sprintf(s, "%4u", measures[display_sensor_type].min);
    oled.print(s);
}

void loop()
{
    EVERY_N_SECONDS(INTERVAL_READ_SENSOR)
    {
        // Первые 15 секунд CO2: 400 ppm  TVOC: 0 ppb
        update_current_measures_every_1s();

        if ((display_mode == DISPLAY_PLOT_CO2) || (display_mode == DISPLAY_PLOT_TVOC))
        {
            if (test_air_quality_input() != AIR_GOOD)
            {
                blink = (blink + 1) % 2;
            }
            else
            {
                blink = 0;
            }
            if (test_air_quality_last_5min() != AIR_GOOD)
            {
                alarm = 1;
            }
            else
            {
                alarm = 0;
            }

            // на графике обновить маленькие цифры - текущие измерения
            print_small_measures();
            oled.update();
        }
    }

    // добавить измерение в данные для графика
    EVERY_N_SECONDS(INTERVAL_UPDATE_MEASURES)
    {
        // добавляет текущее измерение, усредненное с предыдущим в историю
        update_measure_data();
    }

    // переключить большие цифры или график
    EVERY_N_SECONDS(INTERVAL_CHANGE_DISPLAY)
    {
        // выбираем следующий режим по индексу из списка
        // индекс измениться, когда снова сюда попадем
        display_mode_index = (display_mode_index + 1) % display_mode_count;
        display_mode = display_mode_list[display_mode_index];
        display_sensor_type = (display_mode == DISPLAY_PLOT_CO2) ? _CO2E : _TVOC;
        if ((display_mode == DISPLAY_PLOT_CO2) || (display_mode == DISPLAY_PLOT_TVOC))
        {
            // если режим с графиками, то очистить экран и нарисовать график и маленькие цифры
            prepare_display_samples(display_sensor_type);
            oled.clear();
            print_small_measures();
            draw_bars();
            oled.update();
        }
        // alarm = 1;
        // alarm_active=0;
        sound_buzzer_alarm();
    }
}
