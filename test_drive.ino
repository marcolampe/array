 /*   Loundry track Information System (LIS)                                                                                                                                                                                            
  *                                                                                                                                                                                                 
  *        Author: Marco Lampe                                                                                                                                                                                          
  *        Date: 2017-07-17                                                                                                                                                                                         
  *                                                                                                                                                                                                
  */

#include <SoftwareSerial.h>
#include <SeeedRFIDLib.h>



#define RFID_RX_PIN 6
#define RFID_TX_PIN 7

#define VALID_TAG_ID_RANGE 0 // set for defined transponder id e.g 8100000 to allow all id > 81000000
#define TAG_OF_DEAD 8023069 // reset all
#define CLEAN_TAG 8023070 // defines transponder for setting LIS into CLEAN_Mode
#define DIRT_TAG 8023071 // defines transponder for setting LINs into DIRT_Mode

#define Network 0 //set to 1 for Ethernet

#define DEVICE_MODE 0 // 0 --> Dirt // 1 --> Clean
#define DIRT_IN 1
#define DIRT_OUT 2
#define CLEAN_IN 3
#define CLEAN_OUT 4

#define MAX_ELEMENTS 3

//diameter 28.65
SeeedRFIDLib RFID(RFID_RX_PIN, RFID_TX_PIN);
RFIDTag tag;

long Inventory[6];
int pos = 0; // position an dem die aktuelle ID eingefügt wird
int ptr = 0;
int found = 0; // flag für eine gefundene ID in der struktur 
int position = 0; // position des gefundenen datesatzes innerhalb der schleife
int update = 0; // flag für update des datensatzes
int update_state = 0; // flag für den state (1,2,3 o. 4) des updates

int redPin = 4;
int greenPin = 3;
int bluePin = 2;

 struct CLOTH_HANGER
 {
     long transponder_id;
     int status;
 } ;

CLOTH_HANGER hanger[2];
 
void setup() {
  Serial.begin(9600);
  Serial.println("Serial Ready");

   pinMode(redPin, OUTPUT);
   pinMode(greenPin, OUTPUT);
   pinMode(bluePin, OUTPUT);

   digitalWrite(redPin, HIGH);
   digitalWrite(greenPin, HIGH);
   digitalWrite(bluePin, HIGH);

  LoundrySetup();
  DisplayTable();
}


void loop() {
  if (RFID.isIdAvailable()) {
    tag = RFID.readId();

    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);

    Serial.println("------------------");
    Serial.print("LOOP: ID:       "); Serial.println(tag.id);
    Serial.print("LOOP: ID (raw): "); Serial.println(tag.raw);
    Serial.println("------------------");

    digitalWrite(greenPin, LOW);
    delay(500);
    digitalWrite(greenPin, HIGH);

    int found = CheckID(tag.id); //found sollte 1 sein, wenn wir was gefunden haben

    if (tag.id > VALID_TAG_ID_RANGE) {
       
        StatusPrint();

      if ((found == 0)) { // not in our list --> we need to add it
        AddID(tag.id);
      } else if (found == 1) { // tag.id is in our list so we need to update the status
        UpdateID();
      }
      DisplayTable();
    }
  }
}

int CheckID (long ID){
  long comp = ID;
  int found = 0;
  int count = 0;
  int update = 1;
  
  for (int i = 0; i < MAX_ELEMENTS; i++){
   Serial.print("CHECKID New: Inventory["); Serial.print(i); Serial.print("]: ");
   if (comp == hanger[i].transponder_id){
    Serial.println(": FOUND: Found in CheckID");
    count += 1;
    position = i;
    if (DEVICE_MODE == 0) { //DIRT_IN
     update = 1;
     update_state = DIRT_IN;
    } else { // CLEAN_IN
     update = 1;
     update_state = CLEAN_IN;    
    }
   } else {
    Serial.println(": NOT FOUND: Nothing found in CheckID");
    if (comp == TAG_OF_DEAD){
     Serial.println("TAG OF DEAD found");
     count = 0;
    }
   }
       
   if (count > 0) { // found more than one entry
    ptr = 1;
    update = 1;
    found = 1; //ok, no new entry, but we have to make an update
   } else {
    ptr = 0;
    found = 0; //new transponder; needs to be updated
   }
 }
}
int AddID(long ID) {
  // ID nicht gefunden also müssen wir die ID hinzufügen
  // Wir haben 3 Plätze und wir müssen wissen ob noch was frei ist
  // Lösung: wir fangen bei 0 an und zählen nach dem hinzufügen um eins hoch
  // aber die speziellen Transponder dürfen wir nicht mit einfügen (TAG_OF_DEAD, DIRT_TAG, CLEAN_TAG)
  // found sollte 0 sein
  // pos sollte 0 sein (zum ersten aufruf sollte das 0 sein)
  // update ist ebenfalls 0
  // update_status ist auch 0, da wir was neues einfügen
  // wir müssen noch wissen in welchem DEVICE_MODE (0 für Dirt und 1 für Clean) wir uns befinden, denn
  // abhängig davon ist der initial status DIRT_IN oder CLEAN_IN
  if ((ptr == 0)) {
 
      if ((DEVICE_MODE == 0) && (!TAG_OF_DEAD)) {
          hanger[pos].transponder_id  = tag.id;
          hanger[pos].status = DIRT_IN;
      } else {
          hanger[pos].transponder_id  = tag.id;
          hanger[pos].status = CLEAN_IN;
        }
   }

      Serial.print("Inventory Pos "); Serial.print("["); Serial.print(pos); Serial.print("]: "); Serial.print(hanger[pos].transponder_id);Serial.print("  "); Serial.println(hanger[pos].status);
      digitalWrite(bluePin, LOW);
      delay(500);
      digitalWrite(bluePin, HIGH);


        pos = pos +1;
        ptr = 1;
 }

 int NUpdateID() { // Update

  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].transponder_id);
  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].status);
  if (DEVICE_MODE == 0) {
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
void UpdateID(){
  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].transponder_id);
  Serial.print("BEFORE: Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.println(hanger[position].status);
  if (DEVICE_MODE == 0) {
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
void EraseID() {
  //cleanup the array with a transponder --> only debugging
         for (int i=0; i< MAX_ELEMENTS; i++){  
           hanger[i].transponder_id  = 0;
           hanger[i].status = 0;
         }
  pos = 0;
  ptr = 0;
  digitalWrite(redPin, LOW);
  delay(125);
  digitalWrite(redPin, HIGH);
  delay(125);
  digitalWrite(redPin, LOW);
  delay(125);
  digitalWrite(redPin, HIGH);
  delay(125);
}
void Cleanup(){
         for (int i=0; i < MAX_ELEMENTS; i++){  
           hanger[pos].transponder_id  = 0;
           hanger[pos].status = 0;
         } 
}
void DisplayTable(){
  Serial.println("NEW TABLE: ");
  for (int i = 0; i < MAX_ELEMENTS; i++) {
    Serial.print ("  Inventory at Pos "); Serial.print (i); Serial.print(" : Value: "); Serial.print(hanger[i].transponder_id); Serial.print(" : Value: "); Serial.println(hanger[i].status);
  }
}
void LoundrySetup(){
  for (int i = 0; i < MAX_ELEMENTS; i++){
    hanger[i].transponder_id  = 0;
    hanger[i].status = 0;
  }
}
void StatusPrint() {
      Serial.println ("------------------------------");
      Serial.println("Positions");
      Serial.print("pos: "); Serial.println(pos);
      Serial.print("ptr: "); Serial.println(ptr);
      Serial.print("found: "); Serial.println(found);
      Serial.print("update: "); Serial.println(update);
      Serial.print("update_state: "); Serial.println(update_state);
      Serial.println ("------------------------------");
}


