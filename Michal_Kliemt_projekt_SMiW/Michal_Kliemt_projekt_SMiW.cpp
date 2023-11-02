/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLq1kXmPeC"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "TcUuUGeAfk7AXPktK0QfzkJTm4Rkm9cZ"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "esp_sleep.h"

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "UPC5117153";
char pass[] = "Tnumxz7abs6y";

BlynkTimer timer;

#define ADC_PIN 25 
#define WINDOW_PIN 32
#define CABLE_PIN 35

int adc_val;
int window_state;
int cable_state;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

#define ADC_THRESHOLD 2500

// void print_GPIO_wake_up(){
//   uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
//   Serial.print("GPIO that triggered the wake up: GPIO ");
//   Serial.println((log(GPIO_reason))/log(2), 0);
// }

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V4)
{
  int pinValue = param.asInt();
  digitalWrite(2, pinValue);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{

}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
}

void setup()
{
  pinMode(WINDOW_PIN, INPUT);
  pinMode(CABLE_PIN, INPUT);
  
  Serial.begin(115200);
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  // esp_sleep_enable_ext0_wakeup((gpio_num_t)WINDOW_PIN, 0);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)CABLE_PIN, 1);

  adc_val = analogRead(ADC_PIN);
  window_state = digitalRead(WINDOW_PIN);
  cable_state = digitalRead(CABLE_PIN);

  Serial.print("adc val: ");
  Serial.println(adc_val);
  Serial.print("\n");

  // Blynk.begin(auth, ssid, pass);
  // Blynk.run();

  // Serial.print("window state: ");
  // Serial.println(window_state);
  // Serial.print("\n");

  // Serial.print("cable state: ");
  // Serial.println(cable_state);
  // Serial.print("\n");

  Serial.println("Going to sleep now");

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    print_wakeup_reason();
    Serial.print("timeeeeeer\n");

    if (adc_val < ADC_THRESHOLD) {
      Serial.print("adc threshold");
      Blynk.notify("Voltage to low");
    }
  }
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.print("extttttttt\n");
    // uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
    // int pin_num = (log(GPIO_reason))/log(2);

    // Serial.print("Pin num: ");
    // Serial.println(pin_num);
    // Serial.print("\n");

    if (window_state == 1) {
      Serial.print("Window\n");
      Blynk.virtualWrite(V5, window_state); //sending to Blynk
    }
    if (cable_state == 1) {
      Serial.print("Cable\n");
      Blynk.virtualWrite(V4, cable_state); //sending to Blynk
    }
  }

  delay(3000);
  esp_deep_sleep_start();
}

void loop()
{
  // Blynk.run();
  // timer.run();
  // adc_val = analogRead(ADC_PIN);
  // window_state = digitalRead(WINDOW_PIN);
  // cable_state = digitalRead(CABLE_PIN);

  // Serial.println(adc_val);

  // Serial.print("window state: ");
  // Serial.println(window_state);
  // Serial.print("\n");

  // Serial.print("cable state: ");
  // Serial.println(cable_state);
  // Serial.print("\n");

  // delay(1500);


  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
