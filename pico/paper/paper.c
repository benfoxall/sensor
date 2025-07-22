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

int main()
{

    stdio_init_all();
    // Initialize ADC and temperature sensor
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // 4 is the internal temperature sensor

    // The structure to manage the QR code
    QRCode qrcode;

    // Allocate a chunk of memory to store the QR code
    uint8_t qrcodeBytes[qrcode_getBufferSize(10)];

    // height = 122 114 = 4padd

    // type 10 @ LOW = 57x57, 271 bytes

    qrcode_initText(&qrcode, qrcodeBytes, 10, ECC_LOW, "https://benjaminbenben.com?q=123456789012345678901234567890123456789012345678901234567890");

    printf("EPD_2IN13_V2_test Demo\r\n");
    if (DEV_Module_Init() != 0)
    {
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
    EPD_2IN13_V2_Clear();
    DEV_Delay_ms(500);

    // Create a new image cache
    UBYTE *BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_2IN13_V2_WIDTH % 8 == 0) ? (EPD_2IN13_V2_WIDTH / 8) : (EPD_2IN13_V2_WIDTH / 8 + 1)) * EPD_2IN13_V2_HEIGHT;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
    {
        printf("Failed to apply for black memory...\r\n");
        return -1;
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN13_V2_WIDTH, EPD_2IN13_V2_HEIGHT, 270, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_SetMirroring(MIRROR_HORIZONTAL); //
    Paint_Clear(WHITE);

    // drawing

    printf("Drawing\r\n");
    // 1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    // 2.Drawing on the image
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
                    padd + ((x + 1) * size) - 1, // the rect fills an extra line to the right
                    padd + ((y + 1) * size),
                    BLACK,
                    DOT_PIXEL_1X1,
                    DRAW_FILL_FULL);
            }
        }
    }

    Paint_DrawString_EN(130, 2, "t=12345", &Font16, WHITE, BLACK);

    // Read temperature and display it (one decimal place)
    float temp = read_onboard_temperature();
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "T:%.1f", temp);
    Paint_DrawString_EN(130, 20, temp_str, &Font16, WHITE, BLACK);

    EPD_2IN13_V2_Display(BlackImage);
    DEV_Delay_ms(10000);

    // clear
    printf("Clear...\r\n");

    EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
    EPD_2IN13_V2_Clear();
    DEV_Delay_ms(2000); // Analog clock 1s

    printf("Goto Sleep...\r\n");
    EPD_2IN13_V2_Sleep();
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000); // important, at least 2s
    // close 5V
    printf("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();

    // the actual example

    // EPD_2in13_V2_test();

    // while (true) {
    //     printf("Hello, world!\n");
    //     sleep_ms(1000);
    // }
}
