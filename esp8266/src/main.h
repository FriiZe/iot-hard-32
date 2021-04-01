#ifndef MAIN__H
#define MAIN__H

#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// LCD
#define LCD_ADDRESS 0x27
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// EEPROM
#define EEPROM_SIZE 32
#define EEPROM_TIMER 10
#define EEPROM_PASSWORD_ADDRESS 0
#define EEPROM_TIMER_ADDRESS 8
#define EEPROM_USAGE_ADDRESS 24

// KEYPAD 
#define KEY_UP 2
#define KEY_DOWN 8
#define KEY_RETURN 5

// MENU
#define MENU_LENGTH 3
#define SET_ALARM_ON 0
#define CHANGE_TIMER 1
#define CHANGE_PASSWORD 2
#define RESET_MENU -1
#define ENTER_CODE_MSG String("Enter code:")
#define NEW_CODE_MSG String("New code:")
#define CONFIRM_CODE_MSG String("Confirm code:")

// Other
#define DELAY 250

void initLCD();
int askPassword(String message);
bool askConfirmPassword(int code);
void updateCurrentCode();
void displayStars(int &index);
void displayMenu(int caseDirection);
void displayReset();
void goDown();
void goUp();
void goTo();

void savePasswordIntoEeprom(int password);
int getPasswordFromEeprom();
void saveTimerIntoEeprom(int timer);
int getTimerFromEeprom();
void saveUsageIntoEeprom();
int getUsageFromEeprom();

void setupMQTT();
void setupWifi();
void reconnect();
void sendIsActivePayload(String);
void pairAlarm();
void waitForPairing();
void pairing();

#endif // MAIN__H