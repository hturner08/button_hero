#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <iostream>
#include <complex>
#include "fft_psram.h"
class Button{
  public:
  uint32_t state_2_start_time;
  uint32_t button_change_time;    
  uint32_t debounce_duration;
  uint32_t long_press_duration;
  uint8_t pin;
  uint8_t flag;
  bool button_pressed;
  uint8_t state; // This is public for the sake of convenience
  Button(int p) {
  flag = 0;  
    state = 0;
    pin = p;
    state_2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
  }
  void read() {
    uint8_t button_state = digitalRead(pin);  
    button_pressed = !button_state;
  }
int update() {
  read();
  flag = 0;
  int pre_flag = 0;
  if (state==0) {
    if (button_pressed) {
      state = 1;
      button_change_time = millis();
    }
  } else if (state==1) {
    
    if(button_pressed){
      if(millis()-button_change_time > debounce_duration){
        state = 2;
        state_2_start_time = millis();
      }
    }
    else{
      state = 0;
      flag = 0;
      button_change_time = millis();
    }

  } else if (state==2) {
    pre_flag = 1;
    if(button_pressed){
      if(millis()-state_2_start_time > long_press_duration){
        state = 3;
      }
    }else{
      state =4;
      button_change_time = millis();
    }
  } else if (state==3) {
    pre_flag = 2;
    if(button_pressed){
      
    }else{
      state = 4;
      button_change_time = millis();
    }
  } else if (state==4) {        
    // CODE HERE
    if(button_pressed){
      if (millis()-state_2_start_time > long_press_duration){
        state = 3;
       button_change_time = millis();
      }else{
       state = 2;
       button_change_time = millis();
      }
    }else{
      if(millis()-button_change_time > debounce_duration){
        state = 0;
       if (millis()-state_2_start_time > long_press_duration){
       flag = 2;
      }else{
       flag = 1;
      }
      }
    }
  }
  return flag;
}
};
 

const char* CA_CERT = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
                      "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
                      "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
                      "MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
                      "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
                      "hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
                      "v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
                      "eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
                      "tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
                      "C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
                      "zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
                      "mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
                      "V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
                      "bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
                      "3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
                      "J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
                      "291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
                      "ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
                      "AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
                      "TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
                      "-----END CERTIFICATE-----\n";

TFT_eSPI tft = TFT_eSPI();
WiFiClient client2; //global WiFiClient Secure object


TaskHandle_t SendTask;



const int DELAY = 1000;
const int SAMPLE_FREQ = 10000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 1;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = 2*8192;  // number of of samples
int * samples;
std::complex<double> * frequencies; 

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

const char* NETWORK     = "MIT";     // your network SSID (name of wifi network)
const char* PASSWORD = ""; // your network password

const int AUDIO_IN = A0; //pin where microphone is connected
const uint8_t PIN_1 = 5; //button 1
const uint8_t PIN_2 = 0; //button 2

int record_timer;
int record = 0;
int sample_pointer = 1;
int flag1 = 0;
int flag2 = 0;
void setup() {
  Serial.begin(115200);               // Set up serial port
//  sbi(ADCSRA, ADPS2);
//  cbi(ADCSRA, ADPS1);
//  cbi(ADCSRA, ADPS0);
  if(psramFound()){
    Serial.println("PSRAM enabled");
  }else{
    Serial.println("PSRAM not enabled!");
  }
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.setCursor(0,0,1);
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  delay(100); //wait a bit (100 ms)
//  pinMode(PIN_1, INPUT_PULLUP);
//  pinMode(PIN_2, INPUT_PULLUP);
//  pinMode(25,OUTPUT); digitalWrite(25,0);//in case you're controlling your screen with pin 25

  WiFi.begin(NETWORK, PASSWORD); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  xTaskCreatePinnedToCore(
      loop2, /* Function to implement the task */
      "FFT calculator", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &SendTask,  /* Task handle. */
      0); /* Core where the task should run */
  Button record_button = Button(PIN_1);
  record_timer = micros();
  Serial.println(ESP.getFreePsram());
  samples = (int*)ps_malloc(sizeof(int)*NUM_SAMPLES);
  frequencies = (std::complex<double>*)ps_malloc(sizeof(std::complex<double>)*8192);
  Serial.println(ESP.getFreePsram());
  samples[0] = 1551;
  Serial.println("Finished setup");
}

// void shareData(void * pvParameters){
//   if(sample_pointer%(8192) == 0){
//   int mult = sample_pointer/(SAMPLE_FREQ*SAMPLE_DURAION);
//   char request_buffer[200];
//   sprintf(request_buffer, "GET /sandbox/sc/hmturner/cipher.py?topic=%s&t=%d HTTP/1.1\r\n", query,key);
//   strcat(request_buffer, "Host: 608dev-2.net\r\n");
//   strcat("Content-Length: "); client.print(len);
//   strcat("Content-Type: application/x-www-form-urlencoded\r\n\r\n");
//   strcat("\r\n\r\n");
//   for(int i = 0; i < SAMPLE_FREQ*SAMPLE_DURATION){
//     strcat.print("%d",samples[sample_pointer]);
//   }
//   }

// }

//void graphWaves(void pvParameters){
//
//  int pointer2 = 1;
//  while(true){
//    
//  }
//}
void loop2(void * pvParameters){

  while(true){
  if(flag1==1 || flag2==1){
    if(flag1==1){
      fft(samples,frequencies, 8192);
      flag1 = 0;
    }else if(flag2==1){
      fft(samples + 8192, frequencies, 8192);
      flag2 = 0;
    }
    int max_index = 0;
      for(int i = 0; i < 4096; i++){
        if (std::real(frequencies[i]) > std::real(frequencies[max_index])){
          max_index = i;
        }
      }
   Serial.print("Frequency(Hz):");
   Serial.println(max_index-80);
  }
  vTaskDelay(10 / portTICK_PERIOD_MS);
//  Serial.println("loop2");
  }
}

void loop() {
  readMic();
}

void readMic(){ //Read value from microphone
  while(micros()-record_timer < 100){};
  int raw_reading = analogRead(A0);
  record_timer = micros(); 
  int clean_reading = raw_reading-1400;
  samples[sample_pointer] = clean_reading;
  sample_pointer++;
  sample_pointer=sample_pointer%NUM_SAMPLES;
  if(sample_pointer ==0){
     while(flag1==1){  vTaskDelay(50 / portTICK_PERIOD_MS);};
    flag2 = 1;
  }else if(sample_pointer==8192){
    while(flag2==1){  vTaskDelay(50 / portTICK_PERIOD_MS);};
    flag1 = 1;
  }
}

 
