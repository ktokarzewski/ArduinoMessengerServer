#include <ArduinoMessenger.h>
#include <UIPEthernet.h>
#include <IRremote.h>
#define GET_PIN 6
#define SEND_PIN 4
#define ERROR_PIN 5
#define IRRECV_PIN 7
#define FACTOR 50
//#define UIPETHRTNET_DEBUG
//==================== Memory Control ==================//
const PROGMEM char prefix[] = "SEND\n{\"resource\":\"";
const PROGMEM char separator[] = "\",\"value\":\"";
const PROGMEM char suffix[] = "\",\"id\":\"prototype\"};";
char mem_buf[25];

EthernetServer server = EthernetServer(5544);
Messenger messenger;
message m;
char function[5];

IRrecv irrecv(IRRECV_PIN);
IRsend irsend; //pin 13
decode_results results;

byte codeLen;
unsigned int rawCodes[RAWBUF];
void storeCode(decode_results *results) {
  codeLen = results->rawlen - 1;
  for (int i = 1; i <= codeLen; i++) {
    if (i % 2) {
      rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
    }
    else {
      rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
    }
  }
}
void sendCodeToIR(int frequency) {
  irsend.sendRaw(rawCodes, codeLen, frequency);
}

bool compareType(const char* type) {
  if (strcmp_P(m.type, type) == 0) {
    return true;
  }
  else {
    return false;
  }
}
void copyCodeRequest() {
  byte size = strlen(m.request);
  strncpy(function, m.request, size);
}


void decode(char * msg) {
  codeLen = 0;
  char * temp;
  char * code = (char*)malloc(strlen(msg) * sizeof(char));
  strcpy(code, msg);

  temp = strtok(code, " ");
  while (temp != NULL) {
    if (!extractSubsequence(temp)) {
      rawCodes[codeLen] = strtol(temp, NULL, 16) * FACTOR;
      codeLen++;
    }
    temp = strtok(NULL, " ");
  }
  free(code);
}

bool extractSubsequence(char * sub) {
  byte xIndex = 0;
  for (byte i = 0; i < strlen(sub); i++) {
    if (sub[i] == 'x') {
      xIndex = i;
    }
  }

  if (xIndex == 0) {
    return false;
  }
  else {
    int quantity = atoi(sub + xIndex + 1);
    unsigned int hex = strtol(sub, &sub + (xIndex - 1), 16);
    for (byte i = 0; i < quantity; i++) {
      rawCodes[codeLen] = hex * FACTOR;
      codeLen++;
    }
    return true;
  }
}


void printRawCode(EthernetClient client) {
  for (byte i = 0; i < codeLen-1; i++) {
    client.print(rawCodes[i] / FACTOR);
    client.print(F(" "));
  }
  client.print(rawCodes[codeLen-1] / FACTOR);
}

void sendMessage(char * buttonId, EthernetClient client) {
  // copy preffix, separator and suffix to buffer using strcpy_P() before print
  strcpy_P(mem_buf, prefix);
  client.print(mem_buf); // sending prefix

  client.print(buttonId);

  strcpy_P(mem_buf, separator);
  client.print(mem_buf); // sending separator

  printRawCode(client);
  //encode();

  strcpy_P(mem_buf, suffix);
  client.print(mem_buf); // sending suffix

  client.flush();
}

void setup()
{
  uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  IPAddress myIP(192, 168, 1, 10);
  Ethernet.begin(mac, myIP);
  server.begin();

  pinMode(SEND_PIN, OUTPUT);
  pinMode(GET_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);

}
boolean codeReady = false;
EthernetClient client;

void loop(){
  size_t size;

  if (EthernetClient current = server.available()){
    client = current;

    if (client){
      Serial.println(client);
      while ((size = client.available()) > 0){
        if (size < M_BUF) {
          uint8_t* msg = (uint8_t*)malloc(size);
          size = client.read(msg, size);
          messenger.parseMessage(&m, msg, size);
          free(msg);
        }
        else {
          client.flush();
        }
      }
    }

  }
  // Ethernet.maintain();

  if (compareType(GET)) {
    digitalWrite(GET_PIN, HIGH);
    copyCodeRequest();
    irrecv.enableIRIn();
  } else if (compareType(SEND)) {
    decode(m.resource);
    digitalWrite(SEND_PIN, HIGH);
    sendCodeToIR(38);
    delay(1000);
    digitalWrite(SEND_PIN, LOW);
  }
  
  
  if (irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume();
    codeReady = true;
  }
  if (codeReady) {
    if (client.connected()) {
      sendMessage(function, client);
      codeReady = false;
      digitalWrite(GET_PIN, LOW);
    } else {
      digitalWrite(ERROR_PIN, HIGH);

    }
  }

  messenger.reset(&m);
}
