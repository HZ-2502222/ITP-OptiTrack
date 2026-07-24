#include <ArduCAM.h>
#include <memorysaver.h>
#include <ov5642_regs.h>


// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM Mini camera, and can run on any Arduino platform.
// This demo was made for ArduCAM_Mini_5MP_Plus.
// It needs to be used in combination with PC software.
// It can test OV5642 functions.
//

// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM_Mini_5MP_Plus
// and use Arduino IDE 1.6.8 compiler or above
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
//This demo can only work on OV5642_MINI_5MP_Plus  platform.
//#if !(defined OV5642_MINI_5MP_PLUS)
//  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
//#endif
#define BMPIMAGEOFFSET 66
const char bmp_header[BMPIMAGEOFFSET] PROGMEM =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};
// set pin 7 as the slave select for the digital pot:the 
const int CS = 7;
const int trigPin = 3;
const int echoPin = 2;
const int greenLED = 4;
const int yellowLED = 5;
const int redLED = 6;
int capture_timeout = 0;

bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;
 ArduCAM myCAM( OV5642, CS );
uint8_t read_fifo_burst(ArduCAM myCAM);
void setup() {
// put your setup code here, to run once:
uint8_t vid, pid;
uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Set default state to Idle (Red ON)
  digitalWrite(redLED, HIGH);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
#else
  Wire.begin();
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Set default state to Idle (Red ON)
  digitalWrite(redLED, HIGH);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
#endif
Serial.println(F("ACK CMD ArduCAM Start! END"));
// set the CS as an output:
pinMode(CS, OUTPUT);
digitalWrite(CS, HIGH);
// initialize SPI:
SPI.begin();
 //Reset the CPLD
myCAM.write_reg(0x07, 0x80);
delay(100);
myCAM.write_reg(0x07, 0x00);
delay(100); 
while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55){
    Serial.println(F("ACK CMD SPI interface Error! END"));
    delay(1000);continue;
  }else{
    Serial.println(F("ACK CMD SPI interface OK. END"));break;
  }
}
  while(1){
    //Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42)){
      Serial.println(F("ACK CMD Can't find OV5642 module! END"));
      delay(1000);continue;
    }
    else{
      Serial.println(F("ACK CMD OV5642 detected. END"));break;
    } 
  }
//Change to JPEG capture mode and initialize the OV5642 module
myCAM.set_format(JPEG);
myCAM.InitCAM();

  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  //myCAM.OV5642_set_JPEG_size(OV5642_320x240);
  myCAM.OV5642_set_JPEG_size(OV5642_640x480);
  //myCAM.OV5642_set_JPEG_size(OV5642_1024x768);
  myCAM.OV5642_set_Compress_quality(high_quality);
delay(1000);
myCAM.clear_fifo_flag();
myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
}
void loop() {
// put your main code here, to run repeatedly:
uint8_t temp = 0xff, temp_last = 0;
bool is_header = false;
if (Serial.available())
{
  temp = Serial.read();
  switch (temp)
  {
    case 'Z': // <-- USE UNIQUE BYTE 0xFF FOR WIRELESS RESET
      mode = 0;
      start_capture = 0;
      is_header = false;
      digitalWrite(redLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, LOW);
      Serial.println(F("ACK CMD Wireless Reset Successful. END"));
      temp = 0xff;
      break;
    case 0:
      myCAM.OV5642_set_JPEG_size(OV5642_320x240);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_320x240 END"));
    temp = 0xff;
    break;
    case 1:
      myCAM.OV5642_set_JPEG_size(OV5642_640x480);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_640x480 END"));
    temp = 0xff;
    break;
    case 2: 
      myCAM.OV5642_set_JPEG_size(OV5642_1024x768);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_1024x768 END"));
    temp = 0xff;
    break;
    case 3:
    temp = 0xff;
      myCAM.OV5642_set_JPEG_size(OV5642_1280x960);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_1280x960 END"));
    break;
    case 4:
    temp = 0xff;
      myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_1600x1200 END"));
    break;
    case 5:
    temp = 0xff;
      myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_2048x1536 END"));
    break;
    case 6:
    temp = 0xff;
      myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);delay(1000);
      Serial.println(F("ACK CMD switch to OV5642_2592x1944 END"));
    break;
    case 0x10:
    mode = 1;
    temp = 0xff;
    start_capture = 1;
    Serial.println(F("ACK CMD CAM start single shoot. END"));
    break;
    case 0x11: 
    temp = 0xff;
    myCAM.set_format(JPEG);
    myCAM.InitCAM();
    #if !(defined (OV2640_MINI_2MP))
    myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
    #endif
    break;
    case 0x20:
    mode = 2;
    temp = 0xff;
    start_capture = 2;
    Serial.println(F("ACK CMD CAM start video streaming. END"));
    break;
    case 0x30:
    mode = 3;
    temp = 0xff;
    start_capture = 3;
    Serial.println(F("ACK CMD CAM start single shoot. END"));
    break;
    case 0x31:
    temp = 0xff;
    myCAM.set_format(BMP);
    myCAM.InitCAM();     
    myCAM.clear_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
    myCAM.wrSensorReg16_8(0x3818, 0x81);
    myCAM.wrSensorReg16_8(0x3621, 0xA7);
    break;
    case 0x40:
    myCAM.OV5642_set_Light_Mode(Advanced_AWB);temp = 0xff;
     Serial.println(F("ACK CMD Set to Advanced_AWB END"));break;
    case 0x41:
    myCAM.OV5642_set_Light_Mode(Simple_AWB);temp = 0xff;
     Serial.println(F("ACK CMD Set to Simple_AWB END"));break;
     case 0x42:
    myCAM.OV5642_set_Light_Mode(Manual_day);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_day END"));break;
     case 0x43:
    myCAM.OV5642_set_Light_Mode(Manual_A);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_A END"));break;
     case 0x44:
    myCAM.OV5642_set_Light_Mode(Manual_cwf);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_cwf END"));break;
     case 0x45:
    myCAM.OV5642_set_Light_Mode(Manual_cloudy);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_cloudy END"));break;
      case 0x50:
    myCAM.OV5642_set_Color_Saturation(Saturation4);temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+4 END"));break;
   case 0x51:
      myCAM.OV5642_set_Color_Saturation(Saturation3);temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+3 END"));break;
   case 0x52:
    myCAM.OV5642_set_Color_Saturation(Saturation2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation+2 END"));break;
  case 0x53:
    myCAM.OV5642_set_Color_Saturation(Saturation1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation+1 END"));break;
   case 0x54:
    myCAM.OV5642_set_Color_Saturation(Saturation0);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation+0 END"));break;
   case 0x55:
    myCAM.OV5642_set_Color_Saturation(Saturation_1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-1 END"));break;
   case 0x56:
    myCAM.OV5642_set_Color_Saturation(Saturation_2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-2"));break;
    case 0x57:
    myCAM.OV5642_set_Color_Saturation(Saturation_3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-3 END"));break;
   case 0x58:
  myCAM.OV5642_set_Light_Mode(Saturation_4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-4 END"));break; 
   case 0x60:
  myCAM.OV5642_set_Brightness(Brightness4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+4 END"));break;
  case 0x61:
  myCAM.OV5642_set_Brightness(Brightness3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+3 END"));break; 
  case 0x62:
  myCAM.OV5642_set_Brightness(Brightness2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+2 END"));break; 
   case 0x63:
  myCAM.OV5642_set_Brightness(Brightness1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+1 END"));break; 
   case 0x64:
  myCAM.OV5642_set_Brightness(Brightness0);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+0 END"));break; 
    case 0x65:
  myCAM.OV5642_set_Brightness(Brightness_1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-1 END"));break; 
     case 0x66:
  myCAM.OV5642_set_Brightness(Brightness_2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-2 END"));break; 
    case 0x67:
  myCAM.OV5642_set_Brightness(Brightness_3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-3 END"));break; 
    case 0x68:
  myCAM.OV5642_set_Brightness(Brightness_4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-4 END"));break;
case 0x70:
  myCAM.OV5642_set_Contrast(Contrast4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+4 END"));break;
  case 0x71:
  myCAM.OV5642_set_Contrast(Contrast3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+3 END"));break; 
  case 0x72:
  myCAM.OV5642_set_Contrast(Contrast2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+2 END"));break; 
   case 0x73:
  myCAM.OV5642_set_Contrast(Contrast1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+1 END"));break; 
   case 0x74:
  myCAM.OV5642_set_Contrast(Contrast0);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+0 END"));break; 
    case 0x75:
  myCAM.OV5642_set_Contrast(Contrast_1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-1 END"));break; 
     case 0x76:
  myCAM.OV5642_set_Contrast(Contrast_2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-2 END"));break; 
    case 0x77:
  myCAM.OV5642_set_Contrast(Contrast_3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-3 END"));break; 
    case 0x78:
  myCAM.OV5642_set_Contrast(Contrast_4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-4 END"));break;
   case 0x80: 
    myCAM.OV5642_set_hue(degree_180);temp = 0xff;
     Serial.println(F("ACK CMD Set to -180 degree END"));break;   
   case 0x81: 
   myCAM.OV5642_set_hue(degree_150);temp = 0xff;
     Serial.println(F("ACK CMD Set to -150 degree END"));break;  
   case 0x82: 
   myCAM.OV5642_set_hue(degree_120);temp = 0xff;
     Serial.println(F("ACK CMD Set to -120 degree END"));break;  
   case 0x83: 
   myCAM.OV5642_set_hue(degree_90);temp = 0xff;
     Serial.println(F("ACK CMD Set to -90 degree END"));break;   
    case 0x84: 
   myCAM.OV5642_set_hue(degree_60);temp = 0xff;
     Serial.println(F("ACK CMD Set to -60 degree END"));break;   
    case 0x85: 
   myCAM.OV5642_set_hue(degree_30);temp = 0xff;
     Serial.println(F("ACK CMD Set to -30 degree END"));break;  
     case 0x86: 
   myCAM.OV5642_set_hue(degree_0);temp = 0xff;
     Serial.println(F("ACK CMD Set to 0 degree END"));break; 
   case 0x87: 
   myCAM.OV5642_set_hue(degree30);temp = 0xff;
     Serial.println(F("ACK CMD Set to 30 degree END"));break;
   case 0x88: 
   myCAM.OV5642_set_hue(degree60);temp = 0xff;
     Serial.println(F("ACK CMD Set to 60 degree END"));break;
    case 0x89: 
   myCAM.OV5642_set_hue(degree90);temp = 0xff;
     Serial.println(F("ACK CMD Set to 90 degree END"));break;
     case 0x8a: 
   myCAM.OV5642_set_hue(degree120);temp = 0xff;
     Serial.println(F("ACK CMD Set to 120 degree END"));break ; 
   case 0x8b: 
   myCAM.OV5642_set_hue(degree150);temp = 0xff;
     Serial.println(F("ACK CMD Set to 150 degree END"));break ;
   case 0x90: 
   myCAM.OV5642_set_Special_effects(Normal);temp = 0xff;
     Serial.println(F("ACK CMD Set to Normal END"));break ;
      case 0x91: 
   myCAM.OV5642_set_Special_effects(BW);temp = 0xff;
     Serial.println(F("ACK CMD Set to BW END"));break ;
    case 0x92: 
   myCAM.OV5642_set_Special_effects(Bluish);temp = 0xff;
     Serial.println(F("ACK CMD Set to Bluish END"));break ;
      case 0x93: 
   myCAM.OV5642_set_Special_effects(Sepia);temp = 0xff;
     Serial.println(F("ACK CMD Set to Sepia END"));break ;
    case 0x94: 
   myCAM.OV5642_set_Special_effects(Reddish);temp = 0xff;
     Serial.println(F("ACK CMD Set to Reddish END"));break ;
   case 0x95: 
   myCAM.OV5642_set_Special_effects(Greenish);temp = 0xff;
     Serial.println(F("ACK CMD Set to Greenish END"));break ;
   case 0x96: 
   myCAM.OV5642_set_Special_effects(Negative);temp = 0xff;
     Serial.println(F("ACK CMD Set to Negative END"));break ;
   case 0xA0: 
   myCAM.OV5642_set_Exposure_level(Exposure_17_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -1.7EV"));break ;  
     case 0xA1: 
   myCAM.OV5642_set_Exposure_level(Exposure_13_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -1.3EV END"));break ;
      case 0xA2: 
   myCAM.OV5642_set_Exposure_level(Exposure_10_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -1.0EV END"));break ; 
    case 0xA3: 
   myCAM.OV5642_set_Exposure_level(Exposure_07_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -0.7EV END"));break ;
     case 0xA4: 
   myCAM.OV5642_set_Exposure_level(Exposure_03_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -0.3EV END"));break ;
   case 0xA5: 
   myCAM.OV5642_set_Exposure_level(Exposure_default);temp = 0xff;
     Serial.println(F("ACK CMD Set to -Exposure_default END"));break ;
    case 0xA6: 
   myCAM.OV5642_set_Exposure_level(Exposure07_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 0.7EV END"));break ;  
   case 0xA7: 
   myCAM.OV5642_set_Exposure_level(Exposure10_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 1.0EV END"));break ;
    case 0xA8: 
   myCAM.OV5642_set_Exposure_level(Exposure13_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 1.3EV END"));break ; 
    case 0xA9: 
   myCAM.OV5642_set_Exposure_level(Exposure17_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 1.7EV END"));break ; 
   case 0xB0: 
   myCAM.OV5642_set_Sharpness(Auto_Sharpness_default);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Sharpness default END"));break ; 
    case 0xB1: 
   myCAM.OV5642_set_Sharpness(Auto_Sharpness1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Sharpness +1 END"));break ; 
    case 0xB2: 
   myCAM.OV5642_set_Sharpness(Auto_Sharpness2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Sharpness +2 END"));break ; 
      case 0xB3: 
   myCAM.OV5642_set_Sharpness(Manual_Sharpnessoff);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness off END"));break ; 
     case 0xB4: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +1 END"));break ;
     case 0xB5: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +2 END"));break ; 
     case 0xB6: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness3);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +3 END"));break ;
     case 0xB7: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness4);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +4 END"));break ;
    case 0xB8: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness5);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +5 END"));break ;  
    case 0xC0: 
     myCAM.OV5642_set_Mirror_Flip(MIRROR);temp = 0xff;
     Serial.println(F("ACK CMD Set to MIRROR END"));break ;  
    case 0xC1: 
     myCAM.OV5642_set_Mirror_Flip(FLIP);temp = 0xff;
     Serial.println(F("ACK CMD Set to FLIP END"));break ; 
    case 0xC2: 
     myCAM.OV5642_set_Mirror_Flip(MIRROR_FLIP);temp = 0xff;
     Serial.println(F("ACK CMD Set to MIRROR&FLIP END"));break ;
    case 0xC3: 
     myCAM.OV5642_set_Mirror_Flip(Normal);temp = 0xff;
     Serial.println(F("ACK CMD Set to Normal END"));break ;
     case 0xD0: 
     myCAM.OV5642_set_Compress_quality(high_quality);temp = 0xff;
     Serial.println(F("ACK CMD Set to high quality END"));break ;
      case 0xD1: 
     myCAM.OV5642_set_Compress_quality(default_quality);temp = 0xff;
     Serial.println(F("ACK CMD Set to default quality END"));break ;
      case 0xD2: 
     myCAM.OV5642_set_Compress_quality(low_quality);temp = 0xff;
     Serial.println(F("ACK CMD Set to low quality END"));break ;

      case 0xE0: 
     myCAM.OV5642_Test_Pattern(Color_bar);temp = 0xff;
     Serial.println(F("ACK CMD Set to Color bar END"));break ;
      case 0xE1: 
     myCAM.OV5642_Test_Pattern(Color_square);temp = 0xff;
     Serial.println(F("ACK CMD Set to Color square END"));break ;
      case 0xE2: 
     myCAM.OV5642_Test_Pattern(BW_square);temp = 0xff;
     Serial.println(F("ACK CMD Set to BW square END"));break ;
     case 0xE3: 
     myCAM.OV5642_Test_Pattern(DLI);temp = 0xff;
     Serial.println(F("ACK CMD Set to DLI END"));break ;
     case 0xEE:
        mode = 99; // 99 is our emergency lockdown state
        Serial.println(F("ACK CMD EMERGENCY STOP ACTIVATED. END"));
        break;
      
      default:
      break;
  }
}
if (mode == 1)
{
  if (start_capture == 1)
  {
    myCAM.clear_fifo_flag();
    myCAM.flush_fifo();
    delay(10); 
    myCAM.clear_fifo_flag();
    
    // Give the camera's auto-exposure 200ms to adjust
    delay(500); 
    
    //Start capture
    myCAM.start_capture();
    start_capture = 0;
    
    capture_timeout = 0; 
    digitalWrite(yellowLED, HIGH); 
  }
  
  if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println(F("ACK CMD CAM Capture Done. END"));delay(50);
    
    // Check if the frame was actually a valid JPEG
    uint8_t capture_success = read_fifo_burst(myCAM);
    myCAM.clear_fifo_flag();
    
    if (capture_success == 1) {
        // Picture taken successfully!
        digitalWrite(yellowLED, LOW);
        digitalWrite(greenLED, HIGH);
        Serial.println("Capture success! Green ON for 2 seconds...");
        
        delay(2000);
        
        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, LOW);
        Serial.println("Entering Cooldown. Blinking yellow until vehicle clears...");
        mode = 4;
        capture_timeout = 0; 
    } 
    else {
        // Corrupted frame caught! Deep flush the buffer.
        Serial.println("Corrupted frame detected. Deep flushing buffer...");
        myCAM.clear_fifo_flag();
        myCAM.flush_fifo();
        delay(500); // Give the CPLD memory chip time to recover
        
        // Ping the sensor: Is the car still there?
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        float checkDist = (pulseIn(echoPin, HIGH) * 0.0343) / 2;
        
        if (checkDist >= 15.0 || checkDist == 0.0) {
            Serial.println("Vehicle already left! Aborting retry. Resetting to Idle.");
            digitalWrite(yellowLED, LOW);
            digitalWrite(redLED, HIGH);
            mode = 0;
            capture_timeout = 0;
        } else {
            Serial.println("Vehicle still present. Retrying capture...");
            digitalWrite(yellowLED, HIGH);
            start_capture = 1; 
            capture_timeout = 0;
        }
    }
  }
  // Hardware Timeout Failsafe
  else if (start_capture == 0) 
  {
      capture_timeout++;
      
      if (capture_timeout >= 6) {
          Serial.println("CAMERA HARDWARE TIMEOUT! Deep flushing and aborting...");
          
          myCAM.clear_fifo_flag(); 
          myCAM.flush_fifo();
          delay(500);
          
          // Drop back to idle to prevent an infinite hanging loop
          digitalWrite(yellowLED, LOW);
          digitalWrite(redLED, HIGH);
          mode = 0; 
          capture_timeout = 0;
      }
  }
}
else if (mode == 2)
{
  while (1)
  {
    temp = Serial.read();
    if (temp == 0x21)
    {
      start_capture = 0;
      mode = 0;
      Serial.println(F("ACK CMD CAM stop video streaming. END"));
      break;
    }
    switch(temp){
       case 0x40:
    myCAM.OV5642_set_Light_Mode(Advanced_AWB);temp = 0xff;
     Serial.println(F("ACK CMD Set to Advanced_AWB END"));break;
    case 0x41:
    myCAM.OV5642_set_Light_Mode(Simple_AWB);temp = 0xff;
     Serial.println(F("ACK CMD Set to Simple_AWB END"));break;
     case 0x42:
    myCAM.OV5642_set_Light_Mode(Manual_day);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_day END"));break;
     case 0x43:
    myCAM.OV5642_set_Light_Mode(Manual_A);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_A END"));break;
     case 0x44:
    myCAM.OV5642_set_Light_Mode(Manual_cwf);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_cwf END"));break;
     case 0x45:
    myCAM.OV5642_set_Light_Mode(Manual_cloudy);temp = 0xff;
     Serial.println(F("ACK CMD Set to Manual_cloudy END"));break;
      case 0x50:
    myCAM.OV5642_set_Color_Saturation(Saturation4);temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+4 END"));break;
   case 0x51:
      myCAM.OV5642_set_Color_Saturation(Saturation3);temp = 0xff;
     Serial.println(F("ACK CMD Set to Saturation+3 END"));break;
   case 0x52:
    myCAM.OV5642_set_Color_Saturation(Saturation2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation+2 END"));break;
  case 0x53:
    myCAM.OV5642_set_Color_Saturation(Saturation1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation+1 END"));break;
   case 0x54:
    myCAM.OV5642_set_Color_Saturation(Saturation0);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation+0 END"));break;
   case 0x55:
    myCAM.OV5642_set_Color_Saturation(Saturation_1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-1 END"));break;
   case 0x56:
    myCAM.OV5642_set_Color_Saturation(Saturation_2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-2 END"));break;
    case 0x57:
    myCAM.OV5642_set_Color_Saturation(Saturation_3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-3 END"));break;
   case 0x58:
  myCAM.OV5642_set_Light_Mode(Saturation_4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Saturation-4 END"));break; 
   case 0x60:
  myCAM.OV5642_set_Brightness(Brightness4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+4 END"));break;
  case 0x61:
  myCAM.OV5642_set_Brightness(Brightness3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+3 END"));break; 
  case 0x62:
  myCAM.OV5642_set_Brightness(Brightness2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+2 END"));break; 
   case 0x63:
  myCAM.OV5642_set_Brightness(Brightness1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+1 END"));break; 
   case 0x64:
  myCAM.OV5642_set_Brightness(Brightness0);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness+0 END"));break; 
    case 0x65:
  myCAM.OV5642_set_Brightness(Brightness_1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-1 END"));break; 
     case 0x66:
  myCAM.OV5642_set_Brightness(Brightness_2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-2 END"));break; 
    case 0x67:
  myCAM.OV5642_set_Brightness(Brightness_3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-3 END"));break; 
    case 0x68:
  myCAM.OV5642_set_Brightness(Brightness_4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Brightness-4 END"));break;
case 0x70:
  myCAM.OV5642_set_Contrast(Contrast4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+4 END"));break;
  case 0x71:
  myCAM.OV5642_set_Contrast(Contrast3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+3 END"));break; 
  case 0x72:
  myCAM.OV5642_set_Contrast(Contrast2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+2 END"));break; 
   case 0x73:
  myCAM.OV5642_set_Contrast(Contrast1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+1 END"));break; 
   case 0x74:
  myCAM.OV5642_set_Contrast(Contrast0);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast+0 END"));break; 
    case 0x75:
  myCAM.OV5642_set_Contrast(Contrast_1);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-1 END"));break; 
     case 0x76:
  myCAM.OV5642_set_Contrast(Contrast_2);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-2 END"));break; 
    case 0x77:
  myCAM.OV5642_set_Contrast(Contrast_3);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-3 END"));break; 
    case 0x78:
  myCAM.OV5642_set_Contrast(Contrast_4);temp = 0xff;
   Serial.println(F("ACK CMD Set to Contrast-4 END"));break;
   case 0x80: 
    myCAM.OV5642_set_hue(degree_180);temp = 0xff;
     Serial.println(F("ACK CMD Set to -180 degree END"));break;   
   case 0x81: 
   myCAM.OV5642_set_hue(degree_150);temp = 0xff;
     Serial.println(F("ACK CMD Set to -150 degree END"));break;  
   case 0x82: 
   myCAM.OV5642_set_hue(degree_120);temp = 0xff;
     Serial.println(F("ACK CMD Set to -120 degree END"));break;  
   case 0x83: 
   myCAM.OV5642_set_hue(degree_90);temp = 0xff;
     Serial.println(F("ACK CMD Set to -90 degree END"));break;   
    case 0x84: 
   myCAM.OV5642_set_hue(degree_60);temp = 0xff;
     Serial.println(F("ACK CMD Set to -60 degree END"));break;   
    case 0x85: 
   myCAM.OV5642_set_hue(degree_30);temp = 0xff;
     Serial.println(F("ACK CMD Set to -30 degree END"));break;  
     case 0x86: 
   myCAM.OV5642_set_hue(degree_0);temp = 0xff;
     Serial.println(F("ACK CMD Set to 0 degree END"));break; 
   case 0x87: 
   myCAM.OV5642_set_hue(degree30);temp = 0xff;
     Serial.println(F("ACK CMD Set to 30 degree END"));break;
   case 0x88: 
   myCAM.OV5642_set_hue(degree60);temp = 0xff;
     Serial.println(F("ACK CMD Set to 60 degree END"));break;
    case 0x89: 
   myCAM.OV5642_set_hue(degree90);temp = 0xff;
     Serial.println(F("ACK CMD Set to 90 degree END"));break;
     case 0x8a: 
   myCAM.OV5642_set_hue(degree120);temp = 0xff;
     Serial.println(F("ACK CMD Set to 120 degree END"));break ; 
   case 0x8b: 
   myCAM.OV5642_set_hue(degree150);temp = 0xff;
     Serial.println(F("ACK CMD Set to 150 degree END"));break ;
  case 0x90: 
   myCAM.OV5642_set_Special_effects(Normal);temp = 0xff;
     Serial.println(F("ACK CMD Set to Normal END"));break ;
      case 0x91: 
   myCAM.OV5642_set_Special_effects(BW);temp = 0xff;
     Serial.println(F("ACK CMD Set to BW END"));break ;
    case 0x92: 
   myCAM.OV5642_set_Special_effects(Bluish);temp = 0xff;
     Serial.println(F("ACK CMD Set to Bluish END"));break ;
      case 0x93: 
   myCAM.OV5642_set_Special_effects(Sepia);temp = 0xff;
     Serial.println(F("ACK CMD Set to Sepia END"));break ;
    case 0x94: 
   myCAM.OV5642_set_Special_effects(Reddish);temp = 0xff;
     Serial.println(F("ACK CMD Set to Reddish END"));break ;
   case 0x95: 
   myCAM.OV5642_set_Special_effects(Greenish);temp = 0xff;
     Serial.println(F("ACK CMD Set to Greenish END"));break ;
   case 0x96: 
   myCAM.OV5642_set_Special_effects(Negative);temp = 0xff;
     Serial.println(F("ACK CMD Set to Negative END"));break ;
   case 0xA0: 
   myCAM.OV5642_set_Exposure_level(Exposure_17_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -1.7EV END"));break ;  
     case 0xA1: 
   myCAM.OV5642_set_Exposure_level(Exposure_13_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -1.3EV END"));break ;
      case 0xA2: 
   myCAM.OV5642_set_Exposure_level(Exposure_10_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -1.0EV END"));break ; 
    case 0xA3: 
   myCAM.OV5642_set_Exposure_level(Exposure_07_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -0.7EV END"));break ;
     case 0xA4: 
   myCAM.OV5642_set_Exposure_level(Exposure_03_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to -0.3EV END"));break ;
   case 0xA5: 
   myCAM.OV5642_set_Exposure_level(Exposure_default);temp = 0xff;
     Serial.println(F("ACK CMD Set to -Exposure_default END"));break ;
    case 0xA6: 
   myCAM.OV5642_set_Exposure_level(Exposure07_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 0.7EV END"));break ;  
   case 0xA7: 
   myCAM.OV5642_set_Exposure_level(Exposure10_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 1.0EV END"));break ;
    case 0xA8: 
   myCAM.OV5642_set_Exposure_level(Exposure13_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 1.3EV END"));break ; 
    case 0xA9: 
   myCAM.OV5642_set_Exposure_level(Exposure17_EV);temp = 0xff;
     Serial.println(F("ACK CMD Set to 1.7EV END"));break ; 
   case 0xB0: 
   myCAM.OV5642_set_Sharpness(Auto_Sharpness_default);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Sharpness default END"));break ; 
    case 0xB1: 
   myCAM.OV5642_set_Sharpness(Auto_Sharpness1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Sharpness +1 END"));break ; 
    case 0xB2: 
   myCAM.OV5642_set_Sharpness(Auto_Sharpness2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Sharpness +2 END"));break ; 
      case 0xB3: 
   myCAM.OV5642_set_Sharpness(Manual_Sharpnessoff);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness off END"));break ; 
     case 0xB4: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness1);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +1 END"));break ;
     case 0xB5: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness2);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +2 END"));break ; 
     case 0xB6: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness3);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +3 END"));break ;
     case 0xB7: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness4);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +4 END"));break ;
    case 0xB8: 
     myCAM.OV5642_set_Sharpness(Manual_Sharpness5);temp = 0xff;
     Serial.println(F("ACK CMD Set to Auto Manual Sharpness +5 END"));break ;  
    case 0xC0: 
     myCAM.OV5642_set_Mirror_Flip(MIRROR);temp = 0xff;
     Serial.println(F("ACK CMD Set to MIRROR END"));break ;  
    case 0xC1: 
     myCAM.OV5642_set_Mirror_Flip(FLIP);temp = 0xff;
     Serial.println(F("ACK CMD Set to FLIP END"));break ; 
    case 0xC2: 
     myCAM.OV5642_set_Mirror_Flip(MIRROR_FLIP);temp = 0xff;
     Serial.println(F("ACK CMD Set to MIRROR&FLIP END"));break ;
    case 0xC3: 
     myCAM.OV5642_set_Mirror_Flip(Normal);temp = 0xff;
     Serial.println(F("ACK CMD Set to Normal END"));break ;
     case 0xD0: 
     myCAM.OV5642_set_Compress_quality(high_quality);temp = 0xff;
     Serial.println(F("ACK CMD Set to high quality END"));break ;
      case 0xD1: 
     myCAM.OV5642_set_Compress_quality(default_quality);temp = 0xff;
     Serial.println(F("ACK CMD Set to default quality END"));break ;
      case 0xD2: 
     myCAM.OV5642_set_Compress_quality(low_quality);temp = 0xff;
     Serial.println(F("ACK CMD Set to low quality END"));break ;

      case 0xE0: 
     myCAM.OV5642_Test_Pattern(Color_bar);temp = 0xff;
     Serial.println(F("ACK CMD Set to Color bar END"));break ;
      case 0xE1: 
     myCAM.OV5642_Test_Pattern(Color_square);temp = 0xff;
     Serial.println(F("ACK CMD Set to Color square END"));break ;
      case 0xE2: 
     myCAM.OV5642_Test_Pattern(BW_square);temp = 0xff;
     Serial.println(F("ACK CMD Set to BW square END"));break ;
     case 0xE3: 
     myCAM.OV5642_Test_Pattern(DLI);temp = 0xff;
     Serial.println(F("ACK CMD Set to DLI END"));break ;
      
      }
    if (start_capture == 2)
    {
      myCAM.flush_fifo();
      myCAM.clear_fifo_flag();
      //Start capture
      myCAM.start_capture();
      start_capture = 0;
    }
    if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    {
      uint32_t length = 0;
      length = myCAM.read_fifo_length();
      if ((length >= MAX_FIFO_SIZE) | (length == 0))
      {
        myCAM.clear_fifo_flag();
        start_capture = 2;
        continue;
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();//Set fifo burst mode
      temp =  SPI.transfer(0x00);
      length --;
      while ( length-- )
      {
        temp_last = temp;
        temp =  SPI.transfer(0x00);
        if (is_header == true)
        {
          Serial.write(temp);
        }
        else if ((temp == 0xD8) & (temp_last == 0xFF))
        {
          is_header = true;
          Serial.println(F("ACK IMG END"));
          Serial.write(temp_last);
          Serial.write(temp);
        }
        if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
        break;
        delayMicroseconds(4);
      }
      myCAM.CS_HIGH();
      myCAM.clear_fifo_flag();
      start_capture = 2;
      is_header = false;
    }
  }
}
else if (mode == 3)
{
  if (start_capture == 3)
  {
    //Flush the FIFO
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    //Start capture
    myCAM.start_capture();
    start_capture = 0;
  }
  if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println(F("ACK CMD CAM Capture Done. END"));delay(50);
    uint8_t temp, temp_last;
    uint32_t length = 0;
    length = myCAM.read_fifo_length();
    if (length >= MAX_FIFO_SIZE ) 
    {
      Serial.println(F("ACK CMD Over size. END"));
      myCAM.clear_fifo_flag();
      return;
    }
    if (length == 0 ) //0 kb
    {
      Serial.println(F("ACK CMD Size is 0. END"));
      myCAM.clear_fifo_flag();
      return;
    }
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();//Set fifo burst mode

    
    //need to clear the stupid flag sequence
    Serial.write(0xFF);
    Serial.write(0xAA);
    for (temp = 0; temp < BMPIMAGEOFFSET; temp++)
    {
      Serial.write(pgm_read_byte(&bmp_header[temp]));
    }
    //SPI.transfer(0x00);
    char VH, VL;
    int i = 0, j = 0;
    for (i = 0; i < 240; i++)
    {
      for (j = 0; j < 320; j++)
      {
        VH = SPI.transfer(0x00);;
        VL = SPI.transfer(0x00);;
        Serial.write(VL);
        delayMicroseconds(12);
        Serial.write(VH);
        delayMicroseconds(12);
      }
    }
    Serial.write(0xBB);
    Serial.write(0xCC);
    myCAM.CS_HIGH();
    myCAM.clear_fifo_flag();
  }
}
// Ultrasonic sensor
// Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the Echo pin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance (Speed of sound is 343 m/s or 0.0343 cm/us)
  // Divide by 2 because the sound wave travels to the object and back
  float distanceCm = (duration * 0.0343) / 2;
  
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // --- STATE 0 to 1: Vehicle detected ---
  if (distanceCm < 15.0 && distanceCm > 0.0 && mode == 0) {
      Serial.println("Target in range! Blinking yellow for 5 seconds...");
      
      // Turn off Red
      digitalWrite(redLED, LOW);
      
      // 5 full blinks in 5 seconds (500ms HIGH, 500ms LOW)
      for (int i = 0; i < 5; i++) {
          digitalWrite(yellowLED, HIGH);
          delay(500); 
          digitalWrite(yellowLED, LOW);
          delay(500); 
      }
      
      mode = 1;
      start_capture = 1;
      Serial.println(F("ACK CMD CAM start single shoot. END"));
  }

  // --- STATE 4: Cooldown, waiting for vehicle to leave ---
  if (mode == 4) {
      // Check if the car is still parked there
      if (distanceCm < 15.0 && distanceCm > 0.0) {
          // Blink yellow: ON for 500ms
          digitalWrite(yellowLED, HIGH);
          delay(500); 
          digitalWrite(yellowLED, LOW);
          // Note: The delay(500) at the very end of your main loop acts as the 500ms OFF time!
      } 
      // If the car has left (distance >= 15 or sensor reads 0.0)
      else {
          Serial.println("Vehicle cleared. Resetting state machine to Idle.");
          
          // Turn Yellow OFF completely, turn Red ON
          digitalWrite(yellowLED, LOW);
          digitalWrite(redLED, HIGH);
          
          // Unlock the system back to Idle
          mode = 0;
      }
  }
  
  // --- STATE 99: EMERGENCY LOCKDOWN ---
  if (mode == 99) {
      // Turn off everything else
      digitalWrite(greenLED, LOW);
      digitalWrite(yellowLED, LOW);
      
      // Allow it to listen for a reset command even during lockdown
      unsigned long startTime = millis();
      while(millis() - startTime < 200) { // Check for 200ms intervals
          if (Serial.available()) {
              char cmd = Serial.read();
              if (cmd == 'Z') {
                  mode = 0;
                  digitalWrite(redLED, HIGH);
                  return; // Exit lockdown back to normal execution
              }
          }
          digitalWrite(redLED, HIGH);
          delay(100);
          digitalWrite(redLED, LOW);
          delay(100);
      }
  }

  delay(500); // Sensor pings twice a second
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool header_found = false; 
  int chunk_counter = 0; // NEW: Counter to track bytes sent

  length = myCAM.read_fifo_length();
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) 
  {
    Serial.println(F("ACK CMD Over size. END"));
    return 0;
  }
  if (length == 0 ) 
  {
    Serial.println(F("ACK CMD Size is 0. END"));
    return 0;
  }
  
  Serial.flush();
  delay(50); 

  myCAM.CS_LOW();
  myCAM.set_fifo_burst(); 
  temp =  SPI.transfer(0x00);
  length --;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    
    if (is_header == true)
    {
      Serial.write(temp);
      chunk_counter++; // Count the byte
    }
    else if ((temp == 0xD8) && (temp_last == 0xFF))
    {
      is_header = true;
      header_found = true; 
      Serial.println(F("ACK IMG END"));
      Serial.write(temp_last);
      Serial.write(temp);
      chunk_counter += 2;
    }
    
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) 
      break;
      
    // --- WIRELESS FLOW CONTROL ---
    // If we've sent 64 bytes, pause to let the HC-05 breathe
    if (chunk_counter >= 64) {
        Serial.flush(); // Wait for Arduino's transmit buffer to empty
        delay(2);       // Give the HC-05 2ms to beam the data through the air
        chunk_counter = 0; // Reset counter for the next chunk
    }
  }
  myCAM.CS_HIGH();
  
  Serial.flush();
  delay(50);
  Serial.println(); // Drop to a clean line after the binary stream
  
  is_header = false;
  return header_found ? 1 : 0;
}