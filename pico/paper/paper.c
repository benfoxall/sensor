#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "EPD_Test.h" //Examples
#include "qrcode.h"
#include "lib/e-Paper/EPD_2in13_V2.h"

int main()
{
    stdio_init_all();

    // The structure to manage the QR code
    QRCode qrcode;

    // Allocate a chunk of memory to store the QR code
    uint8_t qrcodeBytes[512];

    qrcode_initText(&qrcode, qrcodeBytes, 3, ECC_LOW, "https://benjaminbenben.com?q=123456789012345678901234567890123456789012345678901234567890");

    // basic painting

    // copied in from example
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

    // attempt to clear?
    printf("show image for array\r\n");
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawBitMap(gImage_2in13);

    EPD_2IN13_V2_Display(BlackImage);
    DEV_Delay_ms(2000);

    printf("Drawing\r\n");
    // 1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    Paint_DrawString_EN(140, 15, "wave?!", &Font16, BLACK, WHITE);

    uint8_t size = 3;

    for (uint8_t y = 0; y < qrcode.size; y++)
    {
        for (uint8_t x = 0; x < qrcode.size; x++)
        {
            if (qrcode_getModule(&qrcode, x, y))
            {
                // printf("**");

                // Paint_SetPixel(x * size, y * size, BLACK);

                Paint_DrawRectangle(
                    x * size,
                    y * size,
                    (x + 1) * size,
                    (y + 1) * size,
                    BLACK,
                    DOT_PIXEL_1X1,
                    DRAW_FILL_FULL);
            }
            else
            {
                // Paint_SetPixel(x * size, y * size, WHITE);
                // printf("  ");
            }

            // printf("\n");
        }
    }

    EPD_2IN13_V2_Display(BlackImage);

    DEV_Delay_ms(2000);

    // the actual example

    EPD_2in13_V2_test();

    // while (true) {
    //     printf("Hello, world!\n");
    //     sleep_ms(1000);
    // }
}
