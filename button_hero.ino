#include <soc/sens_reg.h>
#include <soc/sens_struct.h>
#include "arduinoFFT.h"
#include <driver/adc.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <ArduinoJson.h>  
#define ARDUINOJSON_USE_DOUBLE 0
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

///////////////////////////////////////////////////
//////////////WIFI ANAD ESPI GLOBAL VARIABLES//////
/////////////////////////////////////////////////
TFT_eSPI tft = TFT_eSPI();
const char USER[] = "umagana"; //CHANGE YOUR USER VARIABLE!!!
const char GET_URL[] = "GET http://608dev-2.net/sandbox/sc/team27/button_hero_server/data_to_esp32.py HTTP/1.1\r\n";
const char POST_URL[] = "POST http://608dev-2.net/sandbox/sc/team27/button_hero_server/game_status.py HTTP/1.1\r\n";
const char network[] = "MIT";
const char password[] = "";
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
WiFiClient client2; //global WiFiClient Secure object
const int RESPONSE_TIMEOUT = 10000; //ms to wait for response from host
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

char text[50];
char note[10];

#define BACK TFT_BLACK
#define TEXT TFT_WHITE
#define NOTE TFT_GREEN

const int BUTTON1 = 5;
const int BUTTON2 = 19; // RX

const int IDLE = 0;
const int SELECT = 1;
const int PLAY = 2;
const int TO_SCORE = 3;
const int SCORE = 4;

int state;
int old_state;
////////////////////////////////////////////////////////////
/////////////////////////FFT Global Variables//////////////
///////////////////////////////////////////////////////////
portMUX_TYPE DRAM_ATTR timerMux = portMUX_INITIALIZER_UNLOCKED; 
hw_timer_t * adcTimer = NULL; // our timer
TaskHandle_t FFT_Task;
const int SAMPLE_FREQ = (int)(1000000/40);//20KHz
const int LOOPS = 10;
const int SAMPLES = 1024;
int sample_pointer = 0;
int sample_count = 0;
int fft_pointer = 0;
bool fft_condition = false;
int flags[LOOPS] = {0};
int fft_timer = 0;
uint8_t AUDIO_TRANSDUCER = 26;
uint8_t AUDIO_PWM = 1;
volatile int interrupt_counter;
volatile int interrupt_timer = 0;

char note1[4];
char old_note1[4];
char song_note[4];
int *input;

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03
#define ADC1_GPIO36_CHANNEL  ADC1_CHANNEL_0

/////////////////////////////////////////////////////
////////////////MUSIC FORMAT GLOBAL VARIABLES///////
///////////////////////////////////////////////////
unsigned long starting;
int record_timer;
int speed_timer;
int get_timer;
volatile int song_pointer = 0;
float old_note = 0;
float new_note = 0;
int i;
//int state = 0;
float E = 0.02; // buffer to allow in notes (1-E to 1+E)
float CO0 = 16.35;
float score;
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
//346.90,346.90,346.90,438.72,438.72,



/////////////////////////////////////////
///////Interrupt Timers and Handlers////
////////////////////////////////////////
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
    input[sample_pointer++] = local_adc1_read(ADC1_CHANNEL_3)-1362;
    
  if(interrupt_counter %500000 == 0){
    Serial.println((micros()-interrupt_timer)/1000000);
    interrupt_timer = micros();
    interrupt_counter = 0;
  }
  interrupt_counter++;
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
  while(true){
  uint32_t tcount = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(1000));  
  portENTER_CRITICAL_ISR(&timerMux);
  if(fft_condition){
    fft_condition = false;
     portEXIT_CRITICAL_ISR(&timerMux);
    fft_timer = micros();
    float peak = Approx_FFT(input + fft_pointer*SAMPLES,SAMPLES,SAMPLE_FREQ);
    note_name(peak,note1);
    if(song_pointer == 0){
      if(strcmp(note1,song_note)==0){
        song_to_compare.notes[0] = peak;
        song_pointer++;
      }
    }
    else if(strcmp(note1,old_note1) == 0 & (song_to_compare.notes[song_pointer] == 0.0 || strcmp(note1,song_note)==0)){
        song_to_compare.notes[song_pointer] = peak;
        strncpy(song_note,note1,3);
//        Serial.println(peak);
    }
   strncpy(old_note1,note1,3);
   fft_pointer = ++fft_pointer%LOOPS;  
  }else{
   portEXIT_CRITICAL_ISR(&timerMux);
  }
  }
}

/////////////////////////////////////////////
////////////////SETUP AND MAIN LOOP/////////
////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  connectWiFi();
//  WiFi.disconnect();
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
  input = (int*)ps_malloc(sizeof(int)*SAMPLES*LOOPS);
  xTaskCreatePinnedToCore(fft_loop, "Handler Task", 20000, NULL, 0, &FFT_Task,0);
  adcTimer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 MHz hardware clock for easy figuring
  timerAttachInterrupt(adcTimer, &onTimer, true); // Attaches the handler function to the timer 
  timerAlarmWrite(adcTimer, 40, true); // Interrupts when counter == 45, i.e. 22.222 times a second
//  timerAlarmEnable(adcTimer);
  state = IDLE;
  old_state = IDLE;
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
}

void loop() {
switch(state){
  case IDLE:
        if(state != old_state){
           tft.setTextColor(TEXT, BACK); //set color for font
          strcpy(text,"BUTTON");
          screen_set(1, 2, 40, text);
          strcpy(text,"HERO");
          screen_set(0, 2, 60, text);
          strcpy(text,"Waiting for");
          screen_set(0, 1, 80, text);
          strcpy(text,"song...");
          screen_set(0, 1, 90, text);
        }
        else if(millis() - get_timer > 1000) { // every .1 seconds, try to get a new song
        get_timer = millis();
        do_song_request();
      }
      break;
   case PLAY:
      i = 0;
      starting = millis();
      delay(5000);
      strcpy(text, "Playing");
      screen_set(1,1,midy-15,text);
      strcpy(text, song_to_play.title);
      screen_set(0,1,midy,text);
      strcpy(text, "Look at screen!");
      screen_set(0,1,midy+15,text);
      new_note = song_to_play.notes[0];
      note_name(new_note,song_note);
      WiFi.disconnect();
      timerAlarmEnable(adcTimer);
      while(song_pointer==0);
      Serial.println("Gate 3");
      while(song_pointer<song_to_play.length) {
        if(millis() - starting > song_to_play.note_period) {
          starting = millis();
          new_note = song_to_play.notes[song_pointer++];
          note_name(new_note,song_note);
          Serial.println(new_note);
        }
      }
      cli();
      timerAlarmDisable(adcTimer);
      connectWiFi();
//      Serial.print("Interrupt Count");
//      Serial.println(sample_count);
      delay(50);
      strcpy(text,"Nice job! Calculating");
      screen_set(1,1,midy,text);
      delay(1000);
      strcpy(text,"Nice job! Calculating.");
      screen_set(1,1,midy,text);
      delay(1000);
      strcpy(text,"Nice job! Calculating..");
      screen_set(1,1,midy,text);
      delay(1000);
      state=SCORE;
      break;
   case SCORE:
      score = calculate_score();
      send_score();
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
      delay(5000);
      state = IDLE;
      break;
}
delay(100);
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

float calculate_score() { // function will be much longer once the server side can input two long Song structs
  float error_sum = 0.0;
  float denom_sum = 0.0;
  float playing = 0.0;
  float comparing = 0.0;
  Serial.print("Length play: ");
  Serial.print(song_to_play.length);
  Serial.print("Length compare: ");
  Serial.println(song_to_compare.length);
  for(int i = 0; i < song_to_play.length; i++) { // adding the log of the error to account for exponential increase of notes over octaves
    if(song_to_play.notes[i] > CO0/2) playing = log10(song_to_play.notes[i]); // if note is near 0, don't take log, keep as 0.0
    else playing = song_to_play.notes[i];
    if(song_to_compare.notes[i] > CO0/2) comparing = log10(song_to_compare.notes[i]);
    else comparing = song_to_compare.notes[i];
//    if(abs(playing-comparing) > difficulty) error_sum += playing;
//    else error_sum += abs(playing-comparing);
    Serial.print("PLAY: log(");
    Serial.print(song_to_play.notes[i]);
    Serial.print(") = ");
    Serial.print(playing);
    Serial.print("  COMPARE: log(");
    Serial.print(song_to_compare.notes[i]);
    Serial.print(") = ");
    Serial.println(comparing);
    error_sum += abs(playing-comparing);
    denom_sum += playing;
  }
  strcpy(text,"Calculating score...");
  screen_set(1,1,midy,text);
  Serial.println(100*(1-error_sum/denom_sum));
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
