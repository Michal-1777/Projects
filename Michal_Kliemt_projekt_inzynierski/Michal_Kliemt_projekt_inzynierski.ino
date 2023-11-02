#include "EEPROM.h"
#include "math.h"
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define przyciskDDR DDRL
#define przycisk PORTL
#define przyciskIN PINL
#define guzik 0

#define ONEWIRE_PIN 46

#define MQ2_PIN 7
#define MQ9_PIN 6

#define WYCIAG1_PIN 37
#define WYCIAG2_PIN 36
#define WYCIAG3_PIN 35

#define NADMUCH1_PIN 34
#define NADMUCH2_PIN 33
#define NADMUCH3_PIN 32

#define DYM_PIN 31
#define CZAD_PIN 30

#define BME1_ADRES 0x77

#define BME2_ADRES 0x76

#define PUNKT_GRANICZNY_CZADU_I_DYMU 400

#define GRANICZNA_WILGOTNOSC 60.0

LiquidCrystal_I2C lcd(0x3F, 16, 2);

Adafruit_BME280 bme1;

Adafruit_BME280 bme2;

uint8_t sensor1[8] = {0x28, 0xB, 0xAF, 0xD3, 0xC0, 0x21, 0x9, 0xF9}; //pokoj
uint8_t sensor2[8] = {0x28, 0xF8, 0x7D, 0xCE, 0xC0, 0x21, 0x9, 0xAA}; //gora
uint8_t sensor3[8] = {0x28, 0xE4, 0xD0, 0xD3, 0xC0, 0x21, 0x9, 0xF6}; //zewnatrz

OneWire onewire(ONEWIRE_PIN);
DallasTemperature sensors(&onewire);

byte aZOgonkiem[8] = {
	0b00000,
	0b00000,
	0b01110,
	0b00001,
	0b01111,
	0b10001,
	0b01111,
	0b00001
};

byte cZKreska[8] = {
	0b00010,
	0b00100,
	0b01110,
	0b10000,
	0b10000,
	0b10001,
	0b01110,
	0b00000
};

byte eZOgonkiem[8] = {
	0b00000,
	0b00000,
	0b01110,
	0b10001,
	0b11110,
	0b10000,
	0b01110,
	0b00100
};

byte lZKreska[8] = {
	0b01100,
	0b00100,
	0b00110,
	0b01100,
	0b00100,
	0b00100,
	0b01110,
	0b00000
};

byte oZkreska[8] = {
	0b00010,
	0b00100,
	0b01110,
	0b10001,
	0b10001,
	0b10001,
	0b01110,
	0b00000
};

byte sZKreska[8] = {
	0b00010,
	0b00100,
	0b01110,
	0b10000,
	0b01110,
	0b00001,
	0b11110,
	0b00000
};

byte zetZKropka[8] = {
	0b00000,
	0b00100,
	0b11111,
	0b00010,
	0b00100,
	0b01000,
	0b11111,
	0b00000
};

byte zetZKreska[8] = {
	0b00010,
	0b00100,
	0b11111,
	0b00010,
	0b00100,
	0b01000,
	0b11111,
	0b00000
};

const byte wiersze = 4;
const byte kolumny = 4;

char przyciski[wiersze][kolumny] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte wierszePiny[wiersze] = {22, 23, 24, 25};
byte kolumnyPiny[kolumny] = {26, 27, 28, 29};

Keypad klawiatura = Keypad( makeKeymap(przyciski), wierszePiny, kolumnyPiny, wiersze, kolumny );

//zmienna przechowujaca aktualny stan przycisku
volatile uint8_t przyciskAktywacjiKlawiatury = 0;

//zmienne sterujace:

//dla wilgotnosci wiekszej od GRANICZNA_WILGOTNOSC
volatile float temperaturaPowyzejWilgotnosciMaksymalna;
volatile float temperaturaPowyzejWilgotnosciMinimalna;

//dla wilgotnosci mniejszej od GRANICZNA_WILGOTNOSC
volatile float temperaturaPonizejWilgotnosciMaksymalna;
volatile float temperaturaPonizejWilgotnosciMinimalna;

volatile int biegWyciagu;
volatile int biegNadmuchu;

//zmienne do przechowywania odczytanej wartosci z czujnikow
volatile float odczytMq2;
volatile float odczytMq9;

volatile float temperaturaZewnetrzna;
volatile float temperaturaGora;
volatile float temperaturaPokoj;
volatile float temperaturaKuchnia;
volatile float temperaturaLazienka;

volatile float cisnienieKuchia; //przechowywane w hPa
volatile float cisnienieLazienka; //przechowywane w hPa

volatile float wilgotnoscKuchnia;
volatile float wilgotnoscLazienka;

volatile float sredniaTemperaturaBezGory;
volatile float sredniaTemperaturaZGora;
volatile float sredniaWilgotnosc;

ISR(TIMER2_OVF_vect){
  if (!(przyciskIN & (1<<guzik)))
	{
		switch(przyciskAktywacjiKlawiatury)
		{
			case 0:
			przyciskAktywacjiKlawiatury = 1;
			break;
			case 1:
			przyciskAktywacjiKlawiatury = 2;
			break;
		}
	}
	else
	{
    switch(przyciskAktywacjiKlawiatury)
    {
      case 3:
      przyciskAktywacjiKlawiatury = 4;
      break;
      case 4:
      przyciskAktywacjiKlawiatury = 0;
      break;
    }
	}
}

void wypiszOdczytySensorowNaLcd(int i);
void wypiszStatusWentylatorowNaLcd();
void wylaczWszystkiePrzekazniki();
void wypiszOpcjeZmian();
void sprawdzUstawioneTemperatury();
void wypiszAktualnieUstawioneWartosciTemperatury();
float ustawNowaWartoscTemperatury(float staraWartoscTemperatury, String komunikat);

void setup(){
  przyciskDDR &= ~(1<<guzik);
  przycisk |= (1<<guzik);
  TCCR2B = (1<<CS22) | (1<<CS20) | (1<<CS20);
	TIMSK2 = (1<<TOIE2);
	sei();

  biegWyciagu = 0;
  biegNadmuchu = 0;

  EEPROM.get(0, temperaturaPowyzejWilgotnosciMaksymalna);
  EEPROM.get(8, temperaturaPowyzejWilgotnosciMinimalna);
  EEPROM.get(16, temperaturaPonizejWilgotnosciMaksymalna);
  EEPROM.get(32, temperaturaPonizejWilgotnosciMinimalna);

  sprawdzUstawioneTemperatury();

  sredniaTemperaturaBezGory = 0.0;
  sredniaWilgotnosc = 0.0;

  klawiatura.setHoldTime(2000);

  lcd.backlight();
  lcd.begin();
  lcd.clear();

  lcd.createChar(0, aZOgonkiem);
  lcd.createChar(1, cZKreska);
  lcd.createChar(2, eZOgonkiem);
  lcd.createChar(3, lZKreska);
  lcd.createChar(4, oZkreska);
  lcd.createChar(5, sZKreska);
  lcd.createChar(6, zetZKropka);
  lcd.createChar(7, zetZKreska);

  sensors.begin();
  sensors.setResolution(10);

  wypiszAktualnieUstawioneWartosciTemperatury();

  lcd.clear();
  lcd.print("Szukam BME280 1");
  delay(2000);
  if (!bme1.begin(BME1_ADRES)) {
    lcd.clear();
    lcd.print("Nie ma BME280 1");
    while (1);
  }
  lcd.clear();
  lcd.print("Znalaz");
  lcd.write((byte)3);
  lcd.print("em");
  lcd.setCursor(0,1);
  lcd.print("BME 280 1");
  delay(2000);

  lcd.clear();
  lcd.print("Szukam BME280 2");
  delay(2000);
  if (!bme2.begin(BME2_ADRES)) {
    lcd.clear();
    lcd.print("Nie ma BME280 2");
    while (1);
    delay(2000);
  }
  lcd.clear();
  lcd.print("Znalaz");
  lcd.write((byte)3);
  lcd.print("em");
  lcd.setCursor(0,1);
  lcd.print("BME 280 2");
  delay(2000);

  //wyciag
  pinMode(WYCIAG1_PIN, OUTPUT);
  pinMode(WYCIAG2_PIN, OUTPUT);
  pinMode(WYCIAG3_PIN, OUTPUT);
  //nadmuch
  pinMode(NADMUCH1_PIN, OUTPUT);
  pinMode(NADMUCH2_PIN, OUTPUT);
  pinMode(NADMUCH3_PIN, OUTPUT);
  //dym
  pinMode(DYM_PIN, OUTPUT);
  //czad
  pinMode(CZAD_PIN, OUTPUT);
  
  //ustawienie na nie działanie
  digitalWrite(WYCIAG1_PIN, HIGH);
  digitalWrite(WYCIAG2_PIN, HIGH);
  digitalWrite(WYCIAG3_PIN, HIGH);
  digitalWrite(NADMUCH1_PIN, HIGH);
  digitalWrite(NADMUCH2_PIN, HIGH);
  digitalWrite(NADMUCH3_PIN, HIGH);
  digitalWrite(DYM_PIN, HIGH);
  digitalWrite(CZAD_PIN, HIGH);

  lcd.clear();
  lcd.print("MQ2 nagrzewanie!");
  lcd.setCursor(0, 1);
  lcd.print("MQ9 nagrzewanie!");
  delay(20000);
}
  
void loop(){
  if(przyciskAktywacjiKlawiatury != 2){
    odczytMq2 = analogRead(MQ2_PIN);
    odczytMq9 = analogRead(MQ9_PIN);

    sensors.requestTemperatures();
    temperaturaPokoj = (float)(((long)(sensors.getTempC(sensor1) * 10L)) / 10.0);
    temperaturaGora = (float)(((long)(sensors.getTempC(sensor2) * 10L)) / 10.0);
    temperaturaZewnetrzna = (float)(((long)(sensors.getTempC(sensor3) * 10L)) / 10.0);

    temperaturaKuchnia = (float)(((long)(bme1.readTemperature() * 10L)) / 10.0);
    temperaturaLazienka = (float)(((long)(bme2.readTemperature() * 10L)) / 10.0);

    cisnienieKuchia = bme1.readPressure() / 100.0;
    cisnienieLazienka = bme2.readPressure() / 100.0;

    wilgotnoscKuchnia = round(bme1.readHumidity());
    wilgotnoscLazienka = round(bme2.readHumidity());

    sredniaTemperaturaBezGory = (temperaturaPokoj + temperaturaKuchnia + temperaturaLazienka) / 3;
    sredniaTemperaturaBezGory = (float)(((long)(sredniaTemperaturaBezGory * 10L)) / 10.0);
    sredniaTemperaturaZGora = (temperaturaGora + temperaturaPokoj + temperaturaKuchnia + temperaturaLazienka) / 4;
    sredniaTemperaturaZGora = (float)(((long)(sredniaTemperaturaZGora * 10L)) / 10.0);
    sredniaWilgotnosc = (wilgotnoscKuchnia + wilgotnoscLazienka) / 2;

    if(odczytMq2 > PUNKT_GRANICZNY_CZADU_I_DYMU){
      wylaczWszystkiePrzekazniki();

      digitalWrite(WYCIAG3_PIN, LOW);
      digitalWrite(DYM_PIN, LOW);

      biegWyciagu = 3;
      biegNadmuchu = 0;

      lcd.clear();
      lcd.print("Wykryto dym!!!");
      lcd.setCursor(0, 1);
      lcd.print("Dym:");
      lcd.setCursor(5, 1);
      lcd.print((int)odczytMq2);
      delay(4000);
      wypiszStatusWentylatorowNaLcd();
    }else if (odczytMq9 > PUNKT_GRANICZNY_CZADU_I_DYMU) {
      wylaczWszystkiePrzekazniki();

      digitalWrite(WYCIAG3_PIN, LOW);
      digitalWrite(NADMUCH3_PIN, LOW);
      digitalWrite(CZAD_PIN, LOW);

      biegWyciagu = 3;
      biegNadmuchu = 3;

      lcd.clear();
      lcd.print("Wykryto czad!!!");
      lcd.setCursor(0, 1);
      lcd.print("Czad:");
      lcd.setCursor(6, 1);
      lcd.print((int)odczytMq9);
      delay(4000);
      wypiszStatusWentylatorowNaLcd();
    }else if(temperaturaGora > 35.0){
      wylaczWszystkiePrzekazniki();

      digitalWrite(WYCIAG3_PIN, LOW);
      digitalWrite(NADMUCH3_PIN, LOW);

      biegWyciagu = 3;
      biegNadmuchu = 3;

      lcd.clear();
      lcd.print("T g");
      lcd.write((byte)4);
      lcd.print("ra > 35.0!!!");
      lcd.setCursor(0, 1);
      lcd.print("Sprawd");
      lcd.write((byte)7);
      lcd.print("!!!");
      delay(4000);
      wypiszStatusWentylatorowNaLcd();
    }else if (sredniaWilgotnosc > GRANICZNA_WILGOTNOSC) {
      if(temperaturaZewnetrzna > temperaturaPowyzejWilgotnosciMaksymalna){
        wylaczWszystkiePrzekazniki();

        digitalWrite(WYCIAG3_PIN, LOW);
        digitalWrite(NADMUCH3_PIN, LOW);

        biegWyciagu = 3;
        biegNadmuchu = 3;

        wypiszStatusWentylatorowNaLcd();
        for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
      }else if (temperaturaZewnetrzna >= temperaturaPowyzejWilgotnosciMinimalna && temperaturaZewnetrzna <= temperaturaPowyzejWilgotnosciMaksymalna) {
        wylaczWszystkiePrzekazniki();

        digitalWrite(WYCIAG2_PIN, LOW);
        digitalWrite(NADMUCH2_PIN, LOW);

        biegWyciagu = 2;
        biegNadmuchu = 2;

        wypiszStatusWentylatorowNaLcd();
        for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
      }else if (temperaturaZewnetrzna < temperaturaPowyzejWilgotnosciMinimalna) {
        wylaczWszystkiePrzekazniki();

        digitalWrite(WYCIAG1_PIN, LOW);
        digitalWrite(NADMUCH1_PIN, LOW);

        biegWyciagu = 1;
        biegNadmuchu = 1;

        wypiszStatusWentylatorowNaLcd();
        for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
      }
    }else if (temperaturaLazienka > 25.0 || temperaturaKuchnia > 25.0) {
      wylaczWszystkiePrzekazniki();

      digitalWrite(WYCIAG3_PIN, LOW);
      digitalWrite(NADMUCH3_PIN, LOW);

      biegWyciagu = 3;
      biegNadmuchu = 3;

      wypiszStatusWentylatorowNaLcd();
      for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
    }else if (sredniaWilgotnosc <= GRANICZNA_WILGOTNOSC){
      if(temperaturaZewnetrzna > temperaturaPonizejWilgotnosciMaksymalna){
        wylaczWszystkiePrzekazniki();

        digitalWrite(WYCIAG3_PIN, LOW);
        digitalWrite(NADMUCH3_PIN, LOW);

        biegWyciagu = 3;
        biegNadmuchu = 3;

        wypiszStatusWentylatorowNaLcd();
        for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
      }else if (temperaturaZewnetrzna >= temperaturaPonizejWilgotnosciMinimalna && temperaturaZewnetrzna <= temperaturaPonizejWilgotnosciMaksymalna) {
        wylaczWszystkiePrzekazniki();

        digitalWrite(WYCIAG2_PIN, LOW);
        digitalWrite(NADMUCH2_PIN, LOW);

        biegWyciagu = 2;
        biegNadmuchu = 2;

        wypiszStatusWentylatorowNaLcd();
        for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
      }else if (temperaturaZewnetrzna < temperaturaPonizejWilgotnosciMinimalna) {
        wylaczWszystkiePrzekazniki();

        digitalWrite(WYCIAG1_PIN, LOW);
        digitalWrite(NADMUCH1_PIN, LOW);

        biegWyciagu = 1;
        biegNadmuchu = 1;

        wypiszStatusWentylatorowNaLcd();
        for(int i = 0; i < 7; i++) wypiszOdczytySensorowNaLcd(i);
      }
    }
  }else if(przyciskAktywacjiKlawiatury == 2){
    wypiszAktualnieUstawioneWartosciTemperatury();

    lcd.clear();
    lcd.print("Czy chesz doko-");
    lcd.setCursor(0,1);
    lcd.print("na");
    lcd.write((byte)1);
    lcd.print(" zmiany?");
    delay(3000);

    lcd.clear();
    lcd.print("A - tak");
    lcd.setCursor(0,1);
    lcd.print("C - nie");

    char wyborUzytkownika = 'x';
    while(wyborUzytkownika != 'A' && wyborUzytkownika != 'C'){
      wyborUzytkownika = klawiatura.waitForKey();
    }

    if(wyborUzytkownika == 'A'){
      wypiszOpcjeZmian();
      char wyborAkcji = '0';
      while(wyborAkcji != 'C'){
        wyborAkcji = klawiatura.waitForKey();
        switch (wyborAkcji){
          case '1': {
            String komunikat = "Nowa T max pow W";
            lcd.clear();
            lcd.print("Nowa wart dla:");
            lcd.setCursor(0,1);
            lcd.print("T max pow W");
            delay(3000);
            lcd.clear();
            lcd.print(komunikat);
            temperaturaPowyzejWilgotnosciMaksymalna = ustawNowaWartoscTemperatury(temperaturaPowyzejWilgotnosciMaksymalna, komunikat);
            EEPROM.put(0, temperaturaPowyzejWilgotnosciMaksymalna);
            sprawdzUstawioneTemperatury();
            wypiszAktualnieUstawioneWartosciTemperatury();
            wypiszOpcjeZmian();
            break;
          }
          case '2': {
            String komunikat = "Nowa T min pow W";
            lcd.clear();
            lcd.print("Nowa wart dla:");
            lcd.setCursor(0,1);
            lcd.print("T min pow W");
            delay(3000);
            lcd.clear();
            lcd.print(komunikat);
            temperaturaPowyzejWilgotnosciMinimalna = ustawNowaWartoscTemperatury(temperaturaPowyzejWilgotnosciMinimalna, komunikat);
            EEPROM.put(8, temperaturaPowyzejWilgotnosciMinimalna);
            sprawdzUstawioneTemperatury();
            wypiszAktualnieUstawioneWartosciTemperatury();
            wypiszOpcjeZmian();
            break;
          }
          case '3': {
            String komunikat = "Nowa T max pon W";
            lcd.clear();
            lcd.print("Nowa wart dla:");
            lcd.setCursor(0,1);
            lcd.print("T max pon W");
            delay(3000);
            lcd.clear();
            lcd.print(komunikat);
            temperaturaPonizejWilgotnosciMaksymalna = ustawNowaWartoscTemperatury(temperaturaPonizejWilgotnosciMaksymalna, komunikat);
            EEPROM.put(16, temperaturaPonizejWilgotnosciMaksymalna);
            sprawdzUstawioneTemperatury();
            wypiszAktualnieUstawioneWartosciTemperatury();
            wypiszOpcjeZmian();
            break;
          }
          case '4': {
            String komunikat = "Nowa T min pon W";
            lcd.clear();
            lcd.print("Nowa wart dla:");
            lcd.setCursor(0,1);
            lcd.print("T min pon W");
            delay(3000);
            lcd.clear();
            lcd.print(komunikat);
            temperaturaPonizejWilgotnosciMinimalna = ustawNowaWartoscTemperatury(temperaturaPonizejWilgotnosciMinimalna, komunikat);
            EEPROM.put(32, temperaturaPonizejWilgotnosciMinimalna);
            sprawdzUstawioneTemperatury();
            wypiszAktualnieUstawioneWartosciTemperatury();
            wypiszOpcjeZmian();
            break;
          }
          case 'C': {
            break;
          }
          default: {
            lcd.clear();
            lcd.write((byte)7);
            lcd.print("le! Wybierz");
            lcd.setCursor(0,1);
            lcd.print("ponownie!");
          }
        }
      }
    }
    przyciskAktywacjiKlawiatury = 3;
  }
}

float ustawNowaWartoscTemperatury(float staraWartoscTemperatury, String komunikat){
  String nowaTemperaturaString = "";
  lcd.setCursor(0,1);
  char wyborZnaku = '0';
  while(wyborZnaku != 'A' && wyborZnaku != 'C'){
    wyborZnaku = klawiatura.waitForKey();
    switch (wyborZnaku){
      case '1': {
        nowaTemperaturaString += '1';
        lcd.print('1');
        break;
      }
      case '2': {
        nowaTemperaturaString += '2';
        lcd.print('2');
        break;
      }
      case '3': {
        nowaTemperaturaString += '3';
        lcd.print('3');
        break;
      }
      case '4': {
        nowaTemperaturaString += '4';
        lcd.print('4');
        break;
      }
      case '5': {
        nowaTemperaturaString += '5';
        lcd.print('5');
        break;
      }
      case '6': {
        nowaTemperaturaString += '6';
        lcd.print('6');
        break;
      }
      case '7': {
        nowaTemperaturaString += '7';
        lcd.print('7');
        break;
      }
      case '8': {
        nowaTemperaturaString += '8';
        lcd.print('8');
        break;
      }
      case '9': {
        nowaTemperaturaString += '9';
        lcd.print('9');
        break;
      }
      case '0': {
        nowaTemperaturaString += '0';
        lcd.print('0');
        break;
      }
      case '*': {
        nowaTemperaturaString += '.';
        lcd.print('.');
        break;
      }
      case '#': {
        if(nowaTemperaturaString.length() == 0){
          nowaTemperaturaString += '-';
          lcd.print('-');
        }
        break;
      }
      case 'A': {
        break;
      }
      case 'B': {
        nowaTemperaturaString = "";
        lcd.clear();
        lcd.print(komunikat);
        lcd.setCursor(0,1);
        break;
      }
      case 'C': {
        break;
      }
      default: {
      }
    }
  }
  if(wyborZnaku == 'C' || nowaTemperaturaString.length() == 0){
    return staraWartoscTemperatury;
  }
  if(nowaTemperaturaString[0] == "."){
    String tmp = nowaTemperaturaString;
    nowaTemperaturaString = "0";
    nowaTemperaturaString += tmp;
  }else if (nowaTemperaturaString[0] == "-" && nowaTemperaturaString[1] == "."){
    String tmp = nowaTemperaturaString;
    nowaTemperaturaString = "-0";
    nowaTemperaturaString += tmp.substring(2);
  }
  return nowaTemperaturaString.toFloat();
}

void wypiszAktualnieUstawioneWartosciTemperatury(){
    lcd.clear();
    lcd.print("Aktualnie usta-");
    lcd.setCursor(0,1);
    lcd.print("wione warto");
    lcd.write((byte)5);
    lcd.print("ci:");
    delay(3000);

    lcd.clear();
    lcd.print("Max T pow W:");
    lcd.setCursor(0,1);
    lcd.print(temperaturaPowyzejWilgotnosciMaksymalna, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(3000);

    lcd.clear();
    lcd.print("Min T pow W:");
    lcd.setCursor(0,1);
    lcd.print(temperaturaPowyzejWilgotnosciMinimalna, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(3000);

    lcd.clear();
    lcd.print("Max T pon W:");
    lcd.setCursor(0,1);
    lcd.print(temperaturaPonizejWilgotnosciMaksymalna, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(3000);

    lcd.clear();
    lcd.print("Min T pon W:");
    lcd.setCursor(0,1);
    lcd.print(temperaturaPonizejWilgotnosciMinimalna, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(3000);
}

void sprawdzUstawioneTemperatury(){
  if(temperaturaPowyzejWilgotnosciMinimalna > temperaturaPowyzejWilgotnosciMaksymalna){
    float tmp = temperaturaPowyzejWilgotnosciMinimalna;
    temperaturaPowyzejWilgotnosciMinimalna = temperaturaPowyzejWilgotnosciMaksymalna;
    temperaturaPowyzejWilgotnosciMaksymalna = tmp;
    EEPROM.put(0, temperaturaPowyzejWilgotnosciMaksymalna);
    EEPROM.put(8, temperaturaPowyzejWilgotnosciMinimalna);
  }
  if(temperaturaPonizejWilgotnosciMinimalna > temperaturaPonizejWilgotnosciMaksymalna){
    float tmp = temperaturaPonizejWilgotnosciMinimalna;
    temperaturaPonizejWilgotnosciMinimalna = temperaturaPonizejWilgotnosciMaksymalna;
    temperaturaPonizejWilgotnosciMaksymalna = tmp;
    EEPROM.put(16, temperaturaPonizejWilgotnosciMaksymalna);
    EEPROM.put(32, temperaturaPonizejWilgotnosciMinimalna);
  }
}

void wypiszOpcjeZmian(){
      lcd.clear();
      lcd.print("Wybierz co");
      lcd.setCursor(0,1);
      lcd.print("chces zmieni");
      lcd.write((byte)1);
      lcd.print(":");
      delay(3000);

      lcd.clear();
      lcd.print("1. T max pow W");
      lcd.setCursor(0,1);
      lcd.print("2. T min pow W");
      delay(3000);

      lcd.clear();
      lcd.print("3. T max pon W");
      lcd.setCursor(0,1);
      lcd.print("4. T min pon W");
      delay(3000);

      lcd.clear();
      lcd.print("C. Wyj");
      lcd.write((byte)5);
      lcd.print("cie");
}

void wylaczWszystkiePrzekazniki(){
  digitalWrite(WYCIAG1_PIN, HIGH);
  digitalWrite(WYCIAG2_PIN, HIGH);
  digitalWrite(WYCIAG3_PIN, HIGH);
  digitalWrite(NADMUCH1_PIN, HIGH);
  digitalWrite(NADMUCH2_PIN, HIGH);
  digitalWrite(NADMUCH3_PIN, HIGH);
  digitalWrite(DYM_PIN, HIGH);
  digitalWrite(CZAD_PIN, HIGH);
}

void wypiszStatusWentylatorowNaLcd(){
  lcd.clear();
  lcd.print("Wyci");
  lcd.write((byte)0);
  lcd.print("g: ");
  lcd.setCursor(9,0);
  lcd.print(biegWyciagu);

  lcd.setCursor(0,1);
  lcd.print("Nadmuch: ");
  lcd.setCursor(9,1);
  lcd.print(biegNadmuchu);
  delay(5000);
}

void wypiszOdczytySensorowNaLcd(int i){
  if(i == 0){
    lcd.clear();
    lcd.print("Dym:");
    lcd.setCursor(6, 0);
    lcd.print((int)odczytMq2);
    lcd.setCursor(0, 1);
    lcd.print("Czad:");
    lcd.setCursor(6, 1);
    lcd.print((int)odczytMq9);
    delay(4000);
  }else if (i == 1) {
    lcd.clear();
    lcd.print("T ");
    lcd.write((byte)5);
    lcd.print("red b:");
    lcd.print(sredniaTemperaturaBezGory, 1);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("T ");
    lcd.write((byte)5);
    lcd.print("red z:");
    lcd.print(sredniaTemperaturaZGora, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(5000);
  }else if (i == 2) {
    lcd.clear();
    lcd.print("T zew:");
    lcd.print(temperaturaZewnetrzna, 1);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("T g");
    lcd.write((byte)4);
    lcd.print("ra:");
    lcd.print(temperaturaGora, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(5000);
  }else if (i == 3) {
    lcd.clear();
    lcd.print("T pok:");
    lcd.print(temperaturaPokoj, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(4000);
  }else if (i == 4) {
    lcd.clear();
    lcd.print("T kuch:");
    lcd.print(temperaturaKuchnia, 1);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("T ");
    lcd.write((byte)3);
    lcd.print("az:");
    lcd.print(temperaturaLazienka, 1);
    lcd.print((char)223);
    lcd.print("C");
    delay(5000);
  }else if (i == 5) {
    lcd.clear();
    lcd.print("P kuch:");
    lcd.print(cisnienieKuchia, 1);
    lcd.print("hPa");
    lcd.setCursor(0,1);
    lcd.print("P ");
    lcd.write((byte)3);
    lcd.print("az:");
    lcd.print(cisnienieLazienka, 1);
    lcd.print("hPa");
    delay(5000);
  }else if (i == 6) {
    lcd.clear();
    lcd.print("W kuch:");
    lcd.print((int)wilgotnoscKuchnia);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("W ");
    lcd.write((byte)3);
    lcd.print("az:");
    lcd.print((int)wilgotnoscLazienka);
    lcd.print("%");
    delay(5000);
  }
} 