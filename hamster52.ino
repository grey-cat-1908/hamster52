#include <LiquidCrystal.h>
 
#define PIN_RS 6
#define PIN_EN 7
#define PIN_DB4 8
#define PIN_DB5 9
#define PIN_DB6 10
#define PIN_DB7 11

#define PIN_CLICK 3
#define PIN_CHANGE 2

#define MAX_BALANCE 4294967295 // MAX: 4294967295
#define UPGRADE_FACTOR 2
#define UPDATE_INTERVAL 1000

unsigned long previousMillis = 0; 
unsigned long balance = 0, coinsPerClick = 1, coinsPerSecond = 1;
unsigned long cpsCost = 50, cpcCost = 10;
uint8_t mode = 0;
volatile bool clickPressedFlag = false, changePressedFlag = false;
bool clickState = false, changeState = false;
bool lastClickState = false, lastChangeState = false;

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7);

String formatNumber(unsigned long num) {
  char buffer[20];
  if (num >= 1000000000) {
    float formattedNum = floor(num / 1000000000.0 * 100) / 100;
    dtostrf(formattedNum, 6, 2, buffer);
    strcat(buffer, "B");
  } else if (num >= 1000000) {
    float formattedNum = floor(num / 1000000.0 * 100) / 100;
    dtostrf(formattedNum, 6, 2, buffer);
    strcat(buffer, "M");
  } else if (num >= 1000) {
    float formattedNum = floor(num / 1000.0 * 100) / 100;
    dtostrf(formattedNum, 6, 2, buffer);
    strcat(buffer, "K");
  } else {
    sprintf(buffer, "%lu", num);
  }
  return String(buffer);
}

void handleClickPress() {
  clickPressedFlag = true;
}

void handleChangePress() {
  changePressedFlag = true;
}

void runFirstScreen() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("HAMSTER KOMBAT");
  lcd.setCursor(3, 1);
  lcd.print("52 EDITION");

  delay(5000);
}

void setup() {
  pinMode(PIN_CLICK, INPUT_PULLUP);
  pinMode(PIN_CHANGE, INPUT_PULLUP);

  Serial.begin(9600);
  lcd.begin(16, 2);

  runFirstScreen();

  attachInterrupt(digitalPinToInterrupt(PIN_CLICK), handleClickPress, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_CHANGE), handleChangePress, FALLING);
}
 
void loop() {
  unsigned long currentMillis = millis();

  if (MAX_BALANCE - balance <= coinsPerClick || MAX_BALANCE - balance <= coinsPerSecond) {
    balance = 0, coinsPerClick = 1, coinsPerSecond = 1;
    cpsCost = 50, cpcCost = 10;
    mode = 0;

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("ERROR OCCURRED");
    lcd.setCursor(1, 1);
    lcd.print("SYSTEM RESTART");

    delay(5000);

    runFirstScreen();
  }

  if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = currentMillis;
    updateBalance();
    showData();
  }

  clickState = digitalRead(PIN_CLICK) == LOW;
  changeState = digitalRead(PIN_CHANGE) == LOW;

  if (clickState && !lastClickState && clickPressedFlag) {
    clickPressedFlag = false;
    if (mode == 0) {
      balance += coinsPerClick;
    } else if (mode == 1 && balance >= cpcCost) {
      balance -= cpcCost;
      cpcCost *= UPGRADE_FACTOR;
      coinsPerClick *= UPGRADE_FACTOR;
    } else if (mode == 2 && balance >= cpsCost) {
      balance -= cpsCost;
      cpsCost *= UPGRADE_FACTOR;
      coinsPerSecond *= UPGRADE_FACTOR;
    }
    showData();
  }

  if (changeState && !lastChangeState && changePressedFlag) {
    changePressedFlag = false;
    mode = (mode + 1) % 3;
    showData();
  }

  lastClickState = clickState;
  lastChangeState = changeState;
}

void updateBalance() {
  balance += coinsPerSecond;
}

void showData() {
    lcd.clear();
    if (mode == 0) {
      String toShow = "$ " + formatNumber(balance);
      lcd.setCursor(4, 0);
      lcd.print("BALANCE:");
      lcd.setCursor((16 - toShow.length()) / 2, 1);
      lcd.print(toShow);
    } else if (mode == 1 || mode == 2) {
      String firstLine = (mode == 1) ? "> C/c: " : "C/c: ";
      String secondLine = (mode == 2) ? "> C/s: " : "C/s: ";
      
      firstLine += (balance >= cpcCost) ? "Upgrade ^" : "$ " + formatNumber(cpcCost);
      secondLine += (balance >= cpsCost) ? "Upgrade ^" : "$ " + formatNumber(cpsCost);

      lcd.setCursor(0, 0);
      lcd.print(firstLine);
      lcd.setCursor(0, 1);
      lcd.print(secondLine);
    }
}