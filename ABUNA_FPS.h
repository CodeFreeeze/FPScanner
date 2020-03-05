/*
  Finger Print Scanner Library na Nabibili sa eGizmo
  Copyright (c) 2010 Andrew E. Bunac 

  This Library is Created Just for Lazy student of the Philippines
*/
#ifndef ABUNA_FPS_h
#define ABUNA_FPS_h

#include <inttypes.h>
#include "Arduino.h" 

#ifdef __AVR__
  #include <SoftwareSerial.h>
#endif 

#define FPS_ERROR 200

/** FPS STATE MODE **/
#define STANDBY_MODE 0
#define IDENTIFY_MODE  1
#define ENROLL_MODE  2
#define DELETE_DODE  3
#define DELETE_ALL_MODE   4

/** RESPONSE ADDRESS**/
#define ADDRESS_PREFIX_L 0
#define ADDRESS_PREFIX_H 1
#define ADDRESS_RESPONSE_COMMAND_L 2
#define ADDRESS_RESPONSE_COMMAND_H 3
#define ADDRESS_LEN_L 4
#define ADDRESS_LEN_H 5
#define ADDRESS_RESULT_L 6
#define ADDRESS_RESULT_H 7
#define ADDRESS_DATA 8

#define REQUEST_DATA_LEN  16
#define RESPONSE_DATA_LEN  14


/*FPS Commands and Response Packets*/
#define FPS_RESPONSE_SIZE     (24) //48
#define FPS_PACKET_SIZE       (24)
#define DATA_SIZE             (16)
#define COMMAND_PACKET        (0xAA55)
#define RESPONSE_PACKET       (0x55AA)
#define COMMAND_DATA_PACKET   (0xA55A)
#define RESPONSE_DATA_PACKET  (0x5AA5)

#define SWEEP_FINGERPRINT       (0xFF)
#define GD_NEED_FIRST_SWEEP     (0xFFF1)
#define GD_NEED_SECOND_SWEEP    (0xFFF2)
#define GD_NEED_THIRD_SWEEP     (0xFFF3)
#define GD_NEED_RELEASE_FINGER  (0xFFF4)



#define CMD_STANDBY       (0x0117) 
#define CMD_IDENTIFY      (0x0102)
#define CMD_ENROLL        (0x0103)
#define CMD_ENROLL_ONE    (0x0104)
#define CMD_CLEAR_ADDR    (0x0105)
#define CMD_CLEAR_ALL     (0x0106)
#define CMD_ENROLL_COUNT  (0x0128)
#define CMD_CANCEL        (0x0130)
#define CMD_LED_CONTROL   (0x0124)


#define ERR_SUCCESS        (0x00)
#define ERR_FAIL           (0x01)
#define ERR_VERIFY         (0x11)
#define ERR_IDENTIFY       (0x12)
#define ERR_TMPL_IS_NULL   (0x13)
#define ERR_TMPL_NOT_EMPTY (0x14)
#define ERR_ALL_EMPTY      (0x15)
#define ERR_BAD_QLT_IMG    (0x15)
#define ERR_TMPL_ENROLLED  (0x19)

class ABUNA_FPS {
	private:
	  #ifdef __AVR__
  		SoftwareSerial *swSerial;
		#endif
  		HardwareSerial *hwSerial;
  		Stream *fpsSerial;
		byte fpScannerReceived[FPS_PACKET_SIZE];
		byte fpScannerSend[24];	
		uint8_t requestData[FPS_PACKET_SIZE] = {0};
    uint8_t requestPocket[FPS_PACKET_SIZE]; 
		uint8_t responsePocket[FPS_RESPONSE_SIZE];
		int CKS=0;
		byte mode = 0;

    void (*onScannerResponseListener)(uint16_t resultCode, uint16_t responseCode, uint16_t data);
    void (*onScannerResponseDetailedListener)(uint16_t resultCode, uint16_t responseCode, uint16_t data, String description);
    
    void response(uint16_t resultCode, uint16_t responseCode, uint16_t data);
    void createRequestPocket(uint16_t prefix, uint16_t command, uint8_t data[], uint16_t dataSize);  
    void sendPocket(uint8_t pocket[]);
    boolean isPocketMatch(uint16_t pocketMatcher,uint8_t pocketLow, uint8_t pocketHigh);
  
	public:
		#ifdef __AVR__
  		ABUNA_FPS(SoftwareSerial *swSerial);
		#endif
	  ABUNA_FPS(HardwareSerial *hwSerial);
    void begin(double baudRate);
    void monitorScanner();

    void standBy();
    void identify();
    void enroll(uint16_t templateId);
    void deleteById(uint16_t templateId);
    void deleteAll();
    void analyzePocket();

    int getMode();
    uint8_t * getRequestPocket();
    uint8_t * getResponsePocket();

    void response(uint16_t resultCode, uint16_t responseCode, uint16_t data, String description);
    void setOnScannerResponseListener(void (*listener)(uint16_t resultCode, uint16_t responseCode, uint16_t data));
    void set0nScannerResponseDetailedListener(void (*listener)(uint16_t resultCode, uint16_t responseCode, uint16_t data, String description));
};

#endif
