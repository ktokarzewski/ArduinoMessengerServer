#include <Arduino.h>
#include <ArduinoMessenger.h>
#include <UIPEthernet.h>
#include <IRremote.h>

#define SERVER_PORT 5544
#define GET_PIN 6
#define SEND_PIN 4
#define ERROR_PIN 5
#define IRRECV_PIN 7
#define FACTOR 50

EthernetServer server = EthernetServer(SERVER_PORT);
EthernetClient client;

Messenger messenger(client);
message m;
char function[5];

IRrecv irrecv(IRRECV_PIN);
IRsend irsend; //pin 13
decode_results results;

boolean codeReady = false;
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

void printRawCode() {
  for (byte i = 0; i < codeLen - 1; i++) {
    messenger.print(rawCodes[i] / FACTOR);
    messenger.print(" ");
  }
  messenger.print(rawCodes[codeLen - 1] / FACTOR);
}

void sendMessage(char * buttonId) {
  messenger.printPROGMEM(PUT);
  messenger.print("\n");
  messenger.printPROGMEM(request_prefix);
  messenger.print(buttonId);
  messenger.printPROGMEM(value_prefix);
  printRawCode();
  messenger.printPROGMEM(suffix);
}

void ledsOff(){
  digitalWrite(SEND_PIN, LOW);
  digitalWrite(GET_PIN, LOW);
  digitalWrite(ERROR_PIN, LOW);
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


void loop() {

  if (EthernetClient current = server.available()) {
    client = current;
    if (client.connected()) {
      if(messenger.parseMessage(&m)){
        ledsOff();
      } else{
        digitalWrite(ERROR_PIN, HIGH);
      }
    }
  }

  if (compareType(GET)) {
    digitalWrite(GET_PIN, HIGH);
    copyCodeRequest();
    irrecv.enableIRIn();
  } else if (compareType(PUT)) {
    decode(m.resource);
    sendCodeToIR(atoi(m.value));
  }


  if (irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume();
    codeReady = true;
  }
  if (codeReady) {
    if (client.connected()) {
      sendMessage(function);
      codeReady = false;
      digitalWrite(GET_PIN, LOW);
    } else {
      digitalWrite(ERROR_PIN, HIGH);
    }
  }

  messenger.reset(&m);
}
