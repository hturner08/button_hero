#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();
#define BACK TFT_BLUE
#define TEXT TFT_WHITE
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

char title[20];
char artist[30];
char text[50];

uint8_t state;
uint8_t old_state;
float thi_note = 0;
float sec_note = 0;
float new_note = 0;
float old_note = 0;

uint8_t i = 0;
uint32_t countdown;
uint32_t primary_timer;
uint32_t song_timer;

float score;

// ALL NOTES in Hz (names are written with the letter O because some letter-number combinations are already C preset variable names)
double CO0 = 16.35;
double DO0 = 18.35;
double EO0 = 20.60;
double FO0 = 21.83;
double GO0 = 24.50;
double AO0 = 27.50;
double BO0 = 30.87;
double CO1 = 32.70;
double DO1 = 36.71;
double EO1 = 41.20;
double FO1 = 43.65;
double GO1 = 49.00;
double AO1 = 55.00;
double BO1 = 61.74;
double CO2 = 65.41;
double DO2 = 73.42;
double EO2 = 82.41;
double FO2 = 87.31;
double GO2 = 98.00;
double AO2 = 110.00;
double BO2 = 123.47;
double CO3 = 130.81;
double DO3 = 138.59;
double EO3 = 146.83;
double FO3 = 174.61;
double GO3 = 196.00;
double AO3 = 220.00;
double BO3 = 246.94;
double CO4 = 261.63;
double DO4 = 293.66;
double EO4 = 329.63;
double FO4 = 349.23;
double GO4 = 392.00;
double AO4 = 440.00;
double BO4 = 493.88;
double CO5 = 523.25;
double DO5 = 587.33;
double EO5 = 659.25;
double FO5 = 698.46;
double GO5 = 783.99;
double AO5 = 880.00;
double BO5 = 987.77;
double CO6 = 1046.50;
double DO6 = 1174.66;
double EO6 = 1318.51;
double FO6 = 1396.91;
double GO6 = 1567.98;
double AO6 = 1760.00;
double BO6 = 1975.53;
double CO7 = 2093.00;
double DO7 = 2349.32;
double EO7 = 2637.02;
double FO7 = 2793.83;
double GO7 = 3135.96;
double AO7 = 3520.00;
double BO7 = 3951.07;
double CO8 = 4186.01;
double DO8 = 4678.03;
double EO8 = 5274.04;
double FO8 = 5587.65;
double GO8 = 6271.93;
double AO8 = 7040.00;
double BO8 = 7902.13;


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

struct Notes {
  char n[600];
  double hz[200];
};

// used for game when displaying next notes to play
Notes M = {{"CO0 CS0 DO0 DS0 EO0 FO0 FS0 GO0 GS0 AO0 AS0 BO0 CO1 CS1 DO1 DS1 EO1 FO1 FS1 GO1 GS1 AO1 AS1 BO1 CO2 CS2 DO2 DS2 EO2 FO2 FS2 GO2 GS2 AO2 AS2 BO2 CO3 CS3 DO3 DS3 EO3 FO3 FS3 GO3 GS3 AO3 AS3 BO3 CO4 CS4 DO4 DS4 EO4 FO4 FS4 GO4 GS4 AO4 AS4 BO4 CO5 CS5 DO5 DS5 EO5 FO5 FS5 GO5 GS5 AO5 AS5 BO5 CO6 CS6 DO6 DS6 EO6 FO6 FS6 GO6 GS6 AO6 AS6 BO6 CO7 CS7 DO7 DS7 EO7 FO7 FS7 GO7 GS7 AO7 AS7 BO7 CO7 CS7 DO7 DS7 EO7 FO7 FS7 GO7 GS7 AO7 AS7 BO7 CO8 CS8 DO8 DS8 EO8 FO8 FS8 GO8 GS8 AO8 AS8 BO8 "},
           {16.35,17.32,18.35,19.45,20.60,21.83,23.12,24.50,25.96,27.50,29.14,30.87,32.70,34.65,36.71,38.89,41.20,43.65,46.25,49.00,51.91,55.00,58.27,61.74,65.41,69.30,73.42,77.78,82.41,87.31,92.50,98.00,103.83,110.00,116.54,123.47,130.81,138.59,146.83,155.56,164.811,174.61,185.00,196.00,207.65,220.00,233.08,246.94,261.63,277.18,293.66,311.131,329.63,349.23,369.99,392.00,415.30,440.00,466.16,493.88,523.25,554.37,587.33,622.25,659.25,698.46,739.99,783.99,830.61,880.00,932.33,987.77,1046.50,1108.73,1176.66,1244.51,1318.51,1396.91,1479.98,1567.98,1661.22,1760.00,1864.66,1975.53,2093.00,2217.46,2349.32,2489.02,2637.02,2793.83,2959.96,3135.96,3322.44,3520.00,3729.31,3951.07,4186.01,4434.92,4698.63,4978.03,5274.04,5587.65,5919.91,6271.93,6644.88,7040.00,7458.62,7902.13}}; // have to finish this

struct Song {
  char title[50];
  char artist[50];
  double notes[64]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array).
  float note_period; // once we get bpm and fraction from actual song (quarter notes, eight notes, etc.), change them using conversion: note_period = 60.0*1000/bpm/FACTOR
  float factor;
  // FACTOR is (# notes needed per measure/ fraction of notes in beat). For example, # notes needed per measure is often 16 or 32, and typically bpm counts the song's quarter notes, so FACTOR = 16/4 or 32/4
};

//Representing the actual song
Song lights = {"All of the Lights", "Kanye West", {CO4, 0.0, CO4, CO4, CO4, CO4, CO4, 0.0, CO4, 0.0, EO4, EO4, EO4, EO4, AO4, 0.0, GO4, 0.0, GO4, GO4, GO4, GO4, 369.99, 0.0, EO4, 0.0, EO4, EO4, EO4, EO4, EO4, 0.0}, 32, 208.3, 4};
Song song_to_play = lights;  //select one of the riff songs

//Representing user's instrument with some mistakes while playing
//Song user = {{}}; This will come from GET request
Song song_to_compare = {"USER INPUT", "USER",     {CO4, 30.0, CO4, DO4, CO4, CO4, CO4, 0.0, CO4, 0.0, FO4, EO4, EO4, EO4, AO4, 0.0, GO4, 0.0, GO4, GO4, GO4, AO4, 369.99, 0.0, EO4, 0.0, EO4, EO4, EO4, EO4, EO4, 0.0}, 32, 208.3, 4}; 

Button play(BUTTON1); //button object!
Button song(BUTTON2); //button object!

void setup() {
  Serial.begin(115200); //begin serial comms
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
  
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);
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
      if(song == 1) { // change to if(old request buffer != request from get request)
        state = SELECT;
        // title = "title from get request";
        // artist = "artist from get request";
        // bpm = "bpm from get request";
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
  thi_note = song_to_play.notes[0];
  sec_note = song_to_play.notes[0];
  new_note = song_to_play.notes[0];
  ledcWriteTone(AUDIO_PWM, song_to_play.notes[0]);
  int j = 1;
  unsigned long starting = millis();
  while(j < song_to_play.length) {
    if(millis() - starting > song_to_play.note_period) { // once a note period passes, play the next note
      if(j < song_to_play.length - 2) {
        new_note = song_to_play.notes[j];
        sec_note = song_to_play.notes[j+1];
        thi_note = song_to_play.notes[j+2];
      } else if(j == song_to_play.length - 1) {
        new_note = song_to_play.notes[j];
        sec_note = song_to_play.notes[j+1];
        thi_note = 0.0;
      } else {
          new_note = song_to_play.notes[j];
          sec_note = 0.0;
          thi_note = 0.0;
      }
      ledcWriteTone(AUDIO_PWM, new_note);
      screen_set(1, 2, midy-10, "");
      tft.setCursor(30,midy-10);
      tft.print(">");
      tft.print(new_note);
      tft.setTextSize(1);
      tft.setCursor(40, midy+20);
      tft.print(sec_note);
      tft.setCursor(40, midy+40);
      tft.print(thi_note);
      starting = millis();
      j ++;
    }
  }
  strcpy(text,"Nice job! Calculating..");
  screen_set(1,1,midy,text);
  delay(3000);
  ledcWriteTone(AUDIO_PWM, 0.0);
}

float calculate_score() { // function will be much longer once the server side can input two long Song structs
  float error_sum = 0.0;
  float denom_sum = 0.0;
  for(int i = 0; i < song_to_compare.length; i++) {
    error_sum += abs(song_to_play.notes[i] - song_to_compare.notes[i]);
    denom_sum += song_to_play.notes[i];
  }
  strcpy(text,"Calculating score...");
  screen_set(1,1,midy,text);
  return 100*(1-error_sum/denom_sum);
}
