uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  if (client2.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client2.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client2.connected()) { //while we remain connected read out data coming back
      client2.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client2.available()) { //read out remaining text (body of response)
      char_append(response,client2.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client2.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client2.stop();
  }
}  

// ALL NOTES in Hz (names are written with the letter O because some letter-number combinations are already C preset variable names)
//float CO0 = 16.35;
//float CS0 = CO0*pow(2,1.0/12.0);
//float DO0 = CO0*pow(2,2.0/12.0);
//float DS0 = CO0*pow(2,3.0/12.0);
//float EO0 = CO0*pow(2,4.0/12.0);
//float FO0 = CO0*pow(2,5.0/12.0);
//float FS0 = CO0*pow(2,6.0/12.0);
//float GO0 = CO0*pow(2,7.0/12.0);
//float GS0 = CO0*pow(2,8.0/12.0);
//float AO0 = CO0*pow(2,9.0/12.0);
//float AS0 = CO0*pow(2,10.0/12.0);
//float BO0 = CO0*pow(2,11.0/12.0);
//float CO1 = CO0*pow(2,12.0/12.0);
//float CS1 = CO0*pow(2,13.0/12.0);
//float DO1 = CO0*pow(2,14.0/12.0);
//float DS1 = CO0*pow(2,15.0/12.0);
//float EO1 = CO0*pow(2,16.0/12.0);
//float FO1 = CO0*pow(2,17.0/12.0);
//float FS1 = CO0*pow(2,18.0/12.0);
//float GO1 = CO0*pow(2,19.0/12.0);
//float GS1 = CO0*pow(2,20.0/12.0);
//float AO1 = CO0*pow(2,21.0/12.0);
//float AS1 = CO0*pow(2,22.0/12.0);
//float BO1 = CO0*pow(2,23.0/12.0);
//float CO2 = CO0*pow(2,24.0/12.0);
//float CS2 = CO0*pow(2,25.0/12.0);
//float DO2 = CO0*pow(2,26.0/12.0);
//float DS2 = CO0*pow(2,27.0/12.0);
//float EO2 = CO0*pow(2,28.0/12.0);
//float FO2 = CO0*pow(2,29.0/12.0);
//float FS2 = CO0*pow(2,30.0/12.0);
//float GO2 = CO0*pow(2,31.0/12.0);
//float GS2 = CO0*pow(2,32.0/12.0);
//float AO2 = CO0*pow(2,33.0/12.0);
//float AS2 = CO0*pow(2,34.0/12.0);
//float BO2 = CO0*pow(2,35.0/12.0);
//float CO3 = CO0*pow(2,36.0/12.0);
//float CS3 = CO0*pow(2,37.0/12.0);
//float DO3 = CO0*pow(2,38.0/12.0);
//float DS3 = CO0*pow(2,39.0/12.0);
//float EO3 = CO0*pow(2,40.0/12.0);
//float FO3 = CO0*pow(2,41.0/12.0);
//float FS3 = CO0*pow(2,42.0/12.0);
//float GO3 = CO0*pow(2,43.0/12.0);
//float GS3 = CO0*pow(2,44.0/12.0);
//float AO3 = CO0*pow(2,45.0/12.0);
//float AS3 = CO0*pow(2,46.0/12.0);
//float BO3 = CO0*pow(2,47.0/12.0);
//float CO4 = CO0*pow(2,48.0/12.0);
//float CS4 = CO0*pow(2,49.0/12.0);
//float DO4 = CO0*pow(2,50.0/12.0);
//float DS4 = CO0*pow(2,51.0/12.0);
//float EO4 = CO0*pow(2,52.0/12.0);
//float FO4 = CO0*pow(2,53.0/12.0);
//float FS4 = CO0*pow(2,54.0/12.0);
//float GO4 = CO0*pow(2,55.0/12.0);
//float GS4 = CO0*pow(2,56.0/12.0);
//float AO4 = CO0*pow(2,57.0/12.0);
//float AS4 = CO0*pow(2,58.0/12.0);
//float BO4 = CO0*pow(2,59.0/12.0);
//float CO5 = CO0*pow(2,60.0/12.0);
//float CS5 = CO0*pow(2,61.0/12.0);
//float DO5 = CO0*pow(2,62.0/12.0);
//float DS5 = CO0*pow(2,63.0/12.0);
//float EO5 = CO0*pow(2,64.0/12.0);
//float FO5 = CO0*pow(2,65.0/12.0);
//float FS5 = CO0*pow(2,66.0/12.0);
//float GO5 = CO0*pow(2,67.0/12.0);
//float GS5 = CO0*pow(2,68.0/12.0);
//float AO5 = CO0*pow(2,69.0/12.0);
//float AS5 = CO0*pow(2,70.0/12.0);
//float BO5 = CO0*pow(2,71.0/12.0);
//float CO6 = CO0*pow(2,72.0/12.0);
//float CS6 = CO0*pow(2,73.0/12.0);
//float DO6 = CO0*pow(2,74.0/12.0);
//float DS6 = CO0*pow(2,75.0/12.0);
//float EO6 = CO0*pow(2,76.0/12.0);
//float FO6 = CO0*pow(2,77.0/12.0);
//float FS6 = CO0*pow(2,78.0/12.0);
//float GO6 = CO0*pow(2,79.0/12.0);
//float GS6 = CO0*pow(2,80.0/12.0);
//float AO6 = CO0*pow(2,81.0/12.0);
//float AS6 = CO0*pow(2,82.0/12.0);
//float BO6 = CO0*pow(2,83.0/12.0);
//float CO7 = CO0*pow(2,84.0/12.0);
//float CS7 = CO0*pow(2,85.0/12.0);
//float DO7 = CO0*pow(2,86.0/12.0);
//float DS7 = CO0*pow(2,87.0/12.0);
//float EO7 = CO0*pow(2,88.0/12.0);
//float FO7 = CO0*pow(2,89.0/12.0);
//float FS7 = CO0*pow(2,90.0/12.0);
//float GO7 = CO0*pow(2,91.0/12.0);
//float GS7 = CO0*pow(2,92.0/12.0);
//float AO7 = CO0*pow(2,93.0/12.0);
//float AS7 = CO0*pow(2,94.0/12.0);
//float BO7 = CO0*pow(2,95.0/12.0);
//float CO8 = CO0*pow(2,96.0/12.0);
//float CS8 = CO0*pow(2,97.0/12.0);
//float DO8 = CO0*pow(2,98.0/12.0);
//float DS8 = CO0*pow(2,99.0/12.0);
//float EO8 = CO0*pow(2,100.0/12.0);
//float FO8 = CO0*pow(2,101.0/12.0);
//float FS8 = CO0*pow(2,102.0/12.0);
//float GO8 = CO0*pow(2,103.0/12.0);
//float GS8 = CO0*pow(2,104.0/12.0);
//float AO8 = CO0*pow(2,105.0/12.0);
//float AS8 = CO0*pow(2,106.0/12.0);
//float BO8 = CO0*pow(2,107.0/12.0);
