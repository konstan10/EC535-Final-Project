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

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(1000);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_RED, COLOR_BLACK);
    }

    curl_global_init(CURL_GLOBAL_ALL);

    erase();
    box(stdscr, 0, 0);
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, 4, "BeagleBone Black Smart Environment Monitor");
    attroff(COLOR_PAIR(1) | A_BOLD);

    mvprintw(3, 4, "Result:");
    mvprintw(4, 4, "Humidity:");
    mvprintw(5, 4, "Temperature:");
    mvprintw(7, 4, "[Refreshing every second]");
    refresh();

    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        attron(COLOR_PAIR(4));
        mvprintw(3, 13, "%3d   ", result);
        attroff(COLOR_PAIR(4));

        attron(COLOR_PAIR(2));
        mvprintw(4, 14, "%6.2f %% ", humidity);
        attroff(COLOR_PAIR(2));

        attron(COLOR_PAIR(3));
        mvprintw(5, 17, "%6.2f deg C", temperature);
        attroff(COLOR_PAIR(3));

        wnoutrefresh(stdscr);
        doupdate();

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
