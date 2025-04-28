#include "bbb_dht_read.h"
#include "ccs811.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <ncurses.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

#define UNIT_BUTTON_GPIO "/sys/class/gpio/gpio66/value"

typedef struct {
    float humidity;
    float temperature;
    char unit[2];
} SensorData;

int curr_button_state = 0;
int last_button_state = 0;
volatile int disp_fahr = 0;

int read_button() {
    int fd = open(UNIT_BUTTON_GPIO, O_RDONLY);
    char value;
    read(fd, &value, 1);
    close(fd);
    return (value == '0') ? 0 : 1;
}

void *poll_button_state() {
    while (1) {
        curr_button_state = read_button();
        if (last_button_state == 1 && curr_button_state == 0) {
            disp_fahr = !disp_fahr;
        }
        last_button_state = curr_button_state;
        usleep(100000);
    }
}

void *send_data(SensorData *data) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char post_data[100];
        snprintf(post_data, sizeof(post_data),
                 "{\"humidity\": %.2f, \"temperature\": %.2f, \"unit\": \"%s\"}", 
                 data->humidity, data->temperature, data->unit);

        curl_easy_setopt(curl, CURLOPT_URL, "https://fast-kid-sterling.ngrok-free.app/data");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        FILE* null_file = fopen("/dev/null", "w");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, null_file);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    free(data);
}

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    
    float humidity = 0;
    float temperature = 0;
    int eCO2 = 0;
    int TVOC = 0;

    int air_qual_sensor = ccs811Init(2, 0x5A);

    if (air_qual_sensor != 0) {
        printf("error connecting sensor\n");
        return 0;
    }

    pthread_t button_thread;
    pthread_create(&button_thread, NULL, poll_button_state, NULL);
    pthread_detach(button_thread);

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
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_BLUE, COLOR_BLACK);

    curl_global_init(CURL_GLOBAL_ALL);

    erase();
    box(stdscr, 0, 0);
    attron(COLOR_PAIR(5));
    mvprintw(1, 2, "BeagleBone Black Smart Environment Monitor");
    attroff(COLOR_PAIR(5));

    mvprintw(3, 4, "Result:");
    mvprintw(4, 4, "Humidity:");
    mvprintw(5, 4, "Temperature:");
    mvprintw(6, 4, "eCO2:");
    mvprintw(7, 4, "TVOC:");

    attron(COLOR_PAIR(6));
    mvprintw(8, 4, "[Refreshing every second]");
    attroff(COLOR_PAIR(6));
    refresh();

    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);
        int aq_res = ccs811ReadValues(&eCO2, &TVOC);

        if (result != 0 || aq_res != 0) {
            sleep(1);
            continue;
        }

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

        attron(COLOR_PAIR(7));
        mvprintw(6, 15, "%4d ppm", eCO2);
        attroff(COLOR_PAIR(7));

        attron(COLOR_PAIR(7));
        mvprintw(7, 15, "%4d ppb", TVOC);
        attroff(COLOR_PAIR(7));

        refresh();
        
        SensorData *data = (SensorData *)malloc(sizeof(SensorData));
        data->humidity = humidity;
        data->temperature = temp_display;
        strcpy(data->unit, unit);

        pthread_t curl_thread;
        pthread_create(&curl_thread, NULL, send_data, data);
        pthread_detach(curl_thread);

        sleep(1);
    }

    curl_global_cleanup();
    endwin();
    return 0;
}

// int main(int argc, char *argv[])
// {
// int i;
// int eCO2, TVOC;


// 	// Check which I2C channel you're using
// 	// Raspberry Pi's usually use channel 1
// 	// Other ARM boards tend to use channel 0
// 	i = ccs811Init(2, 0x5A);
// 	if (i != 0)
// 	{
// 		return -1; // problem - quit
// 	}
// 	printf("CCS811 device successfully opened.\n");
// 	printf("Allow 48 hours initial burn-in and 20 minutes to warm up (each use)\n");
// 	usleep(1000000); // wait for data to settle for first read

// 	for (i=0; i<36000; i++) // read values once every 5 seconds for 30 minutes
// 	{
// 		if (ccs811ReadValues(&eCO2, &TVOC))
// 		{
// 			printf("eCO2 = %d, TVOC = %d\n", eCO2, TVOC);
// 		}
// 		else
// 			printf("data not ready\n");
// 		printf("%02d:%02d min:sec have passed\n", (i*5)/60, (i*5) % 60);
// 		usleep(1000000); // every 5 seconds
// 	}

// return 0;
// }