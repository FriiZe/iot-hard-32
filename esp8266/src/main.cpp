#include "main.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);
WiFiClient cli;

// ENVIRONMENT
const char *mqtt_server = "mqtt.potb.dev";
const int mqtt_port = 8883;
const char *wifi_ssid = "SFR_EC38";
const char *wifi_password = "57dqfh4cff36dudg5xdj";
const String hostname = "tuf";
const String apiPort = "3000";
const String id = "90812468-dfa9-4fdf-add6-5113e0a77ca9";

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

/**
 * Set EEPROM, connection WiFi,
 * connection MQTT.
 * Get the usage value.
 * */
void setup() {

  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  // Initialisation LCD
  initLCD();

  // Connexion au WiFi
  //setupWifi();
  // Connexion au server MQTT
  //setupMQTT();

  delay(DELAY);

  // Usage check
  checkSaved = getUsageFromEeprom();
  int timer = getTimerFromEeprom();
  Serial.println("USAGE : " + String(checkSaved));
  Serial.println("TIMER : " + String(timer));
  Serial.println("PWD : " + String(getPasswordFromEeprom()));
}

/**
 * Main program
 * */
void loop() {
  // Not first time usage
  if (checkSaved == isSaved) {

    while (true) {
      key = keypad.getKey();

      if (!isMenuPrinted) {
        displayReset();
        LCD.print(menu[indexMenu]);
        isMenuPrinted = true;
      }

      if (key != NO_KEY) {
        goDown();
        goUp();
        goTo();
      }
      delay(DELAY);
    }

  }
   // First time usage 
  else {

    while (!isPasswordConfirm) {
      password = askPassword(ENTER_CODE_MSG);
      isPasswordConfirm = askConfirmPassword(password);

      if (isPasswordConfirm) {
        savePasswordIntoEeprom(password);
        saveTimerIntoEeprom(EEPROM_TIMER);
        saveUsageIntoEeprom();
        checkSaved = isSaved;
      }
    }  

    /* Demande de renseigner un mot de passe (DONE) */
      /* Demande de confirmation (DONE) */
        /* Enregistrer le mot de passe dans l'eeprom (DONE) */
        /* Enregistrer le timer par défaut dans l'eeprom (DONE) */
        /* Enregistrer que ce n'est plus la première fois dans 
        l'eeprom et dans le programme (DONE) */

    /* Appairage */

    /* Menu */ 
 
      /* Armer l'alarme */
        /* Renseigner le mot de passe pour l'armer */
          /* Désarmer l'alarme */
            /* Renseigner le mot de passe pour la désarmer */
              /* Retour au menu principal */
 
      /* Changer le timer */
        /* Ecrire la valeur dans l'eeprom */
          /* Retour au menu principal */
 
      /* Changer le mot de passe */
        /* Demander de saisir quatre chiffre */
          /* Demander une confirmation */
            /* Enregistrer le mot de passe dans l'eeprom */
              /* Retour au menu principal */
 
      /* Appairage */
        /*  */
          /* Retour au menu pricipal */

  }

}

// LCD methods
/**
 * Initialize the LCD
 * */
void initLCD() {
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
int askPassword(String message) {
  
  displayReset();
  LCD.print(message);

  int index = 0;
  isWrongCode = true;
  currentCode = -1;

  while (isWrongCode) {
      key = keypad.getKey();

      if (key != NO_KEY){
        updateCurrentCode();
        displayStars(index);
      }
      // Check if currentCode is a 4 digit number
      if (currentCode > 999) {
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
bool askConfirmPassword(int code) {

  displayReset();
  LCD.print(CONFIRM_CODE_MSG);

  int index = 0;
  isWrongCode = true;
  currentCode = -1;

  while (isWrongCode) {
    key = keypad.getKey();

    if (key != NO_KEY) {
      updateCurrentCode();
      displayStars(index);
    }

    if (currentCode > 999) {
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
int askTimer() {
  
  int timer = getTimerFromEeprom();
  int isChanging = true;

  displayReset();
  LCD.print("Modify timer: ");
  LCD.setCursor(0,1);
  LCD.print(String(timer));

  while (isChanging) {
    key = keypad.getKey();

    if (key != NO_KEY){

      switch (atoi(&key)) {

        case KEY_UP:
          if (timer > 0) {
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
      LCD.setCursor(0,1);
      LCD.print(String(timer));
    }

    delay(DELAY);
  }

  return timer;
}

/** 
 * Save the password into the EEPROM
 * */
void savePasswordIntoEeprom(int password) {
  EEPROM.put(EEPROM_PASSWORD_ADDRESS, password);
  EEPROM.commit();
}

/**
 * Get the saved password from the EEPROM
 * */
int getPasswordFromEeprom() {
  int password = -1;
  EEPROM.get(EEPROM_PASSWORD_ADDRESS, password);
  return password;
}

/**
 * Save the timer into the EEPROM.
 * */
void saveTimerIntoEeprom(int timer) {
  EEPROM.put(EEPROM_TIMER_ADDRESS, timer);
  EEPROM.commit();
}

/**
 * Get the saved timer from the EEPROM
 * */
int getTimerFromEeprom() {
  int timer = -1;
  EEPROM.get(EEPROM_TIMER_ADDRESS, timer);
  return timer;
}

int getUsageFromEeprom() {
  int usage = -1;
  EEPROM.get(EEPROM_USAGE_ADDRESS, usage);
  return usage;
}

/**
 * Save the usage into the EEPROM.
 * */
void saveUsageIntoEeprom() {
  EEPROM.put(EEPROM_USAGE_ADDRESS, isSaved);
  EEPROM.commit();
}

/**
 * Update the current code
 * */
void updateCurrentCode() {
  if (currentCode == -1) {
    currentCode = atoi(&key);
  } else {
    currentCode = currentCode * 10 + atoi(&key);
  }
}

/**
 * Clear and reset cursor to the start
 * */
void displayReset() {
  LCD.clear();
  LCD.setCursor(0, 0);
}

/**
 * Display stars on LCD on 
 * the second line and at the index.
 * */
void displayStars(int &index) {
  LCD.setCursor(index++, 1);
  LCD.print("*");
}

/**
 * Display the menu at the index
 * */
void displayMenu(int caseDirection) {
  displayReset();

  switch (caseDirection) {
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
void goDown() {
  if (atoi(&key) == KEY_DOWN && indexMenu < MENU_LENGTH - 1) {
    displayMenu(KEY_DOWN);
  }
}

/**
 * Go up to the menu
 * */
void goUp() {
  if (atoi(&key) == KEY_UP && indexMenu > 0) {
    displayMenu(KEY_UP);
  }
}

/**
 * Go into the selected option
 * */
void goTo() {
  if (atoi(&key) == KEY_RETURN) {

    switch (indexMenu) {
      case SET_ALARM_ON:
        {
          int password = getPasswordFromEeprom();
          while (password != askPassword(ENTER_CODE_MSG)) {}
          displayReset();
          LCD.print("Alarm on");
          // TODO Complete this part of code
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

          while (password != askPassword(ENTER_CODE_MSG)) {}
          while (!isPasswordConfirm) {
            password = askPassword(NEW_CODE_MSG);
            isPasswordConfirm = askConfirmPassword(password);

            if (isPasswordConfirm) {
              savePasswordIntoEeprom(password);
              saveTimerIntoEeprom(EEPROM_TIMER);
              saveUsageIntoEeprom();
              checkSaved = isSaved;
            }
          } 
          break;
        }
      
      case PAIR_ALARM:
        {
          // TODO Complete this part of code
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

		if (client.connect("ESP8266Client"))
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
		// Loop until we're reconnected
		while (!client.connected())
		{
			Serial.print("Attempting MQTT connection...");
			// Attempt to connect
			if (client.connect("ESP8266Client"))
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

void publish(String isActive)
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

void httpPost()
{

	if (WiFi.status() == WL_CONNECTED)
	{ //Check WiFi connection status

		HTTPClient http; //Declare object of class HTTPClient

		// Prepare JSON document
		DynamicJsonDocument doc(2048);
		Serial.println("id: " + id);
		doc["id"] = id;

		// Serialize JSON document
		String json;
		serializeJson(doc, json);

		// http.begin(espClient, hostname + ":" + apiPort + "/alarms/pair"); //Specify request destination
		http.begin(hostname + ":" + apiPort + "/alarms/pair"); //Specify request destination
		http.addHeader("Content-Type", "application/json");			   //Specify content-type header

		int httpCode = http.POST(json);	   //Send the request
		String payload = http.getString(); //Get the response payload

		Serial.println("httpcode: " + String(httpCode)); //Print HTTP return code
		Serial.println("payload: " + payload);			  //Print request response payload

		http.end(); //Close connection
	}
	else
	{

		Serial.println("Error in WiFi connection");
	}
}

void httpGet()
{

	if (WiFi.status() == WL_CONNECTED)
	{ //Check WiFi connection status

		HTTPClient http; //Declare an object of class HTTPClient

		delay(10000);

		// http.begin(espClient, hostname + ":" + apiPort + "/alarms/:" + id); //Specify request destination
		http.begin(hostname + ":" + apiPort + "/alarms/:" + id); //Specify request destination
		int httpCode = http.GET();									//Send the request

		Serial.println("http code get: " + String(httpCode));

		if(httpCode == 404){
			while (httpCode == 404)
			{

				httpCode = http.GET();
				Serial.println("http code get: " + String(httpCode));
			}
		}
		
		if (httpCode == 200)
		{ //Check the returning code

			String payload = http.getString(); //Get the request response payload
			Serial.println(payload);		   //Print the response payload
		}

		http.end(); //Close connection
	}
}