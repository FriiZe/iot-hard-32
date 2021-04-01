#include "main.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
WiFiClient cli;

// ENVIRONMENT
const char *mqtt_server = "mqtt.potb.dev";
const int mqtt_port = 8883;
const char *wifi_ssid = "SFR_4CA0";
const char *wifi_password = "49dd0e4b41";
const String hostname = "192.168.1.83";
const String apiPort = "3000";
const char *id = "254a89c7-ac15-43c9-b01b-ad465ab44d24";

// KEYPAD
char key = NO_KEY;
const byte ROWS = 3;
const byte COLS = 3;
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
};

byte rowPins[ROWS] = {D0, D3, D4};
byte colPins[COLS] = {D5, D6, D7};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// MENU
LiquidCrystal_I2C LCD(LCD_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
int indexMenu = 0;
bool isMenuPrinted = false;
String menu[MENU_LENGTH] = {
    {"Set alarm on"},
    {"Change timer"},
    {"Change code"},
    {"Pair alarm"},
};

// VARIABLES
int isSaved = 9999;
int checkSaved = -1;

bool isWrongCode = true;
bool isPasswordConfirm = false;
int password = 0;
int currentCode = -1;

void setup()
{

  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  initLCD();
  setupWifi();
  setupMQTT();

  checkSaved = getUsageFromEeprom();
}

/**
 * Main program
 * */
void loop()
{
  // Not first time usage
  if (checkSaved == isSaved)
  {
    while (true)
    {
      key = keypad.getKey();

      if (!isMenuPrinted)
      {
        displayReset();
        LCD.print(menu[indexMenu]);
        isMenuPrinted = true;
      }

      if (key != NO_KEY)
      {
        goDown();
        goUp();
        goTo();
      }
      delay(DELAY);
    }
  }
  // First time usage
  else
  {

    pairing();

    while (!isPasswordConfirm)
    {
      password = askPassword(ENTER_CODE_MSG);
      isPasswordConfirm = askConfirmPassword(password);

      if (isPasswordConfirm)
      {
        savePasswordIntoEeprom(password);
        saveTimerIntoEeprom(EEPROM_TIMER);
        saveUsageIntoEeprom();
        checkSaved = isSaved;
      }
    }
  }
}

// LCD methods
/**
 * Initialize the LCD
 * */
void initLCD()
{
  LCD.init();
  LCD.backlight();
  //LCD.createChar(ARROW_UP, arrowUp);
  //LCD.createChar(ARROW_DOWN, arrowDown);
  LCD.begin(LCD_WIDTH, LCD_HEIGHT);
  displayReset();
}

/**
 * Ask the password and hide number pressed
 * Return the password typed.
 * */
int askPassword(String message)
{

  displayReset();
  LCD.print(message);

  int index = 0;
  isWrongCode = true;
  currentCode = -1;

  while (isWrongCode)
  {
    key = keypad.getKey();

    if (key != NO_KEY)
    {
      updateCurrentCode();
      displayStars(index);
    }
    // Check if currentCode is a 4 digit number
    if (currentCode > 999)
    {
      isWrongCode = false;
    }

    delay(DELAY);
  }
  return currentCode;
}

/**
 * Ask a confirmation of password
 * Return true if the password is fine
 * */
bool askConfirmPassword(int code)
{

  displayReset();
  LCD.print(CONFIRM_CODE_MSG);

  int index = 0;
  isWrongCode = true;
  currentCode = -1;

  while (isWrongCode)
  {
    key = keypad.getKey();

    if (key != NO_KEY)
    {
      updateCurrentCode();
      displayStars(index);
    }

    if (currentCode > 999)
    {
      isWrongCode = false;
      return code == currentCode ? true : false;
    }

    delay(DELAY);
  }
  return false;
}

/**
 * Display timer
 * */
int askTimer()
{

  int timer = getTimerFromEeprom();
  int isChanging = true;

  displayReset();
  LCD.print("Modify timer: ");
  LCD.setCursor(0, 1);
  LCD.print(String(timer));

  while (isChanging)
  {
    key = keypad.getKey();

    if (key != NO_KEY)
    {

      switch (atoi(&key))
      {

      case KEY_UP:
        if (timer > 0)
        {
          timer--;
        }
        break;

      case KEY_DOWN:
        timer++;
        break;

      case KEY_RETURN:
        isChanging = false;
        break;

      default:
        break;
      }

      LCD.clear();
      LCD.setCursor(0, 1);
      LCD.print(String(timer));
    }

    delay(DELAY);
  }

  return timer;
}

/** 
 * Save the password into the EEPROM
 * */
void savePasswordIntoEeprom(int password)
{
  EEPROM.put(EEPROM_PASSWORD_ADDRESS, password);
  EEPROM.commit();
}

/**
 * Get the saved password from the EEPROM
 * */
int getPasswordFromEeprom()
{
  int password = -1;
  EEPROM.get(EEPROM_PASSWORD_ADDRESS, password);
  return password;
}

/**
 * Save the timer into the EEPROM.
 * */
void saveTimerIntoEeprom(int timer)
{
  EEPROM.put(EEPROM_TIMER_ADDRESS, timer);
  EEPROM.commit();
}

/**
 * Get the saved timer from the EEPROM
 * */
int getTimerFromEeprom()
{
  int timer = -1;
  EEPROM.get(EEPROM_TIMER_ADDRESS, timer);
  return timer;
}

int getUsageFromEeprom()
{
  int usage = -1;
  EEPROM.get(EEPROM_USAGE_ADDRESS, usage);
  return usage;
}

/**
 * Save the usage into the EEPROM.
 * */
void saveUsageIntoEeprom()
{
  EEPROM.put(EEPROM_USAGE_ADDRESS, isSaved);
  EEPROM.commit();
}

/**
 * Update the current code
 * */
void updateCurrentCode()
{
  if (currentCode == -1)
  {
    currentCode = atoi(&key);
  }
  else
  {
    currentCode = currentCode * 10 + atoi(&key);
  }
}

/**
 * Clear and reset cursor to the start
 * */
void displayReset()
{
  LCD.clear();
  LCD.setCursor(0, 0);
}

/**
 * Display stars on LCD on 
 * the second line and at the index.
 * */
void displayStars(int &index)
{
  LCD.setCursor(index++, 1);
  LCD.print("*");
}

/**
 * Display the menu at the index
 * */
void displayMenu(int caseDirection)
{
  displayReset();

  switch (caseDirection)
  {
  case KEY_UP:
    LCD.print(menu[--indexMenu]);
    break;
  case KEY_DOWN:
    LCD.print(menu[++indexMenu]);
    break;

  default:
    indexMenu = 0;
    LCD.print(menu[indexMenu]);
    break;
  }
};

/**
 * Go down to the menu
 * */
void goDown()
{
  if (atoi(&key) == KEY_DOWN && indexMenu < MENU_LENGTH - 1)
  {
    displayMenu(KEY_DOWN);
  }
}

/**
 * Go up to the menu
 * */
void goUp()
{
  if (atoi(&key) == KEY_UP && indexMenu > 0)
  {
    displayMenu(KEY_UP);
  }
}

/**
 * Go into the selected option
 * */
void goTo()
{
  if (atoi(&key) == KEY_RETURN)
  {

    switch (indexMenu)
    {
    case SET_ALARM_ON:
    {
      int password = getPasswordFromEeprom();
      while (password != askPassword(ENTER_CODE_MSG))
      {
      }
      sendIsActivePayload("1");
      displayReset();
      LCD.print("Alarm on");
      delay(1000); // Waiting to see the message alarm setting on

      while (password != askPassword("Unlock alarm:"))
      {
      }
      sendIsActivePayload("0");
      displayReset();
      LCD.print("Alarm off");
      delay(1000); // Waiting to see the message alarm setting off

      break;
    }

    case CHANGE_TIMER:
    {
      int timer = askTimer();
      saveTimerIntoEeprom(timer);
      break;
    }

    case CHANGE_PASSWORD:
    {
      int password = getPasswordFromEeprom();
      bool isPasswordConfirm = false;

      while (password != askPassword(ENTER_CODE_MSG))
      {
      }
      while (!isPasswordConfirm)
      {
        password = askPassword(NEW_CODE_MSG);
        isPasswordConfirm = askConfirmPassword(password);

        if (isPasswordConfirm)
        {
          savePasswordIntoEeprom(password);
          saveTimerIntoEeprom(EEPROM_TIMER);
          saveUsageIntoEeprom();
          checkSaved = isSaved;
        }
      }
      break;
    }
    default:
      break;
    }

    displayMenu(RESET_MENU);
  }
}

void setupMQTT()
{

  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect(id))
    {

      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setupWifi()
{

  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  espClient.setInsecure();
}

void reconnect()
{
  if (!client.connected())
  {
    while (!client.connected())
    {
      Serial.print("Attempting MQTT connection...");

      if (client.connect(id))
      {
        Serial.println("connected");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
  client.loop();
}

void sendIsActivePayload(String isActive)
{

  StaticJsonDocument<300> Jsonbuffer;
  JsonObject JSONencoder = Jsonbuffer.createNestedObject();
  Serial.println("is alarme active: " + isActive);
  JSONencoder["isAlarmActive"] = isActive;

  char JSONmessageBuffer[100];
  serializeJson(JSONencoder, JSONmessageBuffer);
  Serial.print("msg: ");
  Serial.println(JSONmessageBuffer);
  if (client.publish("alarms/test/activation", JSONmessageBuffer) == true)
  {
    Serial.println("SUCCESS");
  }
  else
  {
    Serial.println("FAIL");
  }
}

void pairAlarm()
{

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Start pair");
    HTTPClient http;
    auto path = "http://" + hostname + ":" + apiPort + "/alarms/pair";
    http.begin(path);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST("{\"id\":\"" + (String)id + "\"}");
    String payload = http.getString();
    http.end();
  }
  else
  {
    Serial.println("Error in WiFi connection");
  }
}

void waitForPairing()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    displayReset();
    LCD.print("Pairing...");
    HTTPClient http;
    int httpCode;
    unsigned long time = millis();
    bool pairingTimedOut = true;
    while (pairingTimedOut) 
    {
      time = millis();
      pairAlarm();
      do
      {
        auto path = "http://" + hostname + ":" + apiPort + "/alarms/exists";
        http.begin(path);
        http.addHeader("Content-Type", "application/json");
        String payload = "{\"id\":\"" + (String)id + "\"}";
        httpCode = http.POST(payload);
        String returnpayload = http.getString();
        http.end();

        delay(1000);
      } while (httpCode != 200 && millis() - time < 11000);
      if (httpCode == 200) {
        pairingTimedOut = false; 
      }
    }
  }
}

void pairing()
{
  bool isPair = false;
  while (!isPair)
  {
    waitForPairing();
    isPair = true;
  }
}