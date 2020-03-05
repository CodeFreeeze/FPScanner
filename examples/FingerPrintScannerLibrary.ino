#include "ABUNA_FPS.h"

ABUNA_FPS fps = ABUNA_FPS(&Serial3);

#define PRINT_RESPONSE
void setup(){
  Serial.begin(9600);
  fps.begin(115200);  
  fps.set0nScannerResponseDetailedListener(fingerprintResponsed);
}
void loop(){
  if(Serial.available()> 0){
    char rx = Serial.read();

    if(rx ==  'i'){
      Serial.println("Please place finger to identify");
      fps.identify();
    }
    if(rx ==  'e'){
      Serial.println("Enroll");
      fps.enroll(1);
    }
    if(rx ==  'f'){
      Serial.println("Enroll");
      fps.enroll(2);
    }
    if(rx ==  'a'){
      Serial.println("All fp deleted");
      fps.deleteAll();
    }
    if(rx ==  'b'){
      Serial.println("All fp deleted");
      fps.deleteById(1);
    }
    if(rx ==  'c'){
      Serial.println("All fp deleted");
      fps.deleteById(2);
    }
    
  }

  fps.monitorScanner();
}

void fingerprintResponsed(uint16_t resultCode, uint16_t responseCode, uint16_t data, String description){
  Serial.println(description);
}
