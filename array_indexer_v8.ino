

 /*   Loundry track Information System (LIS)                                                                                                                                                                                            
  *                                                                                                                                                                                                 
  *                                                                                                                                                                                                 
  *                                                                                                                                                                                                 
  *                                                                                                                                                                                                
  */
#include <SoftwareSerial.h>
#include <SeeedRFIDLib.h> //Lib: https://github.com/johannrichard/SeeedRFIDLib
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

//diameter 28.65
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

int redPin = 4;
int greenPin = 3;
int bluePin = 2;

 
void setup() {
  Serial.begin(9600);
  Serial.println("Serial Ready");

if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  Serial.println("Ready");
  // print your local IP address:
   printIPAddress();

   LED.begin();
   LED.show(); // Initialize all pixels to 'off'
}


void loop() {

  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (RFID.isIdAvailable()) {
    tag = RFID.readId();
    LED.setPixelColor(0, 255, 255, 255);LED.show();
    Serial.println("------------------");
    Serial.print("LOOP: ID:       "); Serial.println(tag.id);
    Serial.print("LOOP: ID (raw): "); Serial.println(tag.raw);
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
      Serial.println("LOOP: ");
      Serial.print("  found: ");Serial.println(found);
      Serial.print("  pos: "); Serial.println(pos);
      Serial.print("  ptr: "); Serial.println(ptr);
      Serial.print("  count: "); Serial.println(count);
      Serial.print("  where: "); Serial.println(where);
      Serial.print("  maintenance: "); Serial.println(maintenance);

      if ((found == 0) && (maintenance == 0)) {
        Serial.println("Adding ID");
        maintenance = 0;
        AddID(found, ptr);
      } else if ((found == 1)&& (maintenance == 0)) {
        Serial.println("Updating ID");
        maintenance = 0;
        UpdateID(where);
      }
      maintenance = 0;
      table();

    }
  }
}



int CheckID(long ID) {
  long comp = ID;
  bool found;
  int count = 0;
  int me = 0;
  int maint = 0;
  for (int i = 0; i < 6; i++) {
    Serial.print("CHECKID: Inventory["); Serial.print(i); Serial.print("]: ");

    if (comp == Inventory[i]) {
      Serial.println(": FOUND: Found in CheckID");
      LED.setPixelColor(0, 0, 128, 0);LED.show();
      delay(500);
      LED.setPixelColor(0, 0, 0, 0);LED.show();
      count += 1;
      me = i;
    } else {
      Serial.println(": NOT FOUND: Nothing found in CheckID");
      if ((tag.id == TAG_OF_DEAD) || (tag.id == LIS_DIRT) || (tag.id == LIS_CLEAN)){
      maint = 1;
      found = 2;
    }

    }
  }
  if (count > 0) {
    ptr = 1;
    found = 1; //ok, no new entry, but we have to make an update
  } else {
    ptr = 0;
    found = 0; //new transponder; needs to be updated
  }
  where = me;
  maintenance = maint;
  return found;
}


void AddID(int value, int ptr) {
  if ((!value) && (ptr == 0)) {
    if (pos < 6) {
      if (tag.id == TAG_OF_DEAD) {
        Inventory[pos] = 0;
      } else {
        Inventory[pos] = tag.id;
      }

      if (IN_OUT == 0) {
        //Dealing with dirt
        if (tag.id == TAG_OF_DEAD) {
          Inventory[pos + 1] = 0;
        } else {
          Inventory[pos + 1] = DIRT_IN;
        }
      } else {
        if (tag.id == TAG_OF_DEAD) {
          Inventory[pos + 1] = 0;
        } else {
          Inventory[pos + 1] = CLEAN_IN;
        }
      }

      Serial.print("Inventory Pos "); Serial.print("["); Serial.print(pos); Serial.print("]: "); Serial.println(Inventory[pos]);
      Serial.print("Inventory Pos "); Serial.print("["); Serial.print(pos + 1); Serial.print("]: ");; Serial.println(Inventory[pos + 1]);
      LED.setPixelColor(0, 0, 0, 128);LED.show();
      delay(500);
      LED.setPixelColor(0, 0, 0, 0);LED.show();

      if (tag.id == TAG_OF_DEAD) {
        pos = 0;
        ptr = 1;
      } else {
        pos = pos + 2;
        ptr = 1;
      }
         SendHTTP();

    } else {
      Serial.println("Array is full");
    }
  }
}

int UpdateID(int where) { // Update
  if (pos > 0) {
    pos = pos - 2;
  } else {
    pos = 0;
  }

  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(where); Serial.print("]: "); Serial.println(Inventory[where]);
  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(where + 1); Serial.print("]: "); Serial.println(Inventory[where + 1]);
  if (IN_OUT == 0) {
    //Dealing with dirt
    Inventory[where + 1] = DIRT_OUT;
  } else { // Clean stuff
    Inventory[where + 1] = CLEAN_OUT;
  }
  Serial.print("AFTER: Inventory Pos "); Serial.print("["); Serial.print(where); Serial.print("]: "); Serial.println(Inventory[where]);
  Serial.print("AFTER: Inventory Pos "); Serial.print("["); Serial.print(where + 1); Serial.print("]: "); Serial.println(Inventory[where + 1]);
  
  if ((Inventory[where+1] == 2) || (Inventory[where+1] == 4)){
    Cleanup();
  }
//  pos = 0;
    count = 0;
   // where = 0;
}

void Cleanup(){
 // Sende hier daten wenn status 2 oder 3 ist
 Inventory[where] = 0;
 Inventory[where + 1] = 0;
 Serial.print("AFTER CLEANUP: Inventory Pos "); Serial.print("["); Serial.print(where); Serial.print("]: "); Serial.println(Inventory[where]);
 Serial.print("AFTER CLEANUP: Inventory Pos "); Serial.print("["); Serial.print(where + 1); Serial.print("]: "); Serial.println(Inventory[where + 1]);
  
}

void table () {
  Serial.println("TABLE:");
  for (int i = 0; i < 6; i++) {
    Serial.print ("  Inventory at Pos "); Serial.print (i); Serial.print(" : Value: "); Serial.println(Inventory[i]);
  }
}

void printIPAddress(){
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

void EraseID() {
  //cleanup the array with a transponder --> only debugging
  memset(Inventory, 0, sizeof(Inventory));
  pos = 0;
  ptr = 0;
      LED.setPixelColor(0, 128, 0, 0);LED.show();
      delay(500);
      LED.setPixelColor(0, 0, 0, 0);LED.show();
      delay(500);
      LED.setPixelColor(0, 128, 0, 0);LED.show();
      delay(500);
      LED.setPixelColor(0, 0, 0, 0);LED.show();
}

void SendHTTP(){

char PostData ='{\r\n  \"ID\": \"' + Inventory[pos] + ',\r\n  \"state\": \"' + Inventory [pos+1] + '\"\r\n}\r\n';


if (client.connect("https://p0314-iflmap.hcisbp.eu1.hana.ondemand.com/http/laundryService/start",443)){
Serial.println("connected");
  client.println("POST /doit HTTP/1.1");
  client.println("Host: p0314-iflmap.hcisbp.eu1.hana.ondemand.com"); // or generate from your server variable to not hardwire
  client.println("User-Agent: Arduino/uno");
  client.println("authorization: Basic UzAwMTc4MzcwMjY6V1I4NV02WDY=");
  client.println("content-type: application/json");
  client.print("Content-Length: ");
  client.println(strlen(PostData));// number of bytes in the payload
  client.println();// important need an empty line here 
  client.println(PostData);// the payload
Serial.println (PostData);


 } else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  
  }
} 