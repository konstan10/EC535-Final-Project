#include "bbb_dht_read.h"
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <ncurses.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

#define UNIT_BUTTON_GPIO "/sys/class/gpio/gpio66/value"

int read_button() {
    int fd = open(UNIT_BUTTON_GPIO, O_RDONLY);
    char value;
    read(fd, &value, 1);
    close(fd);
    return (value == '0') ? 0 : 1;
}

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    float humidity = 0;
    float temperature = 0;
    int disp_fahr = 0;
    int last_button_state = 0;

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    curl_global_init(CURL_GLOBAL_ALL);

    erase();
    box(stdscr, 0, 0);
    attron(COLOR_PAIR(1));
    mvprintw(1, 2, "BeagleBone Black Smart Environment Monitor");

    mvprintw(3, 4, "Result:");
    mvprintw(4, 4, "Humidity:");
    mvprintw(5, 4, "Temperature:");

    mvprintw(7, 4, "[Refreshing every second]");
    attroff(COLOR_PAIR(1));
    refresh();

    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        int button_state = read_button();
        if (button_state == 0 && last_button_state == 1) {
            disp_fahr = !disp_fahr;
        }
        last_button_state = button_state;

        float temp_display = temperature;
        char unit[2];
        if (disp_fahr) {
            temp_display = temperature * 9.0 / 5.0 + 32.0;
            strcpy(unit, "F");
        } else {
            strcpy(unit, "C");
        }

        attron(COLOR_PAIR(4));
        mvprintw(3, 15, "%3d   ", result);
        attroff(COLOR_PAIR(4));

        attron(COLOR_PAIR(2));
        mvprintw(4, 15, "%4d %% ", (int)humidity);
        attroff(COLOR_PAIR(2));

        attron(COLOR_PAIR(3));
        mvprintw(5, 17, "%4.1f deg %s", temp_display, unit);
        attroff(COLOR_PAIR(3));

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

        sleep(1);
    }

    curl_global_cleanup();
    endwin();
    return 0;
}
