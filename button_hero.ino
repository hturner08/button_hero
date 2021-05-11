#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <ArduinoJson.h>  
#include <SavLayFilter.h>
#define ARDUINOJSON_USE_DOUBLE 0
#include "fft_psram.h"

TFT_eSPI tft = TFT_eSPI();
#define BACK TFT_BLACK
#define TEXT TFT_WHITE
#define NOTE TFT_GREEN

const char USER[] = "umagana"; //CHANGE YOUR USER VARIABLE!!!
const char GET_URL[] = "GET http://608dev-2.net/sandbox/sc/team27/button_hero_server/data_to_esp32.py HTTP/1.1\r\n";
const char network[] = "MIT";
const char password[] = "";
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
WiFiClient client2; //global WiFiClient Secure object

const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //ms to wait between posting step

const uint16_t IN_BUFFER_SIZE = 3000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 3000; //size of buffer to hold HTTP response
char request[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
char old_response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const int SCREEN_HEIGHT = 160;
const int SCREEN_WIDTH = 128;
const int midx = 70; // middle of screen is 40% of screen height and width
const int midy = 50;

const int BUTTON1 = 5;
const int BUTTON2 = 19; // RX
uint8_t AUDIO_TRANSDUCER = 26;
uint8_t AUDIO_PWM = 1;

const int LOOP_PERIOD = 40;

const int IDLE = 0;
const int SELECT = 1;
const int PLAY = 2;
const int TO_SCORE = 3;
const int SCORE = 4;

char text[50];
char note[10];

uint8_t state;
uint8_t old_state;
float old_note = 0;
float new_note = 0;
float sec_note;
float thi_note;
float E = 0.02; // buffer to allow in notes (1-E to 1+E)

uint8_t i = 0;
uint32_t countdown;
uint32_t primary_timer;
uint32_t song_timer;
uint32_t get_timer;

const int AUDIO_IN = A0; //pin where microphone is connected
const uint8_t PIN_1 = 5; //button 1
const uint8_t PIN_2 = 0; //button 2

float score;

//FFT Variables
const int DELAY = 1000;
const int SAMPLE_FREQ = 4096;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 120;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES =SAMPLE_DURATION*SAMPLE_FREQ;  // number of of samples
int * samples;
std::complex<double> * frequencies; 
////double clean_reading;
//SavLayFilter smallFilter (&clean_reading, 0, 5);  

TaskHandle_t RecordTask;
TaskHandle_t FFTTask;
double notes[7] = {440.00,493.88, 523.25,587.33,659.25,698.46,783.99};
char letters[7] = {'A','B','C','D','E','F','G'};

int record_timer;
int speed_timer;
//int state = 0;
int sample_pointer = 1;
int flags[SAMPLE_DURATION] = {0};
char melody[SAMPLE_DURATION];
int pitches[SAMPLE_DURATION];

// ALL NOTES in Hz (names are written with the letter O because some letter-number combinations are already C preset variable names)
float CO0 = 16.35;
float CS0 = CO0*pow(2,1.0/12.0);
float DO0 = CO0*pow(2,2.0/12.0);
float DS0 = CO0*pow(2,3.0/12.0);
float EO0 = CO0*pow(2,4.0/12.0);
float FO0 = CO0*pow(2,5.0/12.0);
float FS0 = CO0*pow(2,6.0/12.0);
float GO0 = CO0*pow(2,7.0/12.0);
float GS0 = CO0*pow(2,8.0/12.0);
float AO0 = CO0*pow(2,9.0/12.0);
float AS0 = CO0*pow(2,10.0/12.0);
float BO0 = CO0*pow(2,11.0/12.0);
float CO1 = CO0*pow(2,12.0/12.0);
float CS1 = CO0*pow(2,13.0/12.0);
float DO1 = CO0*pow(2,14.0/12.0);
float DS1 = CO0*pow(2,15.0/12.0);
float EO1 = CO0*pow(2,16.0/12.0);
float FO1 = CO0*pow(2,17.0/12.0);
float FS1 = CO0*pow(2,18.0/12.0);
float GO1 = CO0*pow(2,19.0/12.0);
float GS1 = CO0*pow(2,20.0/12.0);
float AO1 = CO0*pow(2,21.0/12.0);
float AS1 = CO0*pow(2,22.0/12.0);
float BO1 = CO0*pow(2,23.0/12.0);
float CO2 = CO0*pow(2,24.0/12.0);
float CS2 = CO0*pow(2,25.0/12.0);
float DO2 = CO0*pow(2,26.0/12.0);
float DS2 = CO0*pow(2,27.0/12.0);
float EO2 = CO0*pow(2,28.0/12.0);
float FO2 = CO0*pow(2,29.0/12.0);
float FS2 = CO0*pow(2,30.0/12.0);
float GO2 = CO0*pow(2,31.0/12.0);
float GS2 = CO0*pow(2,32.0/12.0);
float AO2 = CO0*pow(2,33.0/12.0);
float AS2 = CO0*pow(2,34.0/12.0);
float BO2 = CO0*pow(2,35.0/12.0);
float CO3 = CO0*pow(2,36.0/12.0);
float CS3 = CO0*pow(2,37.0/12.0);
float DO3 = CO0*pow(2,38.0/12.0);
float DS3 = CO0*pow(2,39.0/12.0);
float EO3 = CO0*pow(2,40.0/12.0);
float FO3 = CO0*pow(2,41.0/12.0);
float FS3 = CO0*pow(2,42.0/12.0);
float GO3 = CO0*pow(2,43.0/12.0);
float GS3 = CO0*pow(2,44.0/12.0);
float AO3 = CO0*pow(2,45.0/12.0);
float AS3 = CO0*pow(2,46.0/12.0);
float BO3 = CO0*pow(2,47.0/12.0);
float CO4 = CO0*pow(2,48.0/12.0);
float CS4 = CO0*pow(2,49.0/12.0);
float DO4 = CO0*pow(2,50.0/12.0);
float DS4 = CO0*pow(2,51.0/12.0);
float EO4 = CO0*pow(2,52.0/12.0);
float FO4 = CO0*pow(2,53.0/12.0);
float FS4 = CO0*pow(2,54.0/12.0);
float GO4 = CO0*pow(2,55.0/12.0);
float GS4 = CO0*pow(2,56.0/12.0);
float AO4 = CO0*pow(2,57.0/12.0);
float AS4 = CO0*pow(2,58.0/12.0);
float BO4 = CO0*pow(2,59.0/12.0);
float CO5 = CO0*pow(2,60.0/12.0);
float CS5 = CO0*pow(2,61.0/12.0);
float DO5 = CO0*pow(2,62.0/12.0);
float DS5 = CO0*pow(2,63.0/12.0);
float EO5 = CO0*pow(2,64.0/12.0);
float FO5 = CO0*pow(2,65.0/12.0);
float FS5 = CO0*pow(2,66.0/12.0);
float GO5 = CO0*pow(2,67.0/12.0);
float GS5 = CO0*pow(2,68.0/12.0);
float AO5 = CO0*pow(2,69.0/12.0);
float AS5 = CO0*pow(2,70.0/12.0);
float BO5 = CO0*pow(2,71.0/12.0);
float CO6 = CO0*pow(2,72.0/12.0);
float CS6 = CO0*pow(2,73.0/12.0);
float DO6 = CO0*pow(2,74.0/12.0);
float DS6 = CO0*pow(2,75.0/12.0);
float EO6 = CO0*pow(2,76.0/12.0);
float FO6 = CO0*pow(2,77.0/12.0);
float FS6 = CO0*pow(2,78.0/12.0);
float GO6 = CO0*pow(2,79.0/12.0);
float GS6 = CO0*pow(2,80.0/12.0);
float AO6 = CO0*pow(2,81.0/12.0);
float AS6 = CO0*pow(2,82.0/12.0);
float BO6 = CO0*pow(2,83.0/12.0);
float CO7 = CO0*pow(2,84.0/12.0);
float CS7 = CO0*pow(2,85.0/12.0);
float DO7 = CO0*pow(2,86.0/12.0);
float DS7 = CO0*pow(2,87.0/12.0);
float EO7 = CO0*pow(2,88.0/12.0);
float FO7 = CO0*pow(2,89.0/12.0);
float FS7 = CO0*pow(2,90.0/12.0);
float GO7 = CO0*pow(2,91.0/12.0);
float GS7 = CO0*pow(2,92.0/12.0);
float AO7 = CO0*pow(2,93.0/12.0);
float AS7 = CO0*pow(2,94.0/12.0);
float BO7 = CO0*pow(2,95.0/12.0);
float CO8 = CO0*pow(2,96.0/12.0);
float CS8 = CO0*pow(2,97.0/12.0);
float DO8 = CO0*pow(2,98.0/12.0);
float DS8 = CO0*pow(2,99.0/12.0);
float EO8 = CO0*pow(2,100.0/12.0);
float FO8 = CO0*pow(2,101.0/12.0);
float FS8 = CO0*pow(2,102.0/12.0);
float GO8 = CO0*pow(2,103.0/12.0);
float GS8 = CO0*pow(2,104.0/12.0);
float AO8 = CO0*pow(2,105.0/12.0);
float AS8 = CO0*pow(2,106.0/12.0);
float BO8 = CO0*pow(2,107.0/12.0);

// used for game when displaying next notes to play
class Song {
  public:
    char title[50];
    char artist[50];
    float notes[500]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
    int length; //number of notes (essentially length of array).
    float note_period; // once we get bpm and fraction from actual song (quarter notes, eight notes, etc.), change them using conversion: note_period = 60.0*1000/bpm/FACTOR
  // FACTOR is (# notes needed per measure/ fraction of notes in beat). For example, # notes needed per measure is often 16 or 32, and typically bpm counts the song's quarter notes, so FACTOR = 16/4 or 32/4
};

//Representing the actual song
Song lights = {"All of the Lights", "Kanye West", {CO4, 0.0, CO4, CO4, CO4, CO4, CO4, 0.0, CO4, 0.0, EO4, EO4, EO4, EO4, AO4, 0.0, GO4, 0.0, GO4, GO4, GO4, GO4, 369.99, 0.0, EO4, 0.0, EO4, EO4, EO4, EO4, EO4, 0.0}, 32, 208.3};
Song danza = {"Danza Kuduro", "Don Omar", {CO4, EO4, 0.0, AO4, 0.0, FO4, GO4, 0.0, CO5, 0.0, CO5, 0.0, BO4, 0.0, CO5, 0.0, CO4, DO4, 0.0, FO4, 0.0, EO4, FO4, 0.0, AO4, 0.0, AO4, 0.0, GO4, 0.0, GO4, 0.0, CO4, EO4, 0.0, GO4, 0.0, EO4, GO4, 0.0, EO5, 0.0, EO5, 0.0, DO5, 0.0, DO5, 0.0, CO4, EO4, 0.0, GO4, 0.0, EO4, GO4, 0.0, BO4, 0.0, BO4, 0.0, BO4, 0.0, CO5, 0.0}, 64, 115.38};
Song song_to_play;  //initialize song for GET request

//Representing user's instrument with some mistakes while playing
//This will be built from the microphone 
Song song_to_compare = {"user_song", "user", { CO3, EO3, 0.0, AO3, 0.0, FO3, GO3, 0.0, CO4, 0.0, CO4, 0.0, BO4, 0.0, BO4, 0.0, CO3, DO3, 0.0, FO3, 0.0, EO3, FO3, 0.0, AO3, 0.0, AO3, 0.0, GO3, 0.0, GO3, 0.0, CO3, EO3, 0.0, GO3, 0.0, EO3, GO3, 0.0, EO4, 0.0, EO4, 0.0, DO4, 0.0, DO4, 0.0, CO3, EO3, 0.0, GO3, 0.0, EO3, GO3, 0.0, BO3, 0.0, BO3, 0.0, BO3, 0.0, CO4, 0.0}, 64, 115.38};

class Button {
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
      if (state == 0) {
        if (button_pressed) {
          state = 1;
          button_change_time = millis();
        }
      } else if (state == 1) {
        if (button_pressed && millis() - button_change_time >= debounce_duration) {
          state = 2;
          state_2_start_time = millis();
        } else if (!button_pressed) {
          state = 0;
          button_change_time = millis();
        }
      } else if (state == 2) {
        if (not button_pressed) {
          state = 4;
          button_change_time = millis();
        } else if (button_pressed && millis() - state_2_start_time >= long_press_duration) {
          state = 3;
        }
      } else if (state == 3) {
        if (not button_pressed) {
          state = 4;
          button_change_time = millis();
        }
      } else if (state == 4) {
        if (button_pressed && millis() - state_2_start_time < long_press_duration) {
          state = 2;
          button_change_time = millis();
        } else if (button_pressed && millis() - state_2_start_time >= long_press_duration) {
          state = 3;
          button_change_time = millis();
        } else if (not button_pressed && millis() - button_change_time >= debounce_duration) {
          if (millis() - state_2_start_time < long_press_duration) {
            flag = 1;
          } else {
            flag = 2;
          }
          state = 0;
        }
      }
      return flag;
    }
};

Button play(BUTTON1); //button object!
Button song(BUTTON2); //button object!

void setup() {
  Serial.begin(115200); //begin serial comms
  if(psramFound()){
    Serial.println("PSRAM enabled");
  }else{
    Serial.println("PSRAM not enabled!");
  }
  //PRINT OUT WIFI NETWORKS NEARBY
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
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
  
  tft.init();  //init screen
  tft.setRotation(1); //adjust rotation
  tft.setTextColor(TEXT, BACK); //set color for font
  strcpy(text,"BUTTON");
  screen_set(1, 2, 40, text);
  strcpy(text,"HERO");
  screen_set(0, 2, 60, text);
  strcpy(text,"Waiting for");
  screen_set(0, 1, 80, text);
  strcpy(text,"song...");
  screen_set(0, 1, 90, text);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  state = 0;
  old_state = 0;
  record_timer = micros();
  samples = (int*)ps_malloc(sizeof(int)*NUM_SAMPLES);
  frequencies = (std::complex<double>*)ps_malloc(sizeof(std::complex<double>)*4092);
//  Serial.println(ESP.getFreePsram());
  samples[0] = 1500;
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);
  Serial.println("Finished setup");
  get_timer = millis();
}

void loop() {
  int p = play.update(); // 1 if button 1 pressed, 2 if button 1 held
  int s = song.update(); // 1 if button 2 pressed, 2 if button 2 held
  state = screen_update(state, old_state, p, s);
  if(state != old_state) {
    i++;
  } if(i == 2) {
    old_state = state;
    i = 0;
  }
  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}

uint8_t screen_update(uint8_t state, uint8_t old_state, int play, int song) {
  switch(state) {
    case IDLE:
    // GET request here every 1 or 2 seconds, checks for new songs, get information about the song (title, artist, notes, bpm)
      if(state != old_state) {
        strcpy(text,"BUTTON");
        screen_set(1, 2, 40, text);
        strcpy(text,"HERO");
        screen_set(0, 2, 60, text);
        strcpy(text,"Waiting for");
        screen_set(0, 1, 90, text);
        strcpy(text,"song...");
        screen_set(0, 1, 100, text);
      }
      if(millis() - get_timer > 3000) { // every 3 seconds, try to get a new song
        get_timer = millis();
        sprintf(request, GET_URL);
        strcat(request, "Host: 608dev-2.net\r\n"); //add more to the end
        strcat(request, "\r\n"); //add blank line!
        do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        if(old_response != response) {
          char* first_ind = strchr(response, '{');
          DynamicJsonDocument doc(8000);
          DeserializationError error = deserializeJson(doc, first_ind);
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            break;
          }
          strcpy(song_to_play.title,doc["title"]);
          strcpy(song_to_play.artist,doc["artist"]);
          song_to_play.length = doc["note_count"];
          for(int i=0; i< song_to_play.length; i++) {
            song_to_play.notes[i] = doc["frq"][i];
          }
          song_to_play.note_period = doc["note_duration"];
          strcpy(old_response, response);
          state = SELECT;
        }
        /*
        {"frq": [261.6, 261.6, 293.6, 293.6, 0, 0, 440.0, 440.0, 0, 0, 349.2, 349.2, 392.0, 392.0, 0, 0, 523.2, 523.2, 0, 0, 523.2, 523.2, 0, 0, 493.92, 493.92, 0, 0, 523.2, 523.2, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 261.6, 261.6, 293.6, 293.6, 0, 0, 349.2, 349.2, 0, 0, 329.6, 329.6, 349.2, 
        349.2, 0, 0, 440.0, 440.0, 0, 0, 440.0, 440.0, 0, 0, 392.0, 392.0, 0, 0, 392.0, 392.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 261.6, 261.6, 329.6, 329.6, 0, 0, 392.0, 392.0, 0, 0, 329.6, 329.6, 392.0, 392.0, 0, 0, 659.2, 659.2, 0, 0, 659.2, 659.2, 0, 0, 587.2, 587.2, 0, 0, 587.2, 587.2, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 261.6, 261.6, 329.6, 329.6, 0, 0, 392.0, 392.0, 0, 0, 329.6, 329.6, 392.0, 
        392.0, 0, 0, 493.92, 493.92, 0, 0, 493.92, 493.92, 0, 0, 493.92, 493.92, 0, 0, 523.2, 523.2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0], "note_count": 256, "factor": 4, "note_duration": 120.96774193548387, "title": "test_score_m2", "artist": "N/A"}
        */
      }
      break;
    case SELECT:
      if(state != old_state) { // if "song" button just pressed, reprint
        strcpy(text, song_to_play.title);
        screen_set(1, 1, 40, text);
        strcpy(text, song_to_play.artist);
        screen_set(0, 1, 50, text);
        strcpy(text, "Press X to start...");
        screen_set(0, 1, 70, text);
      }
      if(play == 1) {
        state = PLAY;
        countdown = millis()+4000; // start countdown at 3 seconds, millis will "countdown"
        song_timer = countdown;
      }
      break;
    case PLAY:
      intro();
      if(state != old_state) {
        play_song();
      }
      state = TO_SCORE;
      break;
    case TO_SCORE: // show score on leaderboard
            xTaskCreatePinnedToCore(
          loop2, /* Function to implement the task */
          "Analyzing", /* Name of the task */
          10000,  /* Stack size in words */
          NULL,  /* Task input parameter */
          0,  /* Priority of the task */
          &FFTTask,  /* Task handle. */
          0); /* Core where the task should run */
      delay(600000);
      state = SCORE;
      break;
    case SCORE:
      if(state != old_state) {
        score = calculate_score();
        strcpy(text,"Score:");
        screen_set(1, 1, midy-20,text);
        screen_set(0, 2, midy,"");
        tft.setCursor(45,midy);
        tft.print(score);
        tft.print("%");
        strcpy(text,"Press X to start");
        screen_set(0, 1, midy+30, text);
        strcpy(text,"a new song");
        screen_set(0, 1, midy+40, text);
      }
      if(play == 1) {
        state = IDLE;
      }
  }
  return state;
}

void screen_set(bool background, int text_size, int cursorY, char* text) {
  int half_pixels = 2.3*text_size*strlen(text);
  if(background) {
    tft.fillScreen(BACK);
  }
  tft.setTextSize(text_size);
  tft.setCursor(midx-half_pixels,cursorY);
  tft.print(text);
}

void intro() {
  int change = 0;
  while(countdown > millis()) { // countdown timer from 3 to START
    if(countdown - millis() > 3000 && change == 0) {
      strcpy(text, "3");
      screen_set(1,3,midy,text);
      change = 1;
    } else if(countdown - millis() > 2000 && countdown - millis() < 3000 && change == 1) {
      strcpy(text, "2");
      screen_set(1,3,midy,text);
      change = 0;
    } else if(countdown - millis() > 1000 && countdown - millis() < 2000 && change == 0) {
      strcpy(text, "1");
      screen_set(1,3,midy,text);
      change = 1;
    } else if(countdown > millis() && countdown - millis() < 1000 && change == 1) {
      strcpy(text, "START!");
      screen_set(1,3,midy,text);
      change = 0;
    }
  }
}

void play_song() {
  int cols = 6; // 6 notes per column
  int rows = 6; // 6 rows of notes
  int top = 2;
  int left = 2;
  int down = 125;
  int right = 150;
  int i = 0;
  int j = 0; // index for all notes per screen
  int k = 0; // index for note being played one at a time
  int counter = 0; // counts what 'screen' one is on
      xTaskCreatePinnedToCore(
          record_loop, /* Function to implement the task */
          "Recording", /* Name of the task */
          10000,  /* Stack size in words */
          NULL,  /* Task input parameter */
          0,  /* Priority of the task */
          &RecordTask,  /* Task handle. */
          0); /* Core where the task should run */
  tft.setTextSize(1);
  unsigned long starting = millis();
  while(i<song_to_play.length) { 
    j = i%(cols*rows); 
    new_note = song_to_play.notes[i];
    note_name(new_note,note); // new note is freq, note is pointer
    tft.setCursor(left+right/cols*(j%cols),top+down/rows*(j/cols));
    tft.print(note);
    i++;
    if(j == 0) starting = millis();
    if(j == cols*rows-1 || (j == song_to_play.length%(cols*rows)-1 && counter == song_to_play.length/(cols*rows))) { // on the last note of the screen, or on , play song
      k = 0;
      while(k < cols*rows) {
        if(millis() - starting > song_to_play.note_period) {
          starting = millis();
          new_note = song_to_play.notes[k+counter*cols*rows];
          ledcWriteTone(AUDIO_PWM, new_note);
          if(k!=0){ // turn old note back white
            starting = millis();
            old_note = song_to_play.notes[k-1+counter*cols*rows];
            note_name(old_note, note);
            tft.setTextColor(TEXT,BACK);
            tft.setCursor(left+right/cols*((k-1)%cols),top+down/rows*((k-1)/cols));
            tft.print(note);
          }
          note_name(new_note,note); // turn new note green while displaying
          tft.setCursor(left+right/cols*(k%cols),top+down/rows*(k/cols));
          tft.setTextColor(NOTE,BACK);
          tft.print(note);
          k++;
        }
        if(k == song_to_play.length%(cols*rows) && counter == song_to_play.length/(cols*rows)) break;
      }
      tft.setTextColor(TEXT,BACK);
      counter++;
    }
    if(j == 0) tft.fillScreen(BACK);
  }
  strcpy(text,"Nice job! Calculating");
  screen_set(1,1,midy,text);
  delay(1000);
  strcpy(text,"Nice job! Calculating.");
  screen_set(1,1,midy,text);
  delay(1000);
  strcpy(text,"Nice job! Calculating..");
  screen_set(1,1,midy,text);
  delay(1000);
}

float calculate_score() { // function will be much longer once the server side can input two long Song structs
  float error_sum = 0.0;
  float denom_sum = 0.0;
  float playing = 0.0;
  float comparing = 0.0;
  for(int i = 0; i < song_to_compare.length; i++) { // adding the log of the error to account for exponential increase of notes over octaves
    if(song_to_play.notes[i] > CO0/2) playing = log10(song_to_play.notes[i]); // if note is near 0, don't take log, keep as 0.0
    if(song_to_compare.notes[i] > CO0/2) comparing = log10(song_to_compare.notes[i]);
    error_sum += abs(playing-comparing);
    denom_sum += playing;
  }
  strcpy(text,"Calculating score...");
  screen_set(1,1,midy,text);
  return 100*(1-error_sum/denom_sum);
}

void decorate_home() {
  
}

void note_name(float freq, char* note) {
  memset(note, 0, sizeof(note));
  char letter[5];
  float base = CO0;
  double curr = 0.0;
  int ind = -1;
  if(freq < E) strcpy(note, "---");
  else {
    for(int i=0; i < 107; i++) {
      curr = base*pow(2,(double)i/12.0);
      if((1-E)*freq < curr && (1+E)*freq > curr) {
        ind = i;
        break;
      } if(curr > (1+E)*freq) {
        break; 
      }
    }
    if(ind == -1) { // if note not found
      strcpy(note, "NAN");
    } else {
      if(ind%12 == 0) strcpy(letter,"C");
      else if(ind%12 == 1) strcpy(letter,"Db");
      else if(ind%12 == 2) strcpy(letter,"D");
      else if(ind%12 == 3) strcpy(letter,"Eb");
      else if(ind%12 == 4) strcpy(letter,"E");
      else if(ind%12 == 5) strcpy(letter,"F");
      else if(ind%12 == 6) strcpy(letter,"F#");
      else if(ind%12 == 7) strcpy(letter,"G");
      else if(ind%12 == 8) strcpy(letter,"G#");
      else if(ind%12 == 9) strcpy(letter,"A");
      else if(ind%12 == 10) strcpy(letter,"Bb");
      else if(ind%12 == 11) strcpy(letter,"B");
      char octave = ind/12 + '0'; // integer division
      strcpy(note, letter);
      strncat(note, &octave,1);
    }
  }
}

void loop2(void * pvParameters){
  int num_record = (int)(song_to_play.length*song_to_play.note_period);
  while(true){
  for(int i = 0; i < (int)(num_record/SAMPLE_FREQ); i++){
    if(flags[i]==1){
      fft(samples + SAMPLE_FREQ*i,frequencies, SAMPLE_FREQ);
      flags[i] = 0;
      int max_index = 0;
      for(int i = 0; i < SAMPLE_FREQ/2; i++){
//        Serial.print("Frequency:");
//        Serial.print(i);
//        Serial.print(",");
////        Serial.print("Magnitude:");
//        Serial.println(std::abs(frequencies[i]));
        if(std::abs(frequencies[i]) > std::abs(frequencies[max_index])){
          max_index = i;
        }
      }
//   Serial.print("Frequency(Hz):");
//   Serial.println(max_index);
//   Serial.print("Note:");
//   Serial.println(find_note(max_index/2));
    song_to_compare.notes[i] = max_index;
    Serial.println(max_index);
    note_name(max_index, note);
    Serial.println(note);
    }
    }
      vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}


void readMic(){ //Read value from microphone
  if(micros()-record_timer < (int)(1000000/SAMPLE_FREQ)){
    Serial.println("TOO SLOW!");
  }
  while(micros()-record_timer < (int)(1000000/SAMPLE_FREQ)){};
  record_timer = micros(); 
  int raw_reading = analogRead(A0);
  int clean_reading = raw_reading-1362;
  samples[sample_pointer] = clean_reading;
  sample_pointer++;
//  sample_pointer=sample_pointer%NUM_SAMPLES;
  if(sample_pointer %SAMPLE_FREQ == 0){
    flags[sample_pointer/SAMPLE_FREQ -1] = 1;
  }
}

//char find_note(int frequency){
//for(int i = 0; i <7;i++){
//  if(abs((int)(notes[i])-frequency) < 20){
//    return letters[i];
//  }
//  vTaskDelay(10 / portTICK_PERIOD_MS);
//}
////Serial.println(frequency);
//return ' ';
//}

void record_loop(void * pvParameters) {
  int num_record = (int)(song_to_play.length*song_to_play.note_period);
while(true){
    if(sample_pointer<num_record){
    readMic();
    }else{
      flags[SAMPLE_DURATION-1] = 1;
      Serial.print("Time Elapsed(ms):");
      Serial.println(millis()-speed_timer);
//      Serial.print("Melody:");
//      for(int i = 0; i < SAMPLE_DURATION; i++){
//        Serial.print(melody[i]);
//        Serial.print("(");
//        Serial.print(pitches[i]);
//        Serial.println(")");
//        Serial.print(" ");
//      }
      sample_pointer = 1;
      break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
