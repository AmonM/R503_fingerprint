#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
//Globalni parametri
uint8_t data[64];
uint8_t header[] = {0xEF, 0x1, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t capacity = 0;
uint16_t storedPrints = 0;
uint8_t secLvl = 0;
 
//Functions
uint8_t detect[] = {0x1, 0x0, 0x3, 0x1, 0x0,0x05};
uint8_t img2char1[] = {0x1, 0x0, 0x4, 0x2, 0x1, 0x0, 0x8};
uint8_t img2char2[] = {0x1, 0x0, 0x4, 0x2, 0x2, 0x0, 0x9};
uint8_t registerModel[] = {0x1, 0x0, 0x3, 0x5, 0x0, 0x9};
uint8_t saveModel[] = {0x1, 0x0, 0x6, 0x6, 0x1,0x0, 0x1, 0x0, 0x0};
uint8_t readParams[] = {0x1, 0x0, 0x3, 0xf, 0x0, 0x0};
uint8_t search[] = {0x1, 0x0, 0x8, 0x4, 0x1, 0x0, 0x0, 0x0, 0x10, 0x0, 0x0};
uint8_t numberOfTemplates[] = {0x1, 0x0, 0x3, 0x1d, 0x0, 0x21};
uint8_t auraControl [] = {0x1,0x0, 0x7, 0x35, 0x06, 0xFF, 0x3, 0xFF, 0x0, 0x0};

uint8_t sendCMD(uint8_t *command, uint8_t len){
  uint16_t sum = 0;

  for(uint8_t i = 0; i < len-2; i++){
    sum += command[i];
  }

  command[len-1] = sum;
  command[len-2] = (sum>>8);

  for(uint8_t i = 0; i < sizeof(header); i++){
    mySerial.write(header[i]);
  }

  for(uint8_t i = 0; i < len; i++){
    mySerial.write(command[i]);
  }

  while(!mySerial.available()){};

  uint8_t j = 0;
  while(mySerial.available()){
    data[j++] = mySerial.read();
  }
 
  return data[9];
} 

uint8_t addFinger(){
  Serial.println("Scan finger...");
  while(sendCMD(detect, sizeof(detect))){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\r\nImage created!");
  if(!sendCMD(img2char1, sizeof(img2char1))){
    Serial.println("\r\nConverted to character!");
  }else{
    Serial.println("\r\nConverting to character failed!");
  }

  Serial.println("Scan finger again...");
  while(sendCMD(detect, sizeof(detect))){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\r\nImage created!");
  if(!sendCMD(img2char2, sizeof(img2char1))){
    Serial.println("\r\nConverted to character!");
  }else{
    Serial.println("\r\nConverting to character failed!");
  }

  if(!sendCMD(registerModel, sizeof(registerModel))){
        Serial.println("\r\nTemplate generated!");
  }

  if(!sendCMD(saveModel, sizeof(saveModel))){
    Serial.println("\r\nTemplate stored!");
  }
  return 0;
}

uint8_t matchFinger(){
  search[9] = capacity;
  
  Serial.println("Scan finger...");
  while(sendCMD(detect, sizeof(detect))){
    Serial.print(".");
    delay(500);
  }
  
  if(!sendCMD(img2char1, sizeof(img2char1))){
    return sendCMD(search, sizeof(search));
  }else{
    
  }



  return 1;  
}

uint8_t readSysParams(){
  Serial.print(sendCMD(readParams,sizeof(readParams)));
  if(!sendCMD(readParams,sizeof(readParams))){
    capacity = data[15];
    Serial.println(capacity);
  }
  return 0;  
}

uint8_t numberOfTemplatesf(){
  if(!sendCMD(numberOfTemplates, sizeof(numberOfTemplates))){
    for(int i = 0; i < 64; i++){
      storedPrints =  data[10];
      storedPrints = (storedPrints<<8) | data[11];
    }
  }
  Serial.println();
  Serial.println(storedPrints);
  return 0;
}

uint8_t controlLed(uint8_t control, char* color, uint8_t howLong, uint8_t ledSpeed){
  //blue, purple, red
  uint8_t len = strlen(color);
  if(len != 3 && len != 4 && len != 6){
    return 1;
  }else{
    if(color[0] == 'p' && color[1] == 'u' && color[2] == 'r' && color[3] == 'p' && color[4] == 'l' && color[5] == 'e'){
      auraControl[6] = 0x03;
    }else if(color[0] == 'b' && color[1] == 'l' && color[2] == 'u' && color[3] == 'e'){
      auraControl[6] = 0x02;
    }else if(color[0] == 'r' && color[1] == 'e' && color[2] == 'd'){
      auraControl[6] = 0x01;
    }else{
      return 2;
    }
  }

  if(control < 0x01 || control > 0x06){
    return 3;
  }else{
      auraControl[4] = control;
  }

  auraControl[5] = ledSpeed;
  auraControl[7] = howLong;

  return sendCMD(auraControl, sizeof(auraControl));
  
}

void setup() {
  while (!Serial); // For Yun/Leo/Micro/Zero/…
  delay(500);
  
  Serial.begin(57600);
  Serial.println("Poskus prstnega odtisa.");
  delay(100);
  mySerial.begin(57600);
  delay(100);
  readSysParams();
  delay(100);
  numberOfTemplatesf();
  delay(100);
  if(storedPrints == 0){
    controlLed(0x01,"red",0x00, 0x01);
    addFinger();
  }else{
    controlLed(0x01,"purple",0x00, 0xff);
    while(1){
      uint8_t res = matchFinger();
        if(res == 0x00){
          controlLed(0x01,"blue",0x03, 0x10);
          delay(1200);
          break;
        }else if(res == 0x09){
          controlLed(0x01,"red",0x03, 0x10);
          delay(1200);
          controlLed(0x01,"purple",0x00, 0xff);
        }
      delay(100);
      }
  }

  controlLed(0x01,"purple",0x00, 0xff);
  Serial.println("done");
  
}

void loop() { // run over and over
}
