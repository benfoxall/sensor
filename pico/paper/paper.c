#include <stdio.h>
#include "pico/stdlib.h"
#include "EPD_Test.h" //Examples

int main()
{
    stdio_init_all();

    EPD_2in13_V2_test();

    // while (true) {
    //     printf("Hello, world!\n");
    //     sleep_ms(1000);
    // }
}
