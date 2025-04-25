#include "bbb_dht_read.h"
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    float humidity = 66.6;
    float temperature = 420.69;

    curl_global_init(CURL_GLOBAL_ALL);

    while (1) {
        // int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        printf("Result: %d\n", result);
        printf("Humidity: %.2f%%, Temperature: %.2f°C\n", humidity, temperature);

        CURL *curl = curl_easy_init();
        if (curl) {
            char post_data[100];
            snprintf(post_data, sizeof(post_data),
                     "{\"humidity\": %.2f, \"temperature\": %.2f}", humidity, temperature);

            curl_easy_setopt(curl, CURLOPT_URL, "https://fast-kid-sterling.ngrok-free.app/data");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_perform(curl);

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        sleep(1);
    }

    curl_global_cleanup();
    return 0;
}
