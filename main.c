#include "bbb_dht_read.h"
#include <stdio.h>

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    float humidity = 0;
    float temperature = 0;

    int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);
    
    printf("Result: %d\n", result);
    printf("Humidity: %.2f, Temperature: %.2f\n", humidity, temperature);

    return 0;
}
