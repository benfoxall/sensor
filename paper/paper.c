#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "EPD_Test.h" //Examples
#include "qrcode.h"
#include "lib/e-Paper/EPD_2in13_V2.h"

float read_onboard_temperature()
{
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12); // 3.3V / 4096

    // Read the raw ADC value
    uint16_t raw_adc = adc_read();

    // Convert raw ADC value to voltage
    float adc_voltage = (float)raw_adc * conversionFactor;

    // Convert voltage to temperature in Celsius using the datasheet formula
    // T = 27 - (ADC_voltage - 0.706) / 0.001721
    float tempC = 27.0f - (adc_voltage - 0.706f) / 0.001721f;

    return tempC;
}

float adc_to_temp(uint16_t raw_adc)
{
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12); // 3.3V / 4096

    // Convert raw ADC value to voltage
    float adc_voltage = (float)raw_adc * conversionFactor;

    // Convert voltage to temperature in Celsius using the datasheet formula
    // T = 27 - (ADC_voltage - 0.706) / 0.001721
    float tempC = 27.0f - (adc_voltage - 0.706f) / 0.001721f;

    return tempC;
}

#define MAX_SENSOR_READINGS 20
uint16_t sensor_values[MAX_SENSOR_READINGS] = {0};

// Helper to build the sensor data URL string, now with counter
void build_sensor_url(char *url_buf, size_t buf_size, uint16_t *values, int count, unsigned long count_val)
{
    int offset = snprintf(url_buf, buf_size, "https://benjaminbenben.com/sensor?d=");
    for (int j = 0; j < count; j++)
    {
        offset += snprintf(url_buf + offset, buf_size - offset, "%u", values[j]);
        if (j < count - 1)
        {
            offset += snprintf(url_buf + offset, buf_size - offset, ",");
        }
    }
    // Add counter as &count=...
    snprintf(url_buf + offset, buf_size - offset, "&c=%lu", count_val);
}

// Helper to display a QR code and temperature and counter on the e-Paper display
void display_qr_and_temp(const char *url, float temp, unsigned long count)
{
    // The structure to manage the QR code
    QRCode qrcode;
    int qSize = 10;
    uint8_t qrcodeBytes[qrcode_getBufferSize(qSize)];
    qrcode_initText(&qrcode, qrcodeBytes, qSize, ECC_LOW, url);

    // Create a new image cache
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_2IN13_V2_WIDTH % 8 == 0) ? (EPD_2IN13_V2_WIDTH / 8) : (EPD_2IN13_V2_WIDTH / 8 + 1)) * EPD_2IN13_V2_HEIGHT;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for black memory...\r\n");
        return;
    }
    Paint_NewImage(BlackImage, EPD_2IN13_V2_WIDTH, EPD_2IN13_V2_HEIGHT, 270, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_SetMirroring(MIRROR_HORIZONTAL);
    Paint_Clear(WHITE);

    // Draw QR code
    uint8_t size = 2;
    uint8_t padd = 4;
    for (uint8_t y = 0; y < qrcode.size; y++)
    {
        for (uint8_t x = 0; x < qrcode.size; x++)
        {
            if (qrcode_getModule(&qrcode, x, y))
            {
                Paint_DrawRectangle(
                    padd + (x * size),
                    padd + (y * size),
                    padd + ((x + 1) * size) - 1,
                    padd + ((y + 1) * size),
                    BLACK,
                    DOT_PIXEL_1X1,
                    DRAW_FILL_FULL);
            }
        }
    }

    // Draw temperature string
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "T:%.1f C", temp);
    Paint_DrawString_EN(130, 2, temp_str, &Font16, WHITE, BLACK);

    // Draw counter string
    char count_str[20];
    snprintf(count_str, sizeof(count_str), "I:%lu", count);
    Paint_DrawString_EN(130, 22, count_str, &Font16, WHITE, BLACK);

    EPD_2IN13_V2_Display(BlackImage);
    DEV_Delay_ms(1000); // Short delay to ensure display update

    free(BlackImage);
}

int main()
{
    stdio_init_all();
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // 4 is the internal temperature sensor

    printf("EPD_2IN13_V2_test Demo\r\n");
    if (DEV_Module_Init() != 0)
    {
        return -1;
    }

    int current_sensor_index = 0;
    char url_buf[256];
    unsigned long count = 0;

    while (true)
    {
        count++;
        // Read the current onboard temperature (raw value)
        uint16_t temp_raw = adc_read();
        float temp_c = adc_to_temp(temp_raw);

        // Store the temperature (as an integer for uint16_t array)
        sensor_values[current_sensor_index] = (uint16_t)temp_raw;

        printf("\n--- Temperature Reading #%d ---\n", current_sensor_index + 1);
        printf("Raw Temperature (float): %.2f C\n", temp_c);
        printf("Stored Temperature (uint16_t) at index %d: %u C\n", current_sensor_index, sensor_values[current_sensor_index]);

        // Print the current state of the sensor_values array
        printf("sensor_values array: [");
        for (int j = 0; j < MAX_SENSOR_READINGS; j++)
        {
            printf("%u", sensor_values[j]);
            if (j < MAX_SENSOR_READINGS - 1)
            {
                printf(",");
            }
        }
        printf("]\n");

        // Build the URL with the most recent sensor readings (oldest to newest) and counter
        int start = (current_sensor_index + 1) % MAX_SENSOR_READINGS;
        uint16_t ordered_values[MAX_SENSOR_READINGS];
        for (int j = 0; j < MAX_SENSOR_READINGS; j++)
        {
            ordered_values[j] = sensor_values[(start + j) % MAX_SENSOR_READINGS];
        }
        build_sensor_url(url_buf, sizeof(url_buf), ordered_values, MAX_SENSOR_READINGS, count);

        // Wake up and initialize the display
        EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
        EPD_2IN13_V2_Clear();
        DEV_Delay_ms(500);

        // Display QR code, temperature, and counter
        display_qr_and_temp(url_buf, temp_c, count);

        // Put the display to sleep for power saving
        EPD_2IN13_V2_Sleep();
        DEV_Delay_ms(2000); // At least 2s before power off
        DEV_Module_Exit();

        // Move to the next index, wrapping around to 0 if at the end
        current_sensor_index = (current_sensor_index + 1) % MAX_SENSOR_READINGS;

        // Wait for 2.5 seconds before the next reading (change to 30*60*1000 for 30min)
        sleep_ms(2500);
    }
}
