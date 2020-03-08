#include "Arduino.h"
#include "ABUNA_FPS.h"
#ifdef __AVR__
  #include <util/delay.h>
  #include <SoftwareSerial.h>
#endif

#ifdef __AVR__
ABUNA_FPS::ABUNA_FPS(SoftwareSerial *referrenceSS){
	
  hwSerial = NULL;
  	swSerial = referrenceSS;
  	fpsSerial = swSerial;
}
#endif

ABUNA_FPS::ABUNA_FPS(HardwareSerial *referenceHS){
#ifdef __AVR__
	swSerial = NULL;
#endif
	hwSerial = referenceHS;
  	fpsSerial = hwSerial;
}

void ABUNA_FPS::begin(double baudRate){
  delay(1000);
  if (hwSerial){
    hwSerial->begin(baudRate);
  }
#ifdef __AVR__
  if (swSerial){
    swSerial->begin(baudRate);
  }
#endif
}

void ABUNA_FPS::cancel(){
  mode = CANCEL_MODE;
  memset(requestData,0,24);
  createRequestPocket(COMMAND_PACKET, CMD_CANCEL , requestData, 0);
  sendPocket(requestPocket);
}
void ABUNA_FPS::identify(){
  mode = IDENTIFY_MODE;
  memset(requestData,0,24);
  createRequestPocket(COMMAND_PACKET, CMD_IDENTIFY , requestData, 0);
  sendPocket(requestPocket);
}
void ABUNA_FPS::enroll(uint16_t templateId){
  mode = ENROLL_MODE;
  memset(requestData,0,24);
  requestData[0] = templateId;
  createRequestPocket(COMMAND_PACKET, CMD_ENROLL, requestData, 2);
  sendPocket(requestPocket);
}
void ABUNA_FPS::deleteById(uint16_t templateId){
  mode = DELETE_MODE;
  memset(requestData,0,24);
  requestData[0] = templateId;
  createRequestPocket(COMMAND_PACKET, CMD_CLEAR_ADDR, requestData, 2);
  sendPocket(requestPocket);
}
void ABUNA_FPS::deleteAll(){
  mode = DELETE_ALL_MODE;
  memset(requestData,0,24);
  createRequestPocket(COMMAND_PACKET, CMD_CLEAR_ALL, requestData, 0);
  sendPocket(requestPocket);
}

void ABUNA_FPS::monitorScanner(){
  if(fpsSerial->available()){
    while(fpsSerial->available()){
      if(fpsSerial->available() >= 24){
        fpsSerial->readBytes((char*) responsePocket, FPS_RESPONSE_SIZE);
        analyzePocket();    
#ifdef PRINT_RESPONSE   
        for(int i=0; i < FPS_RESPONSE_SIZE; i++){
          Serial.print(responsePocket[i],HEX);
          Serial.print(", ");
        }
        Serial.println("]");
#endif
        
      }
       memset(responsePocket,0,24);
    }
  }
}
void ABUNA_FPS::analyzePocket(){
  
  if(isPocketMatch(RESPONSE_PACKET, responsePocket[ADDRESS_PREFIX_L], responsePocket[ADDRESS_PREFIX_H])){
      
    switch(mode){
      case IDENTIFY_MODE:
        if(isPocketMatch(CMD_IDENTIFY, responsePocket[ADDRESS_RESPONSE_COMMAND_L], responsePocket[ADDRESS_RESPONSE_COMMAND_H])){
          if(ERR_SUCCESS == responsePocket[ADDRESS_RESULT_L]){
            if(isPocketMatch(GD_NEED_RELEASE_FINGER, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
               
               response(ERR_SUCCESS, GD_NEED_RELEASE_FINGER, 0, "Lift Finger");
            }else{
               response(ERR_SUCCESS, IDENTIFY_MODE, responsePocket[ADDRESS_DATA], "Fingerprint Identified, ID:" + String(responsePocket[ADDRESS_DATA], HEX));
            }
          }else if (ERR_FAIL == responsePocket[ADDRESS_RESULT_L]){
            //Check Error
            if(isPocketMatch(ERR_ALL_EMPTY, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
              // No Fingerprint Enroll
              response(ERR_FAIL, ERR_ALL_EMPTY, 0, "No Fingerprint Enroll");
              
            }else if(isPocketMatch(ERR_IDENTIFY, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
              // No Fingerprint Enroll
              response(ERR_FAIL, ERR_IDENTIFY, 0, "Fingerprint Not Registered");

            }else if(isPocketMatch(ERR_BAD_QLT_IMG, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
              // Bad Quality Image
              response(ERR_FAIL, ERR_BAD_QLT_IMG, 0, "Bad Quality Image");
              
            } 
          }
        }
      break;
      case ENROLL_MODE:
        if(isPocketMatch(CMD_ENROLL, responsePocket[ADDRESS_RESPONSE_COMMAND_L], responsePocket[ADDRESS_RESPONSE_COMMAND_H])){
          if(ERR_SUCCESS == responsePocket[ADDRESS_RESULT_L]){
            if(isPocketMatch(GD_NEED_FIRST_SWEEP, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
                // Waiting input fingerprint for the first time.
                response(ERR_SUCCESS, GD_NEED_FIRST_SWEEP, 0, "Waiting input fingerprint for the first time");
                
              }else if(isPocketMatch(GD_NEED_SECOND_SWEEP, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
                // Waiting input fingerprint for the second time.
                response(ERR_SUCCESS, GD_NEED_SECOND_SWEEP, 0, "Waiting input fingerprint for the second time");
                
              }else if(isPocketMatch(GD_NEED_THIRD_SWEEP, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
                // Waiting input fingerprint for the third time.  
                response(ERR_SUCCESS, GD_NEED_THIRD_SWEEP, 0, "Waiting input fingerprint for the third time");
                
              }else if(isPocketMatch(GD_NEED_RELEASE_FINGER, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
                // Lift finger 
                response(ERR_SUCCESS, GD_NEED_RELEASE_FINGER, 0, "Lift Finger");
                
              }else if(ERR_SUCCESS == responsePocket[ADDRESS_DATA +1]){
                response(ERR_SUCCESS, ENROLL_MODE, responsePocket[ADDRESS_DATA], "Fingerprint Enrolled, ID:" + String(responsePocket[ADDRESS_DATA], HEX));
              // Success Enrollment of Fingerprint where Fingerprint Id is responsePocket[ADDRESS_DATA]
            }
          }else if (ERR_FAIL == responsePocket[ADDRESS_RESULT_L]){
            if(isPocketMatch(ERR_TMPL_NOT_EMPTY, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
              // Existed Template Data for the appointed ID
              response(ERR_FAIL, ERR_TMPL_NOT_EMPTY, 0, "Existed Template Data for the appointed ID");
              
            }else if(isPocketMatch(ERR_TMPL_ENROLLED, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
              // The fingerprint has been enrolled
              response(ERR_FAIL, ERR_TMPL_ENROLLED, responsePocket[ADDRESS_DATA + 2], "The fingerprint has been enrolled to ID. " + String(responsePocket[ADDRESS_DATA + 2], HEX));
            }
          }
        }
        break;
      case DELETE_MODE:
        if(isPocketMatch(CMD_CLEAR_ADDR, responsePocket[ADDRESS_RESPONSE_COMMAND_L], responsePocket[ADDRESS_RESPONSE_COMMAND_H])){
          if(ERR_SUCCESS == responsePocket[ADDRESS_RESULT_L]){
            response(ERR_SUCCESS, CMD_CLEAR_ADDR, responsePocket[ADDRESS_DATA], "Success Delete Template, ID:" + String(responsePocket[ADDRESS_DATA], HEX));
          }else if (ERR_FAIL == responsePocket[ADDRESS_RESULT_L]){
            if(isPocketMatch(ERR_TMPL_IS_NULL, responsePocket[ADDRESS_DATA], responsePocket[ADDRESS_DATA + 1])){
              // Existed Template Data for the appointed ID
              response(ERR_FAIL, ERR_TMPL_NOT_EMPTY, 0, "The appointed Template Data is Null");
            }
          }
        }
        break;
      case DELETE_ALL_MODE:
        
        if(isPocketMatch(CMD_CLEAR_ALL, responsePocket[ADDRESS_RESPONSE_COMMAND_L], responsePocket[ADDRESS_RESPONSE_COMMAND_H])){

          response(ERR_SUCCESS, DELETE_ALL_MODE, responsePocket[ADDRESS_DATA], "Successfully Delete All(" + String(responsePocket[ADDRESS_DATA], HEX) + ") Template/s");
        }
        break;      
    }
  }
}
boolean ABUNA_FPS::isPocketMatch(uint16_t pocketMatcher,uint8_t pocketLow, uint8_t pocketHigh){    
  return pocketLow == ((uint8_t)(pocketMatcher & 0xFF)) && 
    pocketHigh == ((uint8_t)((pocketMatcher>>8) & 0xFF));
}

/** Utils**/
void ABUNA_FPS::createRequestPocket(uint16_t prefix, uint16_t command, uint8_t data[], uint16_t dataSize){
 
  uint8_t i = 0;
  memset(requestPocket,0,24);
  // Prefix  
  requestPocket[0] = (uint8_t)(prefix & 0xFF);
  requestPocket[1] = (uint8_t)((prefix>>8) & 0xFF);
  
  // Command
  requestPocket[2] = (uint8_t)(command & 0xFF);
  requestPocket[3] = (uint8_t)((command>>8) & 0xFF);
  
  // Data Size
  requestPocket[4] = (uint8_t)(dataSize & 0xFF);
  requestPocket[5] = (uint8_t)((dataSize>>8) & 0xFF);

  // Data
  for(uint8_t i = 6; i <= 6 +DATA_SIZE - 1; i++){
    requestPocket[i] += data[i-6];
  } 

  // Checksum
  uint16_t checkSum = 0;
  for(i = 0; i <= 21; i++){
    checkSum += requestPocket[i];
  }
  requestPocket[22] = (uint8_t)(checkSum & 0xFF);
  requestPocket[23] = (uint8_t)((checkSum >> 8) & 0xFF);
 

  //Clear the buffer first
  while(fpsSerial->available()){
    fpsSerial->read();
  }

  fpsSerial->write(requestPocket, sizeof(requestPocket));
}

void ABUNA_FPS::sendPocket(uint8_t pocket[]){
  while(fpsSerial->available()){
    fpsSerial->read();
  }
  //clear buffer
  fpsSerial->flush();
  fpsSerial->write(pocket, sizeof(pocket));
}



/**  For Print use only **/
byte ABUNA_FPS::getMode(){
  return mode;
}

uint8_t * ABUNA_FPS::getRequestPocket(){
  // Todo: return response pocket
  return requestPocket;
}
uint8_t * ABUNA_FPS::getResponsePocket(){
  // Todo: return response pocket
  return responsePocket;
}

void ABUNA_FPS::setOnScannerResponseListener(void (*listener)(uint16_t resultCode, uint16_t responseCode, uint16_t data)){
  onScannerResponseListener = listener;
}
void ABUNA_FPS::set0nScannerResponseDetailedListener(void (*listener)(uint16_t resultCode, uint16_t responseCode, uint16_t data, String description)){
  onScannerResponseDetailedListener = listener;
}


void ABUNA_FPS::response(uint16_t resultCode, uint16_t responseCode, uint16_t data, String description){
  if(onScannerResponseDetailedListener != NULL){
    onScannerResponseDetailedListener(resultCode, responseCode, data, description);
  }else if(onScannerResponseListener != NULL){
    onScannerResponseListener(resultCode, responseCode, data);
  }
}
