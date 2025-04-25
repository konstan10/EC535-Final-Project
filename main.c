#include "bbb_dht_read.h"
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <ncurses.h>

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    float humidity = 0;
    float temperature = 0;
    int ch;

    initscr();              // Start ncurses mode
    cbreak();               // Disable line buffering
    noecho();               // Don't echo keypresses
    curs_set(0);            // Hide cursor
    keypad(stdscr, TRUE);   // Enable arrow keys and function keys

    curl_global_init(CURL_GLOBAL_ALL);

    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "BeagleBone Sensor Monitor");
        mvprintw(3, 4, "Result: %d", result);
        mvprintw(4, 4, "Humidity: %.2f%%", humidity);
        mvprintw(5, 4, "Temperature: %.2f°C", temperature);
        mvprintw(7, 4, "[R] Refresh   [Q] Quit");

        refresh();

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

        ch = getch();
        if (ch == 'q' || ch == 'Q') break;
        if (ch == 'r' || ch == 'R') {
            // Simulate refresh on key press
            // The loop will continue updating the data
        }

        sleep(1);
    }

    curl_global_cleanup();
    endwin();
    return 0;
}
