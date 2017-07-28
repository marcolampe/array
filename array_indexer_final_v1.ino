 /*   Loundry track Information System (LIS)                                                                                                                                                                                            
  *                                                                                                                                                                                                 
  *        Author: Marco Lampe                                                                                                                                                                                          
  *        Date: 2017-07-15                                                                                                                                                                                         
  *                                                                                                                                                                                                
  */

#include <SoftwareSerial.h>
#include <SeeedRFIDLib.h> //Lib: https://github.com/johannrichard/SeeedRFIDLib
#include <SPI.h>
#include <Ethernet.h>


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

//diameter 28.65
SeeedRFIDLib RFID(RFID_RX_PIN, RFID_TX_PIN);
RFIDTag tag;

//if (Network == 1) {
  EthernetClient client;
//}


byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};


long Inventory[6]; // will be obsolete


int pos = 0;
int ptr = 0;
int position = 0;

struct CLOTH_HANGER
 {
     long transponder_id;
     int status;
     int array_pos;
 } ;

CLOTH_HANGER hanger[2];

typedef struct
{
  long transponder_id;
  int status;
  int array_pos;
  bool seen_in_array; 
} FOUND_ID;

 
void setup() {
  Serial.begin(9600);
  Serial.println("Serial Ready");

}


void loop() {
  if (RFID.isIdAvailable()) {
    tag = RFID.readId();

    Serial.println("------------------");
    Serial.print("LOOP: ID:       "); Serial.println(tag.id);
    Serial.println("------------------");

    int found = CheckID(tag.id);
    FOUND_ID * result = CheckID(tag.id);

    if (tag.id > VALID_TAG_ID_RANGE) {
      Serial.println("===================");
      Serial.println("Variable from MAIN LOOP");
      Serial.print("pos: "); Serial.println(pos);
      Serial.print("ptr: "); Serial.println(ptr);
      Serial.print("found: "); Serial.println(found);
      Serial.print("position: "); Serial.println(position);
      Serial.print("ID: "); Serial.println(tag.id);
      Serial.println("===================");
      Serial.println("Variable from MAIN LOOP STRUCT");
      Serial.print("found: "); Serial.println(result->seen_in_array);
      Serial.print("position: "); Serial.println(result->array_pos);
      Serial.print("ID: "); Serial.println(result->transponder_id);
      Serial.println("===================");
      if ((found == 0)) { // not in our list --> we need to add it
        Serial.println("ID is new --> lets call AddID()");
        AddID(tag.id, position);
      } else if (found == 1) {
        Serial.println("ID needs update --> lets call UpdateID()");
        UpdateID(tag.id, position);
      }

      Table();

    }
  }
}

/*
 * CheckID checks if the given transponder ID is known to us
 * ID is transponder ID
 * found returns 0 for not found or 1 for found
 * count will 
 * position will tell us the position of the record we are searching for
 */

int CheckID (long ID){

  FOUND_ID result;
  
//  long transponder_id;
//  int status;
//  int array_pos;
//  bool seen_in_array; 

  bool found;
  int count = 0;
  int location = 0; // array_pos
  
  for (int i = 0; i < 3; i++){
   Serial.print("CHECKID New: Inventory["); Serial.print(i); Serial.print("]: ");
   if (ID == hanger[i].transponder_id){

      Serial.println(": FOUND: Found in CheckID");
      count += 1;
      location = i;
      hanger[i].array_pos = i;
   } else {
      Serial.println(": NOT FOUND: Nothing found in CheckID");
      
   }    
 }
  if (count > 0) { // found more than one entry
    position = location;
    found = 1; //ok, no new entry, but we have to make an update
  } else {
    position = location;
    found = 0; //new transponder; needs to be added
  }
  Serial.print("CHECKID: detected match: "); Serial.print(found); Serial.println(" we are leaving CHECKID now");
  return found;
}


FOUND_ID StructCheckID (long ID){
  int count = 0;
  int location = 0; // array_pos
  FOUND_ID result;

  
//  long transponder_id; --> hanger[i].transponder_id;
//  int status;
//  int array_pos; // location --> result[i].array_pos;
//  bool seen_in_array; //found --> result[i].seen_in_array;


  
  for (int i = 0; i < 3; i++){
   Serial.print("CHECKID New: Inventory["); Serial.print(i); Serial.print("]: ");
   if (ID == hanger[i].transponder_id){
      hanger[i].transponder_id = result.transponder_id;
      Serial.println(": FOUND: Found in CheckID");
      count += 1;
      location = i;
      result.seen_in_array = 1;
      result.array_pos = i;
   } else {
      Serial.println(": NOT FOUND: Nothing found in CheckID");
      result.seen_in_array = 0;
   }    
 }

  return result;
}



void AddID(long ID, int position) {
      
      Serial.println("===================");
      Serial.println("Variable from AddID");
      Serial.print("pos: "); Serial.println(pos);
      Serial.print("ptr: "); Serial.println(ptr);
      Serial.print("position: "); Serial.println(position);
      Serial.print("ID: "); Serial.println(tag.id);
      Serial.println("===================");

      if (IN_OUT == 0) {
        //Dealing with dirt
          hanger[position].transponder_id  = ID;
          hanger[position].status = DIRT_IN;
       } else {
          hanger[position].transponder_id  = ID;
          hanger[position].status = CLEAN_IN;
       }

      Serial.print("Inventory Pos "); Serial.print("["); Serial.print(position); Serial.print("]: "); Serial.print(hanger[position].transponder_id);Serial.print("  "); Serial.println(hanger[position].status);
}


int UpdateID(long ID, int position) { // Update

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
}


void Cleanup(){
 Inventory[pos] = 0;
 Inventory[pos + 1] = 0;
 Serial.print("AFTER CLEANUP: Inventory Pos "); Serial.print("["); Serial.print(pos); Serial.print("]: "); Serial.println(Inventory[pos]);
 Serial.print("AFTER CLEANUP: Inventory Pos "); Serial.print("["); Serial.print(pos + 1); Serial.print("]: "); Serial.println(Inventory[pos + 1]);
  
}


void Table(){
  Serial.println("NEW TABLE: ");
  for (int i = 0; i < 3; i++) {
    Serial.print ("  Inventory at Pos "); Serial.print (i); Serial.print(" : Value: "); Serial.print(hanger[i].transponder_id); Serial.print(" : Value: "); Serial.println(hanger[i].status);
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