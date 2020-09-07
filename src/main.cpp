#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>
#include <stdio.h>

#define REFEREE_PIN   12
#define PLAYER_COUNT  2

unsigned int buttonPins[PLAYER_COUNT] = {13, 3};
unsigned int ledPins[PLAYER_COUNT] = {9, 11};
unsigned int errorPins[PLAYER_COUNT] = {8, 7};

unsigned int wins[PLAYER_COUNT] = {0, 0};
unsigned int fouls[PLAYER_COUNT] = {0, 0};

LiquidCrystal_I2C lcd(0x27, 16, 2);
char* first_line_lcd;
char* second_line_lcd;

void print_reaction(unsigned long pressed_time, unsigned long current_time) {
    lcd.clear();
    lcd.print("Speed reaction");

    lcd.setCursor(0, 1);
    sprintf(second_line_lcd, "%lu milliseconds", pressed_time - current_time);
    lcd.print(second_line_lcd);
    delay(1000);
}

void redraw_lcd() {
    lcd.clear();
    sprintf(first_line_lcd, "Score: %u:%u", wins[0], wins[1]);
    lcd.print(first_line_lcd);

    lcd.setCursor(0, 1);
    sprintf(second_line_lcd, "Fouls: %u:%u", fouls[0], fouls[1]);
    lcd.print(second_line_lcd);
}

void set_start_state() {
    lcd.clear();
    lcd.print("Score: 0:0");
    lcd.setCursor(0, 1);
    lcd.print("Fouls: 0:0");
}

void end_game(int winner) {
    if (winner) {
        sprintf(first_line_lcd, "Player red win");
        sprintf(second_line_lcd, "Player blue lose");
    } else {
        sprintf(first_line_lcd, "Player blue win");
        sprintf(second_line_lcd, "Player red lose");
    }

    lcd.clear();
    lcd.print(first_line_lcd);

    lcd.setCursor(0, 1);
    lcd.print(second_line_lcd);

    for (int i = 0; i < PLAYER_COUNT; ++i) {
        wins[i] = 0;
        fouls[i] = 0;
    }

    delay(1500);
    set_start_state();
}

void check_win(int player) {
    if (wins[player] == 3) {
        end_game(player);
    } else {
        redraw_lcd();
        delay(1000);
    }
}

void setup() {
    /* set up LCD*/
    lcd.init();
    lcd.backlight();
    set_start_state();

    Serial.begin(9600);
    pinMode(REFEREE_PIN, OUTPUT);
    for (int player = 0; player < PLAYER_COUNT; ++player) {
        pinMode(ledPins[player], OUTPUT);
        pinMode(errorPins[player], OUTPUT);
        pinMode(buttonPins[player], INPUT_PULLUP);
    }
}

void clear_memory() {
    free(first_line_lcd);
    free(second_line_lcd);
}

void loop() {
    first_line_lcd = (char*)calloc(16, sizeof(char));
    second_line_lcd = (char*)calloc(16, sizeof(char));

    unsigned long start_game_time = millis();
    unsigned long interval = random(1000, 3000); // random interval which referee (light in my case) waiting before give signal
    unsigned long current_time = millis();

    /* checking false start */
    while (current_time - start_game_time < interval) {
        for (int player = 0; player < PLAYER_COUNT; ++player) {
            if (!digitalRead(buttonPins[player])) {
                digitalWrite(errorPins[player], HIGH);
                fouls[player]++;

                if (fouls[player] == 2) {
                    fouls[player] = 0;
                    wins[!player]++;
                }
                check_win(!player);

                digitalWrite(errorPins[player], LOW);
                clear_memory();
                return;
            }
        }

        current_time = millis();
    }

    digitalWrite(REFEREE_PIN, HIGH);
    current_time = millis();
    unsigned long pressed_time;

    /* checking button click */
    for (int player = 0; ; player = (player + 1) % 2) {
        if (!digitalRead(buttonPins[player])) {
            pressed_time = millis();
            digitalWrite(REFEREE_PIN, LOW);
            digitalWrite(ledPins[player], HIGH);

            if (pressed_time - current_time < 999) {
                print_reaction(pressed_time, current_time);
            }

            wins[player]++;
            digitalWrite(ledPins[player], LOW);
            check_win(player);
            break;
        }
    }
}
