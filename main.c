#include "bbb_dht_read.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    float humidity = 0;
    float temperature = 0;

    printf("\033[2J");
    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        printf("\033[H");
        printf("Result: %d\n", result);
        printf("Humidity: %.2f%%, Temperature: %.2f°C\n", humidity, temperature);

        fflush(stdout);
        sleep(1);
    }

    return 0;
}