#include "bbb_dht_read.h"
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <ncurses.h>

void draw_border() {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    mvhline(0, 0, 0, max_x);
    mvhline(max_y-1, 0, 0, max_x);
    mvvline(0, 0, 0, max_y);
    mvvline(0, max_x-1, 0, max_y);
    mvaddch(0, 0, ACS_ULCORNER);
    mvaddch(0, max_x-1, ACS_URCORNER);
    mvaddch(max_y-1, 0, ACS_LLCORNER);
    mvaddch(max_y-1, max_x-1, ACS_LRCORNER);
}

void setup_colors() {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_RED, -1);
    init_pair(4, COLOR_YELLOW, -1);
    init_pair(5, COLOR_WHITE, -1);
}

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

    setup_colors();
    curl_global_init(CURL_GLOBAL_ALL);

    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        clear();
        draw_border();

        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(1, 4, "BeagleBone DHT11 Sensor Monitor");
        attroff(COLOR_PAIR(1) | A_BOLD);

        attron(COLOR_PAIR(5));
        mvprintw(3, 6, "Result Code: ");
        attron(COLOR_PAIR(2));
        printw("%d", result);
        attroff(COLOR_PAIR(2));

        attron(COLOR_PAIR(5));
        mvprintw(5, 6, "Humidity: ");
        attron(COLOR_PAIR(4));
        printw("%.2f %%", humidity);
        attroff(COLOR_PAIR(4));

        attron(COLOR_PAIR(5));
        mvprintw(7, 6, "Temperature: ");
        if (temperature > 30.0) {
            attron(COLOR_PAIR(3) | A_BOLD);
        } else {
            attron(COLOR_PAIR(2));
        }
        printw("%.2f deg C", temperature);
        attroff(COLOR_PAIR(2) | COLOR_PAIR(3) | A_BOLD);

        attron(COLOR_PAIR(1));
        mvprintw(10, 4, "[Refreshing every second]");
        attroff(COLOR_PAIR(1));

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
