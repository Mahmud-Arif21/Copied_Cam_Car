#include "esp_camera.h"
#include <WiFi.h>
//#include <BlynkSimpleEsp32.h>
#include <WiFiClient.h>

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//
// Adafruit ESP32 Feather

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_M5STACK_PSRAM
#define CAMERA_MODEL_AI_THINKER

const char* ssid = "Orion";   //Enter SSID WIFI Name
const char* password = "hellomoto";   //Enter WIFI Password

//char auth[] = "55zN59Pq734i3MG33WcoRJAgpJEVWKdF";

#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22


#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else
#error "Camera model not selected"
#endif

// GPIO Setting
extern int gpLb =  2; // Left 1
extern int gpLf = 14; // Left 2
extern int gpRb = 15; // Right 1
extern int gpRf = 13; // Right 2
extern int gpLed =  4; // Light
//pins to drive motors
int MotorLeft[2] = {gpLb,gpLf};  
int MotorRight[2] = {gpRf,gpRb};

extern String WiFiAddr ="";

void startCameraServer();



/*// Set your Static IP address
IPAddress local_IP(192, 168, 0, 200);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional*/


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  

  pinMode(gpLb, OUTPUT); //Left Backward
  pinMode(gpLf, OUTPUT); //Left Forward
  pinMode(gpRb, OUTPUT); //Right Forward
  pinMode(gpRf, OUTPUT); //Right Backward
  pinMode(gpLed, OUTPUT); //Light

  //initialize
  digitalWrite(gpLb, LOW);
  digitalWrite(gpLf, LOW);
  digitalWrite(gpRb, LOW);
  digitalWrite(gpRf, LOW);
  digitalWrite(gpLed, LOW);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);

  /*// Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }*/

  WiFi.begin(ssid, password);
  //Blynk.begin(auth, ssid, password);
  MotorInit();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  WiFiAddr = WiFi.localIP().toString();
  Serial.println("' to connect");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Blynk.run();

}

//Intialize the motor
void MotorInit()
{
  int i;
  for(i=0 ; i<2; i++)
  {
  pinMode(MotorLeft[i],OUTPUT);
  pinMode(MotorRight[i],OUTPUT);
  }
}
//Robot Driving Functions
void Robot_Forward()
{
   digitalWrite(MotorLeft[0],0);
   digitalWrite(MotorLeft[1],1);
   digitalWrite(MotorRight[0],1);
   digitalWrite(MotorRight[1],0);   
}
void Robot_Backward()
{
   digitalWrite(MotorLeft[0],1);
   digitalWrite(MotorLeft[1],0);
   digitalWrite(MotorRight[0],0);
   digitalWrite(MotorRight[1],1);  
}
void Robot_Left()
{
  digitalWrite(MotorLeft[0],1);
  digitalWrite(MotorLeft[1],0);
  digitalWrite(MotorRight[0],1);
  digitalWrite(MotorRight[1],0);    
}
void Robot_Right()
{
  digitalWrite(MotorLeft[0],0);
  digitalWrite(MotorLeft[1],1);
  digitalWrite(MotorRight[0],0);
  digitalWrite(MotorRight[1],1);    
}

void Robot_Stop()
{
  digitalWrite(MotorLeft[0],0);
  digitalWrite(MotorLeft[1],0);
  digitalWrite(MotorRight[0],0);
  digitalWrite(MotorRight[1],0);    
}


/*
BLYNK_WRITE(V1)
{   
  int value = param.asInt(); // Get value as integer
 // Serial.println("Going Forward");
  if(value)
  {
    Robot_Forward();

  }
}

BLYNK_WRITE(V2)
{   
  int value = param.asInt(); // Get value as integer
  //Serial.println("Moving Right");
  if(value)
  {
    Robot_Right();
    delay(200);
    Robot_Stop();
  }
}


BLYNK_WRITE(V3)
{   
  int value = param.asInt(); // Get value as integer
 // Serial.println("Going back");
  if(value)
  {
    Robot_Backward();

  }
}


BLYNK_WRITE(V4)
{   
  int value = param.asInt(); // Get value as integer
  //Serial.println("Taking Left");
  if(value)
  {
    Robot_Left();
    delay(200);
    Robot_Stop();

  }
}

BLYNK_WRITE(V5)
{   
  int value = param.asInt(); // Get value as integer
 // Serial.println("Braking!!");
  if(value)
  {
    Robot_Stop();
  }
}
*/
