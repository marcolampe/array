#include <SoftwareSerial.h>
#include <SeeedRFIDLib.h> 
#include <SPI.h>
#include <Ethernet.h>
#include <Adafruit_NeoPixel.h>

#define RFID_RX_PIN 6
#define RFID_TX_PIN 7
#define VALID_TAG_ID_RANGE 0 // set for defined transponder id e.g 8100000 to allow all id > 81000000

#define TAG_OF_DEAD 1852566
#define LIS_DIRT 8092111
#define LIS_CLEAN 8023069

#define DIRT_IN 1
#define DIRT_OUT 2
#define CLEAN_IN 3
#define CLEAN_OUT 4


SeeedRFIDLib RFID(RFID_RX_PIN, RFID_TX_PIN);
RFIDTag tag;

Adafruit_NeoPixel LED = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);

EthernetClient client;

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

int IN_OUT = 0; // 0 --> Dirt // 1 --> Clean
long Inventory[6];
int pos = 0;
int ptr = 0;
int count = 0;
int where = 0;
int maintenance = 0;
long ADD_ID = 0;
int ADD_STATUS = 0;

 
void setup() {
  LED.begin();
  LED.show(); // Initialize all pixels to 'off'
  
  Serial.begin(9600);
  LED.setPixelColor(0, 255, 0, 0);LED.show();
  Serial.println("Serial Ready");

if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    LED.setPixelColor(0, 0, 0, 0);LED.show();delay(1000);
    LED.setPixelColor(0, 255, 0, 0);LED.show();delay(1000);
    LED.setPixelColor(0, 0, 0, 0);LED.show();delay(1000);
    LED.setPixelColor(0, 255, 0, 0);LED.show();delay(1000);
    for (;;)
      ;
  }
  delay(1000);
  Serial.println("Ethernet Ready");
  Serial.println(Ethernet.localIP());

 LED.setPixelColor(0, 0, 0, 0);LED.show();
}


void loop() {

   
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }  

  if (RFID.isIdAvailable()) {
    tag = RFID.readId();
    LED.setPixelColor(0, 255, 255, 255);LED.show();
    Serial.println("");
    Serial.println("------------------");
    Serial.print("LOOP: ID:       "); Serial.println(tag.id);
    Serial.println("------------------");
    LED.setPixelColor(0, 0, 0, 0);LED.show();

    if (tag.id == TAG_OF_DEAD) {
      EraseID();
      pos = 0;
    }
    if (tag.id == LIS_DIRT) {
      Serial.println("- Acting as DIRT DEVICE -");
      IN_OUT = 0;
      LED.setPixelColor(0, 255, 255, 0);LED.show();
      delay(1500);
      LED.setPixelColor(0, 0, 0, 0);LED.show();
    }
    if (tag.id == LIS_CLEAN) {
      Serial.println("- Acting as CLEAN DEVICE -");
      IN_OUT = 1;
      LED.setPixelColor(0, 255, 20, 147);LED.show();
      delay(1500);
      LED.setPixelColor(0, 0, 0, 0);LED.show();
    }

    int found = CheckID(tag.id);

    if (tag.id > VALID_TAG_ID_RANGE) {

      if ((found == 0) && (maintenance == 0)) {
        Serial.println("Adding ID");
        maintenance = 0;
        AddID(found, ptr);
        //SendHTTP(Inventory[pos], Inventory[pos+1]); 
      } else if ((found == 1)&& (maintenance == 0)) {
        Serial.println("Updating ID");
        maintenance = 0;
        UpdateID(where);
      }
      maintenance = 0;
      //table();

    }
  }
}
