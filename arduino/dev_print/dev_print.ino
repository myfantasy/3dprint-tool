#include <SoftwareSerial.h>
#include <SD.h>

String endl_ok_msg_responce = "ENDLROKS623746dy7382d6td6326--";
String endl_error_msg_responce = "ENDLRErrS623746dy7382d6td6326--";
String endl_msg_request = "ENDLRQ623746dy7382d6td6326--";

bool _debug = true;

String settingsFilename = "settings.txt";


String WIFI_SSID = "wifi_ssid";
String WIFI_PWD = "12345678";
String HOST = "192.168.0.202";
String PORT = "8080";
String URL = "/doodle";

#define SERIAL_PORT_SPEED 115200
#define modem Serial3
#define MODEM_PORT_SPEED 115200
#define MODEM_NEXT_WAIT_MS_CLEAR 10
#define MODEM_NEXT_WAIT_MS_REQUEST 2

#define ANALOG_PINS_COUNT 16
#define READ_PINS_COUNT 32
#define WRITE_PINS_COUNT 32
#define STEPPERS_COUNT 10

// #define OK "OK"
// #define ERROR "ERROR"
// #define FAIL "FAIL"

#define RESPONCE_LENGTH 51
#define ID_P 0
#define N_P 1
#define TS_P 2
#define TE_P 3
#define WORK_STEPPERS_CNT 5
#define WRITE_CNT 32
#define WRITE_START RESPONCE_LENGTH-WRITE_CNT
#define WORK_STEPPERS_START 4
long index_i = 0;
long steppers[WORK_STEPPERS_CNT] = {0, 0, 0, 0, 0};

int responce[RESPONCE_LENGTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0};


int analog_pins[ANALOG_PINS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int read_pins[READ_PINS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int write_pins[WRITE_PINS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int dir_pins[STEPPERS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int step_pins[STEPPERS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int read_pins_values[READ_PINS_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int requestTimeoutPin = 0;
int requestSuccessPin = 0;
unsigned long requestTimeoutValue = 5000;
unsigned long connectTimeoutValue = 60000;

bool econ = false;

// pins work

void initPinRead(int number) {
  if (number > 0) {
    pinMode(number, INPUT);
  }
}

void initPinWrite(int number) {
  if (number > 0) {
    pinMode(number, OUTPUT);
  }
}

void setPin(int number, int value) {
  if (number > 0) {
    digitalWrite(number, value);
  }
}
// -------------- SD

// Скетч не выполняется дальше. Быстро моргает
void badSetings() {
  pinMode(LED_BUILTIN, OUTPUT);
  while(true){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(400);                    
    digitalWrite(LED_BUILTIN, LOW);
    delay(400);    
  }
}

bool getValue(String str, String key, String& value) {
  if (str.indexOf(key) == 0) {
    value = str.substring(key.length());
    return true;
  }
  return false;
}

void parseFile(String f) {
  if (_debug) {
    Serial.println("parseFile DATA:");
    Serial.println(f);
  }
  while (f.length() > 0) {
    int len = f.indexOf('\n');
    if (len < 0) {
      len = f.length();
    }
    String str = f.substring(0, len);
    f = f.substring(len + 1); // cut string
    getValue(str, "WIFI_SSID=", WIFI_SSID);
    getValue(str, "WIFI_PWD=", WIFI_PWD);
    getValue(str, "HOST=", HOST);
    getValue(str, "PORT=", PORT);
    getValue(str, "URL=", URL);
    String tmp;
    for (int i = 0; i < ANALOG_PINS_COUNT; i++) {
      if (getValue(str, "ANALOG_" + String(i) + "=", tmp)) {
        analog_pins[i] = tmp.toInt();
      }
    }
    for (int i = 0; i < READ_PINS_COUNT; i++) {
      if (getValue(str, "READ_" + String(i) + "=", tmp)) {
        read_pins[i] = tmp.toInt();
      }
    }
    for (int i = 0; i < WRITE_PINS_COUNT; i++) {
      if (getValue(str, "WRITE_" + String(i) + "=", tmp)) {
        write_pins[i] = tmp.toInt();
      }
    }
    for (int i = 0; i < STEPPERS_COUNT; i++) {
      if (getValue(str, "DIR_" + String(i) + "=", tmp)) {
        dir_pins[i] = tmp.toInt();
      }
    }
    for (int i = 0; i < STEPPERS_COUNT; i++) {
      if (getValue(str, "STEP_" + String(i) + "=", tmp)) {
        step_pins[i] = tmp.toInt();
      }
    }
    if (getValue(str, "X_DIR=", tmp)) {
      dir_pins[0] = tmp.toInt();
    }
    if (getValue(str, "Y_DIR=", tmp)) {
      dir_pins[1] = tmp.toInt();
    }
    if (getValue(str, "Z_DIR=", tmp)) {
      dir_pins[2] = tmp.toInt();
    }
    if (getValue(str, "T_DIR=", tmp)) {
      dir_pins[3] = tmp.toInt();
    }
    if (getValue(str, "F_DIR=", tmp)) {
      dir_pins[4] = tmp.toInt();
    }
    if (getValue(str, "X_STEP=", tmp)) {
      step_pins[0] = tmp.toInt();
    }
    if (getValue(str, "Y_STEP=", tmp)) {
      step_pins[1] = tmp.toInt();
    }
    if (getValue(str, "Z_STEP=", tmp)) {
      step_pins[2] = tmp.toInt();
    }
    if (getValue(str, "T_STEP=", tmp)) {
      step_pins[3] = tmp.toInt();
    }
    if (getValue(str, "F_STEP=", tmp)) {
      step_pins[4] = tmp.toInt();
    }
    if (getValue(str, "URL_DISABLE_PORT=", tmp)) {
      requestTimeoutPin = tmp.toInt();
    }
    if (getValue(str, "URL_ENABLE_PORT=", tmp)) {
      requestSuccessPin = tmp.toInt();
    }
    if (getValue(str, "WAIT_TIMEOUT=", tmp)) {
      requestTimeoutValue = tmp.toInt();
    }
    if (getValue(str, "WAIT_AP_TIMEOUT=", tmp)) {
      connectTimeoutValue = tmp.toInt();
    }
  }
  if (_debug) {
    Serial.println("parseFile RESULT:");
    serialPrintlnKV("WIFI_SSID", WIFI_SSID);
    serialPrintlnKV("WIFI_PWD", WIFI_PWD);
    serialPrintlnKV("HOST", HOST);
    serialPrintlnKV("PORT", PORT);
    serialPrintlnKV("URL", URL);

    for (int i = 0; i < ANALOG_PINS_COUNT; i++) {
      serialPrintlnKV("ANALOG_" + String(i), String(analog_pins[i]));
    }
    
    for (int i = 0; i < READ_PINS_COUNT; i++) {
      serialPrintlnKV("READ_" + String(i), String(read_pins[i]));
    }

    for (int i = 0; i < WRITE_PINS_COUNT; i++) {
      serialPrintlnKV("WRITE_" + String(i), String(write_pins[i]));
    }

    for (int i = 0; i < STEPPERS_COUNT; i++) {
      serialPrintlnKV("STEP_" + String(i), String(step_pins[i]));
      serialPrintlnKV("DIR_" + String(i), String(dir_pins[i]));
    }
  }

  for (int i = 0; i < READ_PINS_COUNT; i++) {
    initPinRead(read_pins[i]);
  }
  for (int i = 0; i < WRITE_PINS_COUNT; i++) {
    initPinWrite(write_pins[i]);
  }
  for (int i = 0; i < STEPPERS_COUNT; i++) {
    initPinWrite(dir_pins[i]);
  }
  for (int i = 0; i < STEPPERS_COUNT; i++) {
    initPinWrite(step_pins[i]);
  }
  initPinWrite(requestTimeoutPin);
  initPinWrite(requestSuccessPin);
}
void serialPrintlnKV(String k, String v) {
  Serial.print(k);
  Serial.print(":");
  Serial.println(v);
}

void readSD() {
  File file = SD.open(settingsFilename, FILE_READ);
  if (file) {
    if (file.available()) {
      String data = file.readString();
      if (_debug) {
        Serial.println(data);
      }
      // parse file
      parseFile(data);
    }
    // close file
    file.close();
  } else {
    Serial.println("SD Card: error on opening file");
    badSetings(); //while(true);                // Скетч не выполняется дальше. Быстро моргает
  }
}

// -------------- WIFI

String clearBuffer() {
  if (_debug) {
    Serial.println("clear modem begin");
  }
  while (modem.available()) {
    modem.readString();
    delay(MODEM_NEXT_WAIT_MS_CLEAR);
  }
  if (_debug) {
    Serial.println("clear modem complete");
  }
}

void mForceSend(String command) {
  if (_debug) {
    Serial.println(">>");
    Serial.println(command);
  }
  modem.println(command);
}

String waitResponce(String stopWord1, String stopWord2, unsigned long addTimeout) { // добавить стоп контент
  unsigned long end = millis() + requestTimeoutValue + addTimeout;
  if (_debug) {
    Serial.println("wait responce modem begin");
  }
  
  String res = "";

  while (millis() < end) {
    if (modem.available()) {
      res = res + modem.readString();
      if (stopWord1 !="" && res.indexOf(stopWord1) >= 0) {
        break;
      }
      if (stopWord2 !="" && res.indexOf(stopWord2) >= 0) {
        break;
      }      
    } else {
      delay(MODEM_NEXT_WAIT_MS_REQUEST);
    }
  }
  if (_debug) {
    Serial.println(res);
    Serial.println("wait responce modem complete");
  }
  return res;
}

// Request and responce post

bool establishWifiConnection() {
  String res = "";

  mForceSend("set settings;host="+HOST+";port="+String(PORT)+";wifi_name="+WIFI_SSID+";wifi_pwd="+WIFI_PWD+";path="+URL+";"+endl_msg_request);
  res = waitResponce(endl_ok_msg_responce, endl_error_msg_responce, connectTimeoutValue);
  if (res.indexOf(endl_ok_msg_responce) == -1) {
    return false;
  }

  mForceSend("ap connect;"+endl_msg_request);
  res = waitResponce(endl_ok_msg_responce, endl_error_msg_responce, connectTimeoutValue);
  if (res.indexOf(endl_ok_msg_responce) == -1) {
    return false;
  }

  mForceSend("info;"+endl_msg_request);
  res = waitResponce(endl_ok_msg_responce, endl_error_msg_responce, connectTimeoutValue);
  if (res.indexOf(endl_ok_msg_responce) == -1) {
    return false;
  }
}

void parseResponse(String rs, int rv[]) {
  int p = rs.indexOf("\n");
  int c = 0;
  while(p > 0 && c<RESPONCE_LENGTH) {
    rv[c] = rs.substring(0, p).toInt();
    c++;
    rs = rs.substring(p + 1);
    p = rs.indexOf("\n");
    if (p == -1) {
      p = rs.length();
    }
  }

  // Обнуляем итераторы
  for (int i = 0; i < WORK_STEPPERS_CNT; i++) {
    steppers[i] = 0;
  }
  index_i = 0;
}

String makeRequestBody() {
  String res = "";
  res += responce[ID_P];
  res += '\n';
  res += String(index_i);
  res += '\n';
  for (int i = 0; i < WORK_STEPPERS_CNT; i++) {
    res += String(steppers[i]);
    res += '\n';
  }
  for (int i = 0; i < ANALOG_PINS_COUNT; i++) {
    if (analog_pins[i] > 0) {
      res += String(analogRead(analog_pins[i]));
    } else {
      res += "0";
    }
    res += '\n';
  }
  for (int i = 0; i < READ_PINS_COUNT; i++) {
    if (read_pins[i] > 0) {
      res += String(digitalRead(read_pins[i]));
    } else {
      res += "0";
    }
    res += '\n';
  }
  return res;
}

bool doPostRequest() {
  String res = "";
  if (_debug) {
    Serial.println("responce requested");
  }

  mForceSend("post\n"+makeRequestBody()+"\n"+endl_msg_request);
  res = waitResponce(endl_ok_msg_responce, endl_error_msg_responce, 0);
  if (res.indexOf(endl_ok_msg_responce) == -1) {
    if (_debug) {
      Serial.println("responce request failed");
    }
    return false;
  }

  parseResponse(res, responce);

  if (_debug) {
    for (int i = 0; i < RESPONCE_LENGTH; i++) {
      Serial.print(responce[i]);
      Serial.print(';');
    }
    Serial.println();
    Serial.println("responce request and parsed");
  }

  return true;
}

// Do Logic
void setByResponce() {
  if (_debug) {
    Serial.println("setByResponce begin");
  }
  
  for (int i = WRITE_START; i < RESPONCE_LENGTH; i++) {
    setPin(write_pins[i-WRITE_START], responce[i]);
  }

  for (int i = WORK_STEPPERS_START; i < WORK_STEPPERS_CNT + WORK_STEPPERS_START; i++) {
    if (responce[i] >= 0) {
      setPin(dir_pins[i-WORK_STEPPERS_START], LOW);
    } else {
      setPin(dir_pins[i-WORK_STEPPERS_START], HIGH);
    }
  }

  for (int i = 0; i < READ_PINS_COUNT; i++) {
    if (read_pins[i] > 0) { 
      read_pins_values[i] = digitalRead(read_pins[i]);
    } else {
      read_pins_values[i] = 0;
    } 
  }

  delayMicroseconds(responce[TS_P]);
  delayMicroseconds(responce[TE_P]);

  if (_debug) {
    Serial.println("setByResponce complete");
  }
}

bool readPinsNotChanges() {
  for (int i = 0; i < READ_PINS_COUNT; i++) {
    if (read_pins[i] > 0) { 
      if (read_pins_values[i] != digitalRead(read_pins[i])) {
        return false;
      }
    }
  }
  return true;
}

void doSteps() {
  long dxi[WORK_STEPPERS_CNT];
  bool tick[WORK_STEPPERS_CNT];
  for (int i = 0; i < WORK_STEPPERS_CNT; i++){
    if (responce[N_P] == 0 || responce[WORK_STEPPERS_START+i] == 0) {
      dxi[i] = 0;
    } else {
      dxi[i] = responce[N_P] / abs(responce[WORK_STEPPERS_START+i] * 2);
    }
  }

  long val = 0;

  for (; index_i < responce[N_P];) {
    index_i++;

    if (_debug && index_i%10==0) {
      Serial.println("doSteps "+String(index_i));
      Serial.print(responce[N_P]);
      Serial.print(";");
      Serial.print("||");
      for (int i = 0; i < WORK_STEPPERS_CNT; i++){
        Serial.print(steppers[i]);
        Serial.print("(");
        Serial.print(dxi[i]);
        Serial.print(";");
        Serial.print(responce[WORK_STEPPERS_START+i]);
        Serial.print(")");
        Serial.print(";");
      }
      Serial.println();      
    }

    if (!readPinsNotChanges()) {
      if (_debug) {
        Serial.println("loop exit by params changes");
      }
      return;    
    }
    for (int i = 0; i < WORK_STEPPERS_CNT; i++){
      val = (index_i + dxi[i])*(responce[WORK_STEPPERS_START+i])/responce[N_P];
      if (val!=steppers[i]) {
        if (val >0 && steppers[i] <0) {
            Serial.print(">>");
            Serial.print(i);
            Serial.print(";");
            Serial.print(val);
            Serial.print(";");
            Serial.print(steppers[i]);
            Serial.print("|");
            Serial.print((index_i + dxi[i])*(responce[WORK_STEPPERS_START+i])/responce[N_P]);
            Serial.print("|");
            Serial.print((index_i + dxi[i])*(responce[WORK_STEPPERS_START+i]));
            Serial.print(";");
            Serial.print(responce[N_P]);
            Serial.print(";");
            Serial.println(); 
        }
        steppers[i] = val;
        tick[i] = true;
      } else {
        tick[i] = false;
      }
    }
    
    for (int i = 0; i < WORK_STEPPERS_CNT; i++){
      if (tick[i]) {
        setPin(step_pins[i], HIGH);
      }
    }
    delayMicroseconds(responce[TS_P]);
    
    for (int i = 0; i < WORK_STEPPERS_CNT; i++){
      if (tick[i]) {
        setPin(step_pins[i], LOW);
      }
    }
    delayMicroseconds(responce[TE_P]);
  }
}

// // https://radioprog.ru/post/397  #AT commands
// // https://radioprog.ru/post/401  #AT WIFI commands
// // https://radioprog.ru/post/407  #AT TCP/IP commands
// bool establishWifiConnection() {
//   String res = ""

//   mForceSend("ATE0");
//   res = waitResponce(OK, ERROR, 0)
//   if res.indexOf(OK) == -1 {
//     return false;
//   }

//   mForceSend("AT+CWMODE_CUR=1");
//   res = waitResponce(OK, ERROR, 0)
//   if res.indexOf(OK) == -1 {
//     return false;
//   }

//   mForceSend("AT+CWJAP_CUR?");
//   res = waitResponce(OK, FAIL, 0)
//   if res.indexOf(OK) == -1 {
//     return false;
//   }

//   mForceSend("AT+CWJAP_CUR=\"" + WIFI_SSID + "\",\"" + WIFI_PWD + "\"");
//   res = waitResponce(OK, FAIL, connectTimeoutValue)
//   if res.indexOf(OK) == -1 {
//     return false;
//   }

//   mForceSend("AT+CIPMUX=1");
//   res = waitResponce(OK, ERROR, 0)
//   if res.indexOf(OK) == -1 {
//     return false;
//   }

//   return true;
// }

void setup() {
  Serial.begin(SERIAL_PORT_SPEED);
  modem.begin(MODEM_PORT_SPEED);

  if (!SD.begin()) {                            // Проверяем, есть ли связь с картой и, если нет, то
    Serial.println("Initialization failed");    // выводим текст в последовательный порт.
    badSetings(); //while(true);                // Скетч не выполняется дальше. Быстро моргает
  }
  // put your setup code here, to run once:
  readSD();

  econ = establishWifiConnection();
}

void loop() {
  if (!econ) {
    econ = establishWifiConnection();
  }
  
  if (_debug) {
    Serial.println("start loop");
  }

  // put your main code here, to run repeatedly:
  if (!doPostRequest()) {
    econ = false;
    setPin(requestTimeoutPin, HIGH);
    setPin(requestSuccessPin, LOW);
    if (_debug) {
      Serial.println("request fail try reconect");
    }
    if (establishWifiConnection()) {
      econ = true;
      if (_debug) {
        Serial.println("reconect OK");
      }
    } else {
      if (_debug) {
        Serial.println("reconect fail");
      }
    }
    return;    
  }

  setPin(requestTimeoutPin, LOW);
  setPin(requestSuccessPin, HIGH);

  setByResponce();
  doSteps();

}
