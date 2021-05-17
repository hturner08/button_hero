#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <ArduinoJson.h>  
#define ARDUINOJSON_USE_DOUBLE 0
#include <soc/sens_reg.h>
#include <soc/sens_struct.h>
#include <driver/adc.h>

portMUX_TYPE DRAM_ATTR timerMux = portMUX_INITIALIZER_UNLOCKED; 
hw_timer_t * adcTimer = NULL; // our timer

TFT_eSPI tft = TFT_eSPI();
#define BACK TFT_BLACK
#define TEXT TFT_WHITE
#define NOTE TFT_GREEN

const char USER[] = "umagana"; //CHANGE YOUR USER VARIABLE!!!
const char GET_URL[] = "GET http://608dev-2.net/sandbox/sc/team27/button_hero_server/data_to_esp32.py HTTP/1.1\r\n";
const char POST_URL[] = "POST http://608dev-2.net/sandbox/sc/team27/button_hero_server/game_status.py HTTP/1.1\r\n";
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
char request_score[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
char response_score[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
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
const int SAMPLE_FREQ = (int)(1000000/40);//25KHz
const int LOOPS = 10;
const int SAMPLES = 1024;
int * input;
int fft_pointer = 0;
int sample_pointer = 0;
int song_pointer = 0;
bool fft_condition = false;

TaskHandle_t RecordTask;
TaskHandle_t FFT_Task;
double notes[7] = {440.00,493.88, 523.25,587.33,659.25,698.46,783.99};
char letters[7] = {'A','B','C','D','E','F','G'};

int record_timer;
int speed_timer;
//int state = 0;

float CO0 = 16.35;

// used for game when displaying next notes to play
class Song {
  public:
    char title[50];
    char artist[50];
    float notes[500]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
    int length; //number of notes (essentially length of array).
    float note_period; // once we get bpm and fraction from actual song (quarter notes, eight notes, etc.), change them using conversion: note_period = 60.0*1000/bpm/FACTOR
    char user[50];
  // FACTOR is (# notes needed per measure/ fraction of notes in beat). For example, # notes needed per measure is often 16 or 32, and typically bpm counts the song's quarter notes, so FACTOR = 16/4 or 32/4
};

//Representing the actual song
//Song lights = {"All of the Lights", "Kanye West", {CO4, 0.0, CO4, CO4, CO4, CO4, CO4, 0.0, CO4, 0.0, EO4, EO4, EO4, EO4, AO4, 0.0, GO4, 0.0, GO4, GO4, GO4, GO4, 369.99, 0.0, EO4, 0.0, EO4, EO4, EO4, EO4, EO4, 0.0}, 32, 208.3, "Uriel"};
//Song danza = {"Danza Kuduro", "Don Omar", {CO4, EO4, 0.0, AO4, 0.0, FO4, GO4, 0.0, CO5, 0.0, CO5, 0.0, BO4, 0.0, CO5, 0.0, CO4, DO4, 0.0, FO4, 0.0, EO4, FO4, 0.0, AO4, 0.0, AO4, 0.0, GO4, 0.0, GO4, 0.0, CO4, EO4, 0.0, GO4, 0.0, EO4, GO4, 0.0, EO5, 0.0, EO5, 0.0, DO5, 0.0, DO5, 0.0, CO4, EO4, 0.0, GO4, 0.0, EO4, GO4, 0.0, BO4, 0.0, BO4, 0.0, BO4, 0.0, CO5, 0.0}, 64, 115.38};
Song song_to_play;  //initialize song for GET request

//Representing user's instrument with some mistakes while playing
//This will be built from the microphone 
//Song song_to_compare = {"user_song", "user", { CO3, EO3, 0.0, AO3, 0.0, FO3, GO3, 0.0, CO4, 0.0, CO4, 0.0, BO4, 0.0, BO4, 0.0, CO3, DO3, 0.0, FO3, 0.0, EO3, FO3, 0.0, AO3, 0.0, AO3, 0.0, GO3, 0.0, GO3, 0.0, CO3, EO3, 0.0, GO3, 0.0, EO3, GO3, 0.0, EO4, 0.0, EO4, 0.0, DO4, 0.0, DO4, 0.0, CO3, EO3, 0.0, GO3, 0.0, EO3, GO3, 0.0, BO3, 0.0, BO3, 0.0, BO3, 0.0, CO4, 0.0}, 64, 115.38, "Uriel"};
Song song_to_compare = {"user_song", "user", {0.0}, 32, 208.3, "Uriel"};

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


int IRAM_ATTR local_adc1_read(int channel) {
    uint16_t adc_value;
    //set controller
    SENS.sar_meas_start1.meas1_start_force = true;  //RTC controller controls the ADC,not ulp coprocessor
    SENS.sar_meas_start1.sar1_en_pad = (1 << channel); // only one channel is selected
    while (SENS.sar_slave_addr1.meas_status != 0);
    SENS.sar_meas_start1.meas1_start_sar = 0;
    SENS.sar_meas_start1.meas1_start_sar = 1;
    while(SENS.sar_meas_start1.meas1_done_sar == 0);
    adc_value = SENS.sar_meas_start1.meas1_data_sar;
    return adc_value;
}

void IRAM_ATTR onTimer() {
    input[sample_pointer++] = local_adc1_read(ADC1_CHANNEL_0)-1362;
  
  if (sample_pointer%SAMPLES == 0) { 
    // Notify adcTask that the buffer is full.
    portENTER_CRITICAL_ISR(&timerMux);
    if(fft_condition){
      Serial.println("OVERLAPPED!");
    }
    fft_condition = true;
    portEXIT_CRITICAL_ISR(&timerMux);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(FFT_Task, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }  
  sample_pointer = sample_pointer%(SAMPLES*LOOPS);
}

void fft_loop(void * pvParameters){
  float peak = 0;
  float old_peak = peak;
  char note[3];
  char old_note[3];
  char song_note[3];
  while(true){
  uint32_t tcount = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(1000));  
  portENTER_CRITICAL_ISR(&timerMux);
  if(fft_condition){
    fft_condition = false;
     portEXIT_CRITICAL_ISR(&timerMux);
      speed_timer = micros();
      peak = Approx_FFT(input + fft_pointer*SAMPLES,SAMPLES,SAMPLE_FREQ);
      note_name(peak,note);
//      int fft_time = micros()-fft_timer;
//      Serial.print("Time(Microseconds):");
//      Serial.println(fft_time);
//      Serial.print("Peak(1024 Samples):");
//      Serial.println(peak);
      if(song_to_compare.notes[song_pointer] == 0.0 & strcmp(note,old_note) == 0){
          song_to_compare.notes[song_pointer] = peak;
          strncpy(song_note,note,3);
      }else if(strcmp(note,old_note) == 0 & strcmp(song_note,old_note) != 0){
        song_pointer++;
      }
   fft_pointer = ++fft_pointer%LOOPS;
   old_peak = peak;
   note_name(old_peak,old_note);
   Serial.println(micros()-speed_timer);
  }else{
   portEXIT_CRITICAL_ISR(&timerMux);
  }
  }
}

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
  decorate_home();
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  state = 0;
  old_state = 0;
  input = (int*)ps_malloc(sizeof(int)*SAMPLES*LOOPS);
  xTaskCreatePinnedToCore(fft_loop, "Handler Task", 20000, NULL, 0, &FFT_Task,0);
//  adcTimer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 MHz hardware clock for easy figuring
//  timerAttachInterrupt(adcTimer, &onTimer, true); // Attaches the handler function to the timer 
//  timerAlarmWrite(adcTimer, 40, true); // Interrupts when counter == 45, i.e. 22.222 times a second
//  timerAlarmEnable(adcTimer);
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
        decorate_home();
        strcpy(text,"HERO");
        screen_set(0, 2, 60, text);
        strcpy(text,"Waiting for");
        screen_set(0, 1, 80, text);
        strcpy(text,"song...");
        screen_set(0, 1, 90, text);
      }
      if(millis() - get_timer > 500) { // every 3 seconds, try to get a new song
        get_timer = millis();
        Serial.print(get_timer);
        sprintf(request, GET_URL);
        strcat(request, "Host: 608dev-2.net\r\n"); //add more to the end
        strcat(request, "\r\n"); //add blank line!
        do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.println(response);
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
          strcpy(song_to_play.user,doc["user"]);
          song_to_play.length = doc["note_count"];
          for(int i=0; i< song_to_play.length; i++) {
            song_to_play.notes[i] = doc["frq"][i];
          }
          song_to_play.note_period = doc["note_duration"];
          strcpy(old_response, response);
          if(song_to_play.length != 0){
            delay(3500);
            state = PLAY;
          }
        }
      }
      break;
    case SELECT:
      if(state != old_state) { // if "song" button just pressed, reprint
        strcpy(text, song_to_play.title);
        screen_set(1, 2, 40, text);
        strcpy(text, song_to_play.artist);
        screen_set(0, 1, 60, text);
        strcpy(text, "Press button to start...");
        screen_set(0, 1, 75, text);
        decorate_home();
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
         
//      delay(60000);
      state = SCORE;
      break;
    case SCORE:
      if(state != old_state) {
        score = calculate_score();
        Serial.print("Score: ");
        Serial.println(score);
        // POST score to python
        char body[100]; //for body
        sprintf(body,"user=%s&result=%f",song_to_play.user,score);
        int body_len = strlen(body);  //calculate body length (for header reporting)
        sprintf(request_score, POST_URL);
        strcat(request_score, "Host: 608dev-2.net\r\n"); //add more to the end
        strcat(request_score,"Content-Type: application/x-www-form-urlencoded\r\n");
        sprintf(request_score+strlen(request_score),"Content-Length: %d\r\n", body_len);
        strcat(request_score, "\r\n"); 
        strcat(request_score,body); 
        strcat(request_score,"\r\n"); 
        do_http_request("608dev-2.net", request_score, response_score, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        // TFT score to screen
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
        get_timer = millis();
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
  int i = 0;
  unsigned long starting = millis();
  strcpy(text, "Look at screen!");
  screen_set(1,1,midy,text);
  adcTimer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 MHz hardware clock for easy figuring
  timerAttachInterrupt(adcTimer, &onTimer, true); // Attaches the handler function to the timer 
  timerAlarmWrite(adcTimer, 40, true); // Interrupts when counter == 45, i.e. 22.222 times a second
  timerAlarmEnable(adcTimer);
  new_note = song_to_play.notes[0];
  while(i<song_to_play.length) {
    if(millis() - starting > song_to_play.note_period) {
      starting = millis();
      i++;
      new_note = song_to_play.notes[i];
//      Serial.println(new_note);
    }
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
    Serial.println(song_to_compare.notes[i]);
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
  tft.drawLine(50,100,110,100,TFT_GREEN);
  tft.drawLine(50,100,75,108,TFT_GREEN);
  tft.drawLine(75,108,110,100,TFT_GREEN);
  tft.drawLine(50,24,110,24,TFT_GREEN);
  tft.drawLine(50,24,85,16,TFT_GREEN);
  tft.drawLine(85,16,110,24,TFT_GREEN);
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
