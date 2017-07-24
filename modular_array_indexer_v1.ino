#include <SoftwareSerial.h>
#include <SeeedRFIDLib.h>

#define RFID_RX_PIN 6
#define RFID_TX_PIN 7
#define VALID_TAG_ID_RANGE 0 // set for defined transponder id e.g 8100000 to allow all id > 81000000

#define TAG_OF_DEAD 8023069
#define Network 0 //set to 1 for Ethernet

#define IN_OUT 0 // 0 --> Dirt // 1 --> Clean
#define DIRT_IN 1
#define DIRT_OUT 2
#define CLEAN_IN 3
#define CLEAN_OUT 4

SeeedRFIDLib RFID(RFID_RX_PIN, RFID_TX_PIN);
RFIDTag tag;

int pos = 0;
int ptr = 0;
int position = 0;
int count = 0;

int redPin = 4;
int greenPin = 3;
int bluePin = 2;

 struct CLOTH_HANGER
 {
     long transponder_id;
     int status;
 };

CLOTH_HANGER hanger[2];
 
void setup() {
  Serial.begin(9600);
  Serial.println("Serial Ready");
}

void loop() {
  if (RFID.isIdAvailable()) {
    tag = RFID.readId();
    Serial.println("------------------");
    Serial.print("LOOP: ID:       "); Serial.println(tag.id);
    Serial.print("LOOP: ID (raw): "); Serial.println(tag.raw);
    Serial.println("------------------");

    if (tag.id == TAG_OF_DEAD) {
//      EraseID();
//      pos = 0;
    }

    int found = NCheckID(tag.id);

    if (tag.id > VALID_TAG_ID_RANGE) {
      Serial.println("Positions");
      Serial.print("count: "); Serial.println(count);
      Serial.print("position: "); Serial.println(position);
      Serial.print("found: "); Serial.println(found);


      if ((found == 0)) { // not in our list --> we need to add it
        NAddID(tag.id);
      } else if (found == 1) {
        Serial.println("ID needs update --> lets call UpdateID()");
        NUpdateID();
      }
      Ntable();
    }
  }
}

void EraseID() {
  //cleanup the array with a transponder --> only debugging
}

int NCheckID (long ID){
  long comp = ID;
  int found = 4;
  int count = 0;
  position = 0;
  
  for (int i = 0; i < 3; i++){
   Serial.print("CHECKID New: Inventory["); Serial.print(i); Serial.print("]: ");
   if (comp == hanger[i].transponder_id){
      Serial.println(": FOUND: Found in CheckID");
      count += 1;
      position = i;
      found = 1;
      Serial.print("count = ");Serial.println(count);
      Serial.print("position = ");Serial.println(position);
      Serial.print("found = ");Serial.println(found);
   } else {
      Serial.println(": NOT FOUND: Nothing found in CheckID");
      found = 0;
   }    
 }
  Serial.print("CHECKID: found is: "); Serial.println(found); 
  return found;
}

void NAddID(long ID) {
  if ((ptr == 0)) {
    if (pos < 6) {
      if (tag.id == TAG_OF_DEAD) {
        hanger[pos].transponder_id  = 0;
      } else {
        hanger[pos].transponder_id  = tag.id;
      }

      if (IN_OUT == 0) {
        //Dealing with dirt
        if (tag.id == TAG_OF_DEAD) { //reset the array on receive this tag
          hanger[pos].status = 0;
        } else {
          hanger[pos].status = DIRT_IN;
        }
      } else {
        if (tag.id == TAG_OF_DEAD) {
          hanger[pos].status = 0;
        } else {
          hanger[pos].status = CLEAN_IN;
        }
      }
      Serial.print("Inventory Pos "); Serial.print("["); Serial.print(pos); Serial.print("]: "); Serial.print(hanger[pos].transponder_id);Serial.print("  "); Serial.println(hanger[pos].status);
      if (tag.id == TAG_OF_DEAD) {
        pos = 0;
        ptr = 1;
      } else {
        pos = pos + 2;
        ptr = 1;
      }
    } else {
      Serial.println("Array is full");
    }
  }
}

int NUpdateID() { // Update 
  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].transponder_id);
  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].status);
  if (IN_OUT == 0) {
    //Dealing with dirt
    hanger[position].status = DIRT_OUT;
  } else { // Clean stuff
    hanger[position].status = CLEAN_OUT;
  }
  Serial.print("AFTER: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].transponder_id);
  Serial.print("AFTER: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].status);
  
  //if ((Inventory[pos+1] == 2) || (Inventory[pos+1] == 4)){
   // Cleanup();
  //}
  
}

void Cleanup(){
}

void Ntable(){
  Serial.println("NEW TABLE: ");
  for (int i = 0; i < 3; i++) {
    Serial.print ("  Inventory at Pos "); Serial.print (i); Serial.print(" : Value: "); Serial.print(hanger[i].transponder_id); Serial.print(" : Value: "); Serial.println(hanger[i].status);
  }
}