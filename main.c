#include "bbb_dht_read.h"
#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Define screen width and height for the display
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

void draw_border(SDL_Renderer *renderer) {
    // Set the border color (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Draw the top and bottom borders
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        SDL_RenderDrawPoint(renderer, i, 0);             // Top border
        SDL_RenderDrawPoint(renderer, i, SCREEN_HEIGHT - 1); // Bottom border
    }

    // Draw the left and right borders
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        SDL_RenderDrawPoint(renderer, 0, i);             // Left border
        SDL_RenderDrawPoint(renderer, SCREEN_WIDTH - 1, i); // Right border
    }

    // Draw the corners
    SDL_RenderDrawPoint(renderer, 0, 0);                    // Top-left corner
    SDL_RenderDrawPoint(renderer, SCREEN_WIDTH - 1, 0);     // Top-right corner
    SDL_RenderDrawPoint(renderer, 0, SCREEN_HEIGHT - 1);    // Bottom-left corner
    SDL_RenderDrawPoint(renderer, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1); // Bottom-right corner
}

void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y) {
    SDL_Color textColor = {255, 255, 255, 255}; // White color
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

int main() {
    int type = DHT11;
    int gpio_base = 2;
    int gpio_number = 3;
    float humidity = 0;
    float temperature = 0;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    // Create an SDL window
    SDL_Window *window = SDL_CreateWindow("BeagleBone Sensor Monitor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Create a renderer for the window
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load the font
    TTF_Font *font = TTF_OpenFont("path/to/your/font.ttf", 24);
    if (!font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    curl_global_init(CURL_GLOBAL_ALL);

    // Main loop
    while (1) {
        int result = bbb_dht_read(type, gpio_base, gpio_number, &humidity, &temperature);

        // Clear the screen with black color
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the border
        draw_border(renderer);

        // Render the sensor data as text
        char text[100];
        snprintf(text, sizeof(text), "Result: %d", result);
        render_text(renderer, font, text, 10, 10);

        snprintf(text, sizeof(text), "Humidity: %.2f%%", humidity);
        render_text(renderer, font, text, 10, 50);

        snprintf(text, sizeof(text), "Temperature: %.2f°C", temperature);
        render_text(renderer, font, text, 10, 90);

        // Refresh the screen
        SDL_RenderPresent(renderer);

        // Perform HTTP POST request
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

        // Sleep for 1 second (timeout)
        SDL_Delay(1000);
    }

    curl_global_cleanup();

    // Clean up and quit SDL_ttf and SDL
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
