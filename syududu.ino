/*
  =============================================================================
  AtmoBadge V4.4 - FLAPPY EDITION
  Fix & Features:
  - Layar bebas kedip (Smart Redraw Engine)
  - 3x Klik: System Check Mode (True Hardware Ping)
  - 4x Klik: Hidden Mini Game (Flappy Badge)
  - Booting: Full Super Mario Bros Theme
  =============================================================================
*/

#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <OneButton.h>
#include <time.h>
#include <math.h>

// ===================== PIN =====================
#define MQ135_PIN      0
#define I2C_SDA_PIN    8
#define I2C_SCL_PIN    9
#define TFT_DC         2
#define TFT_CS         7
#define TFT_RST        3
#define BUZZER_PIN     10
#define BUTTON_PIN     1

// ===================== WARNA =====================
#define COLOR_BLACK    0x0000
#define COLOR_WHITE    0xFFFF
#define COLOR_RED      0xF800
#define COLOR_GREEN    0x07E0
#define COLOR_CYAN     0x07FF
#define COLOR_YELLOW   0xFFE0
#define COLOR_ORANGE   0xFD20
#define COLOR_BLUE     0x001F
#define COLOR_PURPLE   0x780F
#define COLOR_LIME     0x3FE0
#define COLOR_TEAL     0x0410
#define COLOR_PINK     0xF81F
#define COLOR_DGRAY    0x2104
#define COLOR_LGRAY    0xC618

// ===================== CREDENTIAL =====================
const char* ssid           = "ahay";
const char* password       = "ozza2026";
const char* telegramToken  = "8721794179:AAE7Q8tcVWbLpxxlj10HVSbEPvZL7FsehK8";
const char* telegramChatId = "1360961353";

// ===== OPENWEATHERMAP =====
const char* OWM_API_KEY = "cb8a1fecc2afa5a8f470839f9fb5f296";
const float OWM_LAT     = -7.9797;   
const float OWM_LON     = 112.6304;
#define OWM_INTERVAL_MS  600000      

// ===================== KONSTANTA =====================
#define WAKTU_PEMANASAN  60000  
#define INTERVAL_BACA    5000
#define AMBANG_WASPADA   1000   
#define AMBANG_BAHAYA    1500   
#define BUFFER_SIZE      10
#define CENTER_X         120
#define CENTER_Y         120
#define RADIUS           120
#define AUTO_SLEEP_MS    25000

// ===================== NADA MELODI =====================
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_C6  1047

// Melodi Super Mario Bros (FULL LENGTH)
const int marioMelody[] = {
  NOTE_E5, NOTE_E5, 0, NOTE_E5, 0, NOTE_C5, NOTE_E5, 0,
  NOTE_G5, 0, NOTE_G4, 0,
  
  NOTE_C5, 0, NOTE_G4, 0, NOTE_E4, 0,
  NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
  0, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_B4, 0,

  NOTE_C5, 0, NOTE_G4, 0, NOTE_E4, 0,
  NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
  0, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_B4, 0,

  // Bridge
  0, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_E5,
  0, NOTE_GS4, NOTE_A4, NOTE_C5, 0, NOTE_A4, NOTE_C5, NOTE_D5,
  0, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_E5,
  0, NOTE_C6, NOTE_C6, NOTE_C6, 0,

  0, NOTE_G5, NOTE_FS5, NOTE_F5, NOTE_DS5, NOTE_E5,
  0, NOTE_GS4, NOTE_A4, NOTE_C5, 0, NOTE_A4, NOTE_C5, NOTE_D5,
  0, NOTE_DS5, 0, NOTE_D5,
  0, NOTE_C5, 0
};

const int marioDurations[] = {
  8, 8, 8, 8, 8, 8, 8, 8,
  4, 4, 4, 4,
  
  4, 8, 4, 8, 4, 8,
  4, 4, 8, 4,
  6, 6, 6, 4, 8, 8,
  8, 4, 8, 8, 4, 4,

  4, 8, 4, 8, 4, 8,
  4, 4, 8, 4,
  6, 6, 6, 4, 8, 8,
  8, 4, 8, 8, 4, 4,

  // Bridge
  8, 8, 8, 8, 4, 4,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 4, 4,
  8, 8, 4, 4, 4,

  8, 8, 8, 8, 4, 4,
  8, 8, 8, 8, 8, 8, 8, 8,
  4, 4, 4, 4,
  4, 4, 2
};

// ===================== OBJEK =====================
Adafruit_GC9A01A display(TFT_CS, TFT_DC, TFT_RST);
Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
OneButton btn = OneButton(BUTTON_PIN, true, true);

// ===================== STATE =====================
enum State { BOOTING, IDLE_SENSING, ALERT_MODE, CHECK_MODE, GAME_MODE };
volatile State currentState = BOOTING;

enum FaceType { FACE_SMILE, FACE_HOT, FACE_MASK, FACE_NONE };
volatile FaceType currentFace = FACE_NONE;

// ===================== DATA GLOBAL =====================
float ppmBuffer[BUFFER_SIZE]   = {0};
unsigned long timeBuffer[BUFFER_SIZE] = {0};
int  bufferIndex = 0;
bool bufferFull  = false;

volatile float currentPPM    = 0;
volatile float currentTemp   = 0;
volatile float currentHum    = 0;
volatile float currentPress  = 0;
volatile float predictedPPM  = 0;

String cuacaStatus     = "Menunggu";
String infoCuacaDetail = "Sync NTP...";

volatile bool showDataPage  = false;
volatile bool isIdleMode    = false;
volatile bool forceRedraw   = true;
volatile bool wifiConnected = false;
volatile bool ntpSynced     = false;

volatile unsigned long lastActivityTime = 0;

QueueHandle_t telegramQueue;

// ===================== GAME DATA (FLAPPY BADGE) =====================
float birdY = 120;
float birdV = 0;
int pipeX = 240;
int pipeGapY = 120;
int gameScore = 0;
bool gameOver = false;
volatile bool gameJump = false;

// ===================== SMART TEXT RENDERER =====================
String lastPrintedText[240];

void clearTextBuffer() {
  for(int i=0; i<240; i++) lastPrintedText[i] = "";
}

int safeHW(int y, int margin = 10) {
  float dy = abs((float)(y + 4 - CENTER_Y));
  if (dy >= RADIUS) return 0;
  int hw = (int)sqrtf((float)(RADIUS * RADIUS) - dy * dy);
  return max(0, hw - margin);
}

void printCenter(int y, const char* txt, uint16_t col, uint16_t bg, int sz) {
  if (y < 0 || y >= 240) return;
  if (lastPrintedText[y] == txt && !forceRedraw) return;

  int cw = 6 * sz;
  int hw = safeHW(y);
  int maxC = (hw * 2) / cw;
  int newLen = strlen(txt);
  
  char buf[40];
  int use = min(newLen, maxC);
  strncpy(buf, txt, use);
  buf[use] = '\0';
  int newPrintLen = strlen(buf);

  int oldLen = lastPrintedText[y].length();
  if (oldLen > newPrintLen && !forceRedraw) {
    int oldW = oldLen * cw;
    int cxOld = CENTER_X - oldW / 2;
    display.fillRect(cxOld, y, oldW, 8 * sz, bg);
  }

  lastPrintedText[y] = txt;
  int cx = CENTER_X - (newPrintLen * cw) / 2;
  
  display.setTextColor(col, bg);
  display.setTextSize(sz);
  display.setCursor(cx, y);
  display.print(buf);
}

uint16_t ppmColor(float ppm) {
  if (ppm < 400)               return COLOR_GREEN;
  if (ppm < AMBANG_WASPADA)    return COLOR_YELLOW;
  if (ppm < AMBANG_BAHAYA)     return COLOR_ORANGE;
  return COLOR_RED;
}

const char* ppmLevel(float ppm) {
  if (ppm < 400)            return "Bersih";
  if (ppm < AMBANG_WASPADA) return "Sedang";
  if (ppm < AMBANG_BAHAYA)  return "Waspada";
  return "BAHAYA!";
}

// ===================== FORWARD DECL =====================
void TaskSensor(void *p);
void TaskDisplay(void *p);
void TaskNetwork(void *p);
void TaskMario(void *p);
void drawSmileFace(int frame, bool fullRedraw);
void drawHotFace(int frame, bool fullRedraw);
void drawMaskFace(int frame, bool fullRedraw);
void drawHeatRing(float ppm);
void drawDataPageFull();
void drawDataPageUpdate(bool doFullDraw);
void drawAOD(bool fullRedraw);
void drawAnalogClock(int cx, int cy, int r, bool fullRedraw);
void aksiKlik1();
void aksiKlik2();
void aksiMultiKlik();
void aksiTahan();

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== AtmoBadge V4.4 Flappy ===");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  btn.attachClick(aksiKlik1);
  btn.attachDoubleClick(aksiKlik2);
  btn.attachMultiClick(aksiMultiKlik);
  btn.attachLongPressStart(aksiTahan);

  display.begin();
  display.setRotation(0);
  display.fillScreen(COLOR_BLACK);

  display.setTextColor(COLOR_CYAN);
  display.setTextSize(2);
  display.setCursor(CENTER_X - (9 * 12) / 2, 90);
  display.print("AtmoBadge");
  display.setTextColor(COLOR_LGRAY);
  display.setTextSize(1);
  display.setCursor(CENTER_X - (8*6)/2, 112);
  display.print("Starting.");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  bool aht_ok = aht.begin(&Wire, 0, 0x38);
  bool bmp_ok = bmp.begin(0x76, BMP280_CHIPID);
  if (!bmp_ok) bmp_ok = bmp.begin(0x77, BMP280_CHIPID);

  if (!aht_ok || !bmp_ok) {
    display.fillScreen(COLOR_BLACK);
    display.setTextColor(COLOR_RED);
    display.setTextSize(2);
    display.setCursor(CENTER_X - (10*12)/2, 90);
    display.print("SENSOR ERR");
    display.setTextSize(1);
    display.setTextColor(COLOR_WHITE);
    if (!aht_ok) { display.setCursor(CENTER_X-(13*6)/2, 115); display.print("AHT20 missing"); }
    if (!bmp_ok) { display.setCursor(CENTER_X-(14*6)/2, 130); display.print("BMP280 missing"); }
    while (1) delay(100);
  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);   
  WiFi.persistent(false);
  WiFi.begin(ssid, password);

  telegramQueue = xQueueCreate(5, sizeof(String *));
  lastActivityTime = millis();

  xTaskCreate(TaskSensor,  "Sensor",  4096, NULL, 1, NULL);
  xTaskCreate(TaskDisplay, "Display", 6144, NULL, 2, NULL);
  xTaskCreate(TaskNetwork, "Network", 8192, NULL, 1, NULL);
  xTaskCreate(TaskMario,   "Mario",   2048, NULL, 1, NULL); 
}

void loop() {
  btn.tick();
  if (!isIdleMode
      && (millis() - lastActivityTime > AUTO_SLEEP_MS)
      && currentState != ALERT_MODE
      && currentState != BOOTING
      && currentState != GAME_MODE) {
    isIdleMode  = true;
    forceRedraw = true;
  }
  vTaskDelay(pdMS_TO_TICKS(10));
}

// ============================================================
// TASK 1: SENSOR
// ============================================================
void TaskSensor(void *p) {
  unsigned long bootTime = millis();
  bool alertSent = false;
  static float smoothed = -1.0;

  for (;;) {
    if (currentState == BOOTING) {
      if (millis() - bootTime >= WAKTU_PEMANASAN) {
        currentState = IDLE_SENSING;
        forceRedraw  = true;
      }
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    sensors_event_t hEvt, tEvt;
    aht.getEvent(&hEvt, &tEvt);
    currentTemp  = tEvt.temperature;
    currentHum   = hEvt.relative_humidity;
    currentPress = bmp.readPressure() / 100.0F;

    long sumADC = 0;
    for (int i = 0; i < 30; i++) {
      sumADC += analogRead(MQ135_PIN);
      vTaskDelay(pdMS_TO_TICKS(2));
    }
    float avgADC = sumADC / 30.0f;

    if (smoothed < 0) smoothed = avgADC;
    else smoothed = 0.1f * avgADC + 0.9f * smoothed;  

    float voltage = (smoothed / 4095.0f) * 3.3f;
    if (voltage < 0.01f) voltage = 0.01f;

    float RS_sensor = 10000.0f * (3.3f - voltage) / voltage;
    if (RS_sensor <= 0) RS_sensor = 1;

    const float RO_CLEAN_AIR = 76000.0f;  
    float ratio = RS_sensor / RO_CLEAN_AIR;

    float T = currentTemp;
    float H = currentHum;
    T = constrain(T, 10.0f, 45.0f);
    H = constrain(H, 20.0f, 90.0f);
    float correction = (-0.00035f*(T*T)) + (0.0202f*T) + (-0.000833f*(H*H)) + (-0.02718f*H) + 1.39538f;
    if (correction < 0.1f) correction = 0.1f;  

    float ratioCorrected = ratio / correction;

    float ppmRaw = 0;
    if (ratioCorrected > 0) {
      ppmRaw = 116.6020682f * powf(ratioCorrected, -2.769034857f);
    }
    currentPPM = constrain(ppmRaw, 0, 5000);

    timeBuffer[bufferIndex] = millis() / 1000;
    ppmBuffer[bufferIndex]  = currentPPM;
    bufferIndex++;
    if (bufferIndex >= BUFFER_SIZE) { bufferIndex = 0; bufferFull = true; }

    if (bufferFull) {
      float sX=0,sY=0,sXY=0,sX2=0;
      unsigned long t0 = timeBuffer[bufferIndex];
      for (int i=0;i<BUFFER_SIZE;i++){
        float x=(float)(timeBuffer[i]-t0), y=ppmBuffer[i];
        sX+=x; sY+=y; sXY+=x*y; sX2+=x*x;
      }
      float d = BUFFER_SIZE*sX2 - sX*sX;
      if (fabsf(d) > 0.001f) {
        float m = (BUFFER_SIZE*sXY - sX*sY) / d;
        float c = (sY - m*sX) / BUFFER_SIZE;
        float xFuture = (millis()/1000.0f) + 300.0f - t0;
        float rawPred = m * xFuture + c;
        float maxPred = currentPPM * 1.30f;
        float minPred = currentPPM * 0.70f;
        predictedPPM = constrain(rawPred, minPred, maxPred);
        predictedPPM = constrain(predictedPPM, 0, 5000);
      }
    }

    // Jangan kirim alarm bahaya kalau lagi asyik main game
    bool danger = (currentPPM >= AMBANG_BAHAYA) || (bufferFull && predictedPPM >= AMBANG_BAHAYA);

    if (danger) {
      if (currentState != ALERT_MODE && currentState != GAME_MODE) forceRedraw = true;
      if (currentState != GAME_MODE) {
        currentState = ALERT_MODE;
        isIdleMode = false;
        lastActivityTime = millis();
        for (int i=0;i<3;i++){
          digitalWrite(BUZZER_PIN, HIGH); vTaskDelay(pdMS_TO_TICKS(300));
          digitalWrite(BUZZER_PIN, LOW);  vTaskDelay(pdMS_TO_TICKS(150));
        }
      }
      if (!alertSent){
        String *msg = new String("🚨 *BAHAYA POLUSI!*");
        xQueueSend(telegramQueue, &msg, 0);
        alertSent = true;
      }
    } else {
      if (currentState == ALERT_MODE) {
        forceRedraw = true;
        currentState = IDLE_SENSING;
      }
      alertSent = false;
    }

    vTaskDelay(pdMS_TO_TICKS(INTERVAL_BACA));
  }
}

// ============================================================
// GAMBAR WAJAH (Smart Render)
// ============================================================
void drawSmileFace(int frame, bool fullRedraw) {
  if (fullRedraw) {
    display.fillCircle(CENTER_X-46, CENTER_Y+8, 7, 0xF810);
    display.fillCircle(CENTER_X+46, CENTER_Y+8, 7, 0xF810);
    for (int d=20; d<=160; d+=5){
      float r=d*PI/180.0f;
      display.fillCircle(CENTER_X+(int)(38*cosf(r)), CENTER_Y+22+(int)(20*sinf(r)), 3, COLOR_WHITE);
    }
    display.fillRect(CENTER_X-14, CENTER_Y+26, 28, 10, COLOR_WHITE);
    display.drawLine(CENTER_X, CENTER_Y+26, CENTER_X, CENTER_Y+36, COLOR_LGRAY);
  }

  if (frame == 0) {
    display.fillRect(CENTER_X-38, CENTER_Y-22, 18, 5, COLOR_BLACK);
    display.fillRect(CENTER_X+20, CENTER_Y-22, 18, 5, COLOR_BLACK);
    display.fillCircle(CENTER_X-28, CENTER_Y-20, 9, COLOR_WHITE);
    display.fillCircle(CENTER_X+28, CENTER_Y-20, 9, COLOR_WHITE);
    display.fillCircle(CENTER_X-26, CENTER_Y-18, 4, COLOR_BLACK);
    display.fillCircle(CENTER_X+30, CENTER_Y-18, 4, COLOR_BLACK);
    display.fillCircle(CENTER_X-24, CENTER_Y-22, 2, COLOR_WHITE);
    display.fillCircle(CENTER_X+32, CENTER_Y-22, 2, COLOR_WHITE);
  } else {
    display.fillCircle(CENTER_X-28, CENTER_Y-20, 9, COLOR_BLACK);
    display.fillCircle(CENTER_X+28, CENTER_Y-20, 9, COLOR_BLACK);
    display.fillRect(CENTER_X-38, CENTER_Y-22, 18, 5, COLOR_WHITE);
    display.fillRect(CENTER_X+20, CENTER_Y-22, 18, 5, COLOR_WHITE);
  }
}

void drawHotFace(int frame, bool fullRedraw) {
  if (fullRedraw) {
    uint16_t ec = COLOR_RED;
    int ax=CENTER_X-40, bx=CENTER_X-22;
    display.drawLine(ax,CENTER_Y-30,bx,CENTER_Y-14,ec);
    display.drawLine(ax,CENTER_Y-14,bx,CENTER_Y-30,ec);
    display.drawLine(ax+1,CENTER_Y-30,bx+1,CENTER_Y-14,ec);
    display.drawLine(ax+1,CENTER_Y-14,bx+1,CENTER_Y-30,ec);
    int cx2=CENTER_X+22, dx=CENTER_X+40;
    display.drawLine(cx2,CENTER_Y-30,dx,CENTER_Y-14,ec);
    display.drawLine(cx2,CENTER_Y-14,dx,CENTER_Y-30,ec);
    display.drawLine(cx2+1,CENTER_Y-30,dx+1,CENTER_Y-14,ec);
    display.drawLine(cx2+1,CENTER_Y-14,dx+1,CENTER_Y-30,ec);

    display.drawLine(CENTER_X-42,CENTER_Y-36,CENTER_X-18,CENTER_Y-32,COLOR_ORANGE);
    display.drawLine(CENTER_X+18,CENTER_Y-32,CENTER_X+42,CENTER_Y-36,COLOR_ORANGE);
    display.drawLine(CENTER_X-42,CENTER_Y-35,CENTER_X-18,CENTER_Y-31,COLOR_ORANGE);
    display.drawLine(CENTER_X+18,CENTER_Y-31,CENTER_X+42,CENTER_Y-35,COLOR_ORANGE);
  }

  display.fillCircle(CENTER_X, CENTER_Y+35, 16, COLOR_BLACK);
  int mr = (frame==0) ? 10 : 16;
  display.fillCircle(CENTER_X, CENTER_Y+35, mr, COLOR_WHITE);

  display.fillRect(CENTER_X+42, CENTER_Y-52, 32, 52, COLOR_BLACK);
  int sy = (frame==0) ? CENTER_Y-32 : CENTER_Y-18;
  display.fillCircle(CENTER_X+55, sy, 6, COLOR_CYAN);
  display.fillTriangle(CENTER_X+49, sy, CENTER_X+61, sy, CENTER_X+55, sy-14, COLOR_CYAN);
}

void drawMaskFace(int frame, bool fullRedraw) {
  uint16_t mc = COLOR_CYAN;
  uint16_t sc = COLOR_WHITE;
  
  if (fullRedraw) {
    display.drawLine(CENTER_X-46,CENTER_Y+8, CENTER_X-82,CENTER_Y-10,sc);
    display.drawLine(CENTER_X-46,CENTER_Y+9, CENTER_X-82,CENTER_Y-9, sc);
    display.drawLine(CENTER_X+46,CENTER_Y+8, CENTER_X+82,CENTER_Y-10,sc);
    display.drawLine(CENTER_X+46,CENTER_Y+9, CENTER_X+82,CENTER_Y-9, sc);
    display.drawLine(CENTER_X-46,CENTER_Y+48,CENTER_X-78,CENTER_Y+66,sc);
    display.drawLine(CENTER_X+46,CENTER_Y+48,CENTER_X+78,CENTER_Y+66,sc);
    display.fillRoundRect(CENTER_X-48,CENTER_Y+5,96,55,10,mc);
    display.drawLine(CENTER_X-38,CENTER_Y+22,CENTER_X+38,CENTER_Y+22,sc);
    display.drawLine(CENTER_X-38,CENTER_Y+38,CENTER_X+38,CENTER_Y+38,sc);
    display.fillRoundRect(CENTER_X-14,CENTER_Y+6,28,6,3,COLOR_LGRAY);
    
    display.setTextColor(COLOR_RED, mc);
    display.setTextSize(1);
    display.setCursor(CENTER_X-18, CENTER_Y+46);
    display.print("BAHAYA!");
  }

  if (frame == 0) {
    display.fillRect(CENTER_X-38,CENTER_Y-26,20,7,COLOR_BLACK);
    display.fillRect(CENTER_X+18,CENTER_Y-26,20,7,COLOR_BLACK);
    display.fillCircle(CENTER_X-28, CENTER_Y-22, 9, COLOR_RED);
    display.fillCircle(CENTER_X+28, CENTER_Y-22, 9, COLOR_RED);
    display.fillCircle(CENTER_X-26,CENTER_Y-20,4,COLOR_BLACK);
    display.fillCircle(CENTER_X+30,CENTER_Y-20,4,COLOR_BLACK);
    
    display.setTextColor(COLOR_RED, COLOR_BLACK);
    display.setTextSize(2);
    display.setCursor(CENTER_X-6, CENTER_Y-88);
    display.print("!");
  } else {
    display.fillCircle(CENTER_X-28, CENTER_Y-22, 9, COLOR_BLACK);
    display.fillCircle(CENTER_X+28, CENTER_Y-22, 9, COLOR_BLACK);
    display.fillRect(CENTER_X-12,CENTER_Y-92,24,22,COLOR_BLACK);
    display.fillRect(CENTER_X-38,CENTER_Y-26,20,7,COLOR_RED);
    display.fillRect(CENTER_X+18,CENTER_Y-26,20,7,COLOR_RED);
  }
}

void drawHeatRing(float ppm) {
  uint16_t c = ppmColor(ppm);
  display.drawCircle(CENTER_X,CENTER_Y,119,c);
  display.drawCircle(CENTER_X,CENTER_Y,118,c);
  display.drawCircle(CENTER_X,CENTER_Y,117,c);
}

// ============================================================
// AOD & DATA PAGE ...
// ============================================================
void drawAnalogClock(int cx, int cy, int r, bool fullRedraw) {
  time_t now; time(&now);
  struct tm *ti = localtime(&now);
  float sA = (ti->tm_sec*6-90)*PI/180.0f;
  float mA = (ti->tm_min*6+ti->tm_sec*0.1f-90)*PI/180.0f;
  float hA = ((ti->tm_hour%12)*30+ti->tm_min*0.5f-90)*PI/180.0f;

  static float old_sA = 999, old_mA = 999, old_hA = 999;

  if (fullRedraw) {
    display.fillCircle(cx, cy, r+2, COLOR_BLACK);
    display.drawCircle(cx, cy, r, COLOR_LGRAY);
    display.drawCircle(cx, cy, r-1, COLOR_DGRAY);

    for (int i=0; i<12; i++){
      float a = (i*30-90)*PI/180.0f;
      int tx = cx + (int)((r-5)*cosf(a));
      int ty = cy + (int)((r-5)*sinf(a));
      if (i % 3 == 0) display.fillRect(tx-2, ty-2, 4, 4, COLOR_WHITE);
      else            display.fillCircle(tx, ty, 1, COLOR_LGRAY);
    }
    old_sA = 999; 
  }

  if (sA == old_sA) return; 

  if (old_sA != 999) {
    int sx2 = cx + (int)((r*0.88f)*cosf(old_sA));
    int sy2 = cy + (int)((r*0.88f)*sinf(old_sA));
    display.drawLine(cx, cy, sx2, sy2, COLOR_BLACK);

    int mx2 = cx + (int)((r*0.75f)*cosf(old_mA));
    int my2 = cy + (int)((r*0.75f)*sinf(old_mA));
    display.drawLine(cx, cy, mx2, my2, COLOR_BLACK);
    display.drawLine(cx+1, cy, mx2+1, my2, COLOR_BLACK);

    int hx = cx + (int)((r*0.50f)*cosf(old_hA));
    int hy = cy + (int)((r*0.50f)*sinf(old_hA));
    display.drawLine(cx, cy, hx, hy, COLOR_BLACK);
    display.drawLine(cx+1, cy, hx+1, hy, COLOR_BLACK);
    display.drawLine(cx, cy+1, hx, hy+1, COLOR_BLACK);
  }

  for (int i=0; i<12; i++){
    float a = (i*30-90)*PI/180.0f;
    int tx = cx + (int)((r-5)*cosf(a));
    int ty = cy + (int)((r-5)*sinf(a));
    if (i % 3 == 0) display.fillRect(tx-2, ty-2, 4, 4, COLOR_WHITE);
    else            display.fillCircle(tx, ty, 1, COLOR_LGRAY);
  }

  int hx = cx + (int)((r*0.50f)*cosf(hA));
  int hy = cy + (int)((r*0.50f)*sinf(hA));
  display.drawLine(cx, cy, hx, hy, COLOR_WHITE);
  display.drawLine(cx+1, cy, hx+1, hy, COLOR_WHITE);
  display.drawLine(cx, cy+1, hx, hy+1, COLOR_WHITE);

  int mx2 = cx + (int)((r*0.75f)*cosf(mA));
  int my2 = cy + (int)((r*0.75f)*sinf(mA));
  display.drawLine(cx, cy, mx2, my2, COLOR_CYAN);
  display.drawLine(cx+1, cy, mx2+1, my2, COLOR_CYAN);

  int sx2 = cx + (int)((r*0.88f)*cosf(sA));
  int sy2 = cy + (int)((r*0.88f)*sinf(sA));
  display.drawLine(cx, cy, sx2, sy2, COLOR_RED);

  display.fillCircle(cx, cy, 5, COLOR_ORANGE);
  display.fillCircle(cx, cy, 2, COLOR_WHITE);

  old_sA = sA; old_mA = mA; old_hA = hA;
}

void drawAOD(bool fullRedraw) {
  drawAnalogClock(CENTER_X, 65, 52, fullRedraw);

  if (fullRedraw) {
    display.drawLine(CENTER_X-80, 118, CENTER_X+80, 118, COLOR_DGRAY);
    display.drawLine(CENTER_X-90, 152, CENTER_X+90, 152, COLOR_DGRAY);
  }

  time_t now; time(&now);
  if (now > 100000) {
    struct tm *ti = localtime(&now);
    char tb[9]; sprintf(tb, "%02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);
    printCenter(122, tb, COLOR_WHITE, COLOR_BLACK, 2);

    char db[12]; sprintf(db, "%02d/%02d/%04d", ti->tm_mday, ti->tm_mon+1, ti->tm_year+1900);
    printCenter(140, db, COLOR_LGRAY, COLOR_BLACK, 1);
  } else {
    printCenter(122, "Zzz..", COLOR_YELLOW, COLOR_BLACK, 2);
  }

  char pb[16]; sprintf(pb, "Gas: %.0f PPM", currentPPM);
  printCenter(158, pb, ppmColor(currentPPM), COLOR_BLACK, 1);

  String cs = "Cuaca: " + cuacaStatus;
  printCenter(170, cs.c_str(), COLOR_CYAN, COLOR_BLACK, 1);

  char tmp[12]; sprintf(tmp, "Suhu: %.1f C", currentTemp);
  printCenter(182, tmp, (currentTemp>=32)?COLOR_ORANGE:COLOR_WHITE, COLOR_BLACK, 1);

  printCenter(194, ppmLevel(currentPPM), ppmColor(currentPPM), COLOR_BLACK, 1);
}

void drawDataPageFull() {
  display.fillRect(CENTER_X-55, 5, 110, 26, COLOR_TEAL);
  display.setTextColor(COLOR_BLACK, COLOR_TEAL);
  display.setTextSize(2);
  display.setCursor(CENTER_X-36, 10);
  display.print("SENSOR");
}

void drawDataPageUpdate(bool doFullDraw) {
  char line[24];
  int y = 38;

  sprintf(line,"Suhu  : %.1f C", currentTemp);
  printCenter(y, line, (currentTemp>=31)?COLOR_ORANGE:COLOR_WHITE, COLOR_BLACK, 1);
  y += 17;

  sprintf(line,"Hum   : %.0f %%", currentHum);
  printCenter(y, line, COLOR_CYAN, COLOR_BLACK, 1);
  y += 17;

  sprintf(line,"Tekan : %.0f hPa", currentPress);
  printCenter(y, line, COLOR_YELLOW, COLOR_BLACK, 1);
  y += 17;

  sprintf(line,"Cuaca : %s", cuacaStatus.c_str());
  printCenter(y, line, COLOR_GREEN, COLOR_BLACK, 1);
  y += 14;

  printCenter(y, infoCuacaDetail.c_str(), COLOR_PINK, COLOR_BLACK, 1);
  y += 14;

  if (doFullDraw) {
    int hw = safeHW(y);
    display.drawLine(CENTER_X-hw+12, y, CENTER_X+hw-12, y, COLOR_DGRAY);
  }
  y += 7;

  char ppmBig[12]; sprintf(ppmBig,"%.0f PPM",currentPPM);
  printCenter(y, ppmBig, ppmColor(currentPPM), COLOR_BLACK, 2);
  y += 22;

  printCenter(y, ppmLevel(currentPPM), ppmColor(currentPPM), COLOR_BLACK, 1);
  y += 14;

  if (bufferFull) {
    char pr[20]; sprintf(pr,"Pred: %.0f PPM",predictedPPM);
    printCenter(y, pr, ppmColor(predictedPPM), COLOR_BLACK, 1);
  } else {
    printCenter(y, "Tunggu prediksi..", COLOR_LGRAY, COLOR_BLACK, 1);
  }
  y += 14;

  static int lastDrawnIdx = -1;
  int bx=CENTER_X-72, bw=144, bh=24, by=196;

  if (doFullDraw) {
    display.fillRect(bx, by, bw, bh, COLOR_DGRAY);
    display.drawRect(bx, by, bw, bh, COLOR_LGRAY);
    display.setTextColor(COLOR_LGRAY, COLOR_DGRAY);
    display.setTextSize(1);
    display.setCursor(bx+2, by+2);
    display.print("PPM log");
    lastDrawnIdx = -1; 
  }

  if (lastDrawnIdx != bufferIndex) {
    int barW = bw/BUFFER_SIZE;
    float maxP = 2000.0f;
    for(int i=0; i<BUFFER_SIZE; i++){
      int idx = (bufferIndex+i)%BUFFER_SIZE;
      int bH = constrain((int)(ppmBuffer[idx]/maxP*(bh-4)), 1, bh-4);
      display.fillRect(bx+i*barW+1, by+1, barW-1, bh-3-bH, COLOR_DGRAY);
      display.fillRect(bx+i*barW+1, by+bh-2-bH, barW-1, bH, ppmColor(ppmBuffer[idx]));
    }
    lastDrawnIdx = bufferIndex;
  }

  time_t t; time(&t);
  if (t>100000){
    struct tm *ti=localtime(&t);
    char ck[9]; sprintf(ck,"%02d:%02d:%02d",ti->tm_hour,ti->tm_min,ti->tm_sec);
    printCenter(226, ck, COLOR_LGRAY, COLOR_BLACK, 1);
  }

  display.fillCircle(CENTER_X, 234, 4, wifiConnected?COLOR_GREEN:COLOR_RED);
}

// ============================================================
// TASK 2: DISPLAY (ENGINE UTAMA)
// ============================================================
void TaskDisplay(void *p) {
  static int bootAngle = 0;
  static int lastBarLen = 0;
  int lastFrame = -1;
  uint16_t lastRing = 0;

  for (;;) {

    // ===== GAME MODE (Flappy Badge) =====
    if (currentState == GAME_MODE) {
      // BACA TOMBOL RAW AGAR INSTAN & BISA DI-SPAM (Zero Latency)
      static bool lastBtnRaw = HIGH;
      bool currentBtnRaw = digitalRead(BUTTON_PIN);
      bool isJustPressed = (currentBtnRaw == LOW && lastBtnRaw == HIGH);
      lastBtnRaw = currentBtnRaw;

      if (isJustPressed) {
        lastActivityTime = millis();
        if (gameOver) {
          birdY = 120; birdV = 0; pipeX = 240; gameScore = 0; gameOver = false;
          pipeGapY = random(70, 170);
          forceRedraw = true;
        } else {
          gameJump = true;
        }
      }

      if (forceRedraw) {
        clearTextBuffer();
        display.fillScreen(COLOR_BLACK);
        if (!gameOver) {
          // Gambar pipa awal jika baru mulai main
          display.fillRect(pipeX, 0, 30, pipeGapY - 40, COLOR_GREEN);
          display.fillRect(pipeX, pipeGapY + 40, 30, 240 - (pipeGapY + 40), COLOR_GREEN);
        }
        forceRedraw = false;
      }

      if (!gameOver) {
        if (gameJump) { 
          birdV = -6.5; // Kekuatan lompat
          gameJump = false; 
          tone(BUZZER_PIN, NOTE_E5, 30); 
        }
        birdV += 0.8; // Tarikan gravitasi
        float oldBirdY = birdY;
        birdY += birdV;

        int pipeWidth = 30;
        int pipeGap = 80;
        int pipeSpeed = 5; // Kecepatan pipa
        
        int topHeight = pipeGapY - pipeGap/2;
        int bottomY = pipeGapY + pipeGap/2;
        int bottomHeight = 240 - bottomY;

        pipeX -= pipeSpeed;

        // Teknik Zero Flicker: Hanya hapus & gambar tepi pipa yang bergerak
        // Hapus ekor pipa (warna hitam)
        display.fillRect(pipeX + pipeWidth, 0, pipeSpeed, topHeight, COLOR_BLACK);
        display.fillRect(pipeX + pipeWidth, bottomY, pipeSpeed, bottomHeight, COLOR_BLACK);

        // Cek jika burung berhasil lewat celah pipa
        if (pipeX + pipeWidth < 60 && pipeX + pipeWidth + pipeSpeed >= 60) {
          gameScore++;
          tone(BUZZER_PIN, NOTE_C6, 50);
        }

        // Reset pipa jika sudah keluar layar
        if (pipeX < -pipeWidth) {
          pipeX = 240;
          pipeGapY = random(70, 170);
          topHeight = pipeGapY - pipeGap/2;
          bottomY = pipeGapY + pipeGap/2;
          bottomHeight = 240 - bottomY;
        }

        // Gambar kepala pipa (warna hijau)
        display.fillRect(pipeX, 0, pipeSpeed, topHeight, COLOR_GREEN);
        display.fillRect(pipeX, bottomY, pipeSpeed, bottomHeight, COLOR_GREEN);

        // Gambar ulang burung
        display.fillCircle(60, (int)oldBirdY, 5, COLOR_BLACK);
        display.fillCircle(60, (int)birdY, 5, COLOR_YELLOW);
        display.drawPixel(62, (int)birdY - 1, COLOR_BLACK); // Mata burung

        // Update skor di atas layar
        char sc[8]; sprintf(sc, "%d", gameScore);
        printCenter(20, sc, COLOR_WHITE, COLOR_BLACK, 3);

        // Deteksi Tabrakan (Collision)
        if (birdY > 240 || birdY < 0) gameOver = true;
        if (pipeX < 60+5 && pipeX+pipeWidth > 60-5) {
          if (birdY-5 < topHeight || birdY+5 > bottomY) gameOver = true;
        }

        // Jika Tabrakan, Tampilkan Game Over
        if (gameOver) {
          tone(BUZZER_PIN, NOTE_CS5, 150); vTaskDelay(pdMS_TO_TICKS(150));
          tone(BUZZER_PIN, NOTE_A4, 400);
          display.setTextColor(COLOR_RED, COLOR_BLACK);
          display.setTextSize(3);
          display.setCursor(CENTER_X - 72, 70);
          display.print("GAMEOVER");
          
          char sct[16]; sprintf(sct, "Score: %d", gameScore);
          printCenter(110, sct, COLOR_WHITE, COLOR_BLACK, 2);
          
          printCenter(160, "Tap to Restart", COLOR_YELLOW, COLOR_BLACK, 1);
          printCenter(180, "Hold to Exit", COLOR_LGRAY, COLOR_BLACK, 1);
        }
      }
      
      vTaskDelay(pdMS_TO_TICKS(30)); // Delay game frame (~33 FPS)
      continue;
    }

    // ===== AOD =====
    if (isIdleMode) {
      if (forceRedraw) {
        clearTextBuffer();
        display.fillScreen(COLOR_BLACK);
        display.drawCircle(CENTER_X,CENTER_Y,119,COLOR_TEAL);
        display.drawCircle(CENTER_X,CENTER_Y,118,COLOR_TEAL);
        display.drawCircle(CENTER_X,CENTER_Y,115,COLOR_DGRAY);
      }
      drawAOD(forceRedraw);
      forceRedraw = false;
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    // ===== SYSTEM CHECK MODE (3x Klik) =====
    if (currentState == CHECK_MODE) {
      if (forceRedraw) {
        clearTextBuffer();
        display.fillScreen(COLOR_BLACK);
        
        display.fillRect(0, 15, 240, 26, COLOR_BLUE);
        display.setTextColor(COLOR_WHITE, COLOR_BLUE);
        display.setTextSize(2);
        display.setCursor(CENTER_X - (12*12)/2, 20);
        display.print("SYSTEM CHECK");

        int y = 60;
        bool all_ok = true; // Tambahan variabel untuk track status
        
        // 1. Cek WiFi
        printCenter(y, "Checking WiFi...", COLOR_LGRAY, COLOR_BLACK, 1);
        vTaskDelay(pdMS_TO_TICKS(800));
        if (WiFi.status() == WL_CONNECTED) {
          printCenter(y, "WiFi: OK", COLOR_GREEN, COLOR_BLACK, 1);
        } else {
          printCenter(y, "WiFi: ERROR", COLOR_RED, COLOR_BLACK, 1);
          all_ok = false;
        }
        y += 20;

        // 2. Cek I2C (Live Hardware Ping)
        printCenter(y, "Checking I2C...", COLOR_LGRAY, COLOR_BLACK, 1);
        vTaskDelay(pdMS_TO_TICKS(800));
        Wire.beginTransmission(0x38); // Cek AHT20
        byte aht_err = Wire.endTransmission();
        Wire.beginTransmission(0x76); // Cek BMP280 (0x76)
        byte bmp_err = Wire.endTransmission();
        if (bmp_err != 0) {
            Wire.beginTransmission(0x77); // Cek BMP280 (0x77)
            bmp_err = Wire.endTransmission();
        }
        if (aht_err == 0 && bmp_err == 0) {
          printCenter(y, "I2C Sensor: OK", COLOR_GREEN, COLOR_BLACK, 1);
        } else {
          printCenter(y, "I2C Sensor: ERR", COLOR_RED, COLOR_BLACK, 1);
          all_ok = false;
        }
        y += 20;

        // 3. Cek Gas MQ135 (Live Analog Check)
        printCenter(y, "Checking Gas...", COLOR_LGRAY, COLOR_BLACK, 1);
        vTaskDelay(pdMS_TO_TICKS(800));
        int rawGas = analogRead(MQ135_PIN);
        // Jika kabel dilepas, nilai biasanya jatuh ke 0 atau naik penuh jadi noise 4095
        if (rawGas > 10 && rawGas < 4090) {
          printCenter(y, "MQ135 Gas: OK", COLOR_GREEN, COLOR_BLACK, 1);
        } else {
          printCenter(y, "MQ135 Gas: ERR", COLOR_RED, COLOR_BLACK, 1);
          all_ok = false;
        }
        y += 20;

        // 4. Cek Cloud
        printCenter(y, "Checking Cloud...", COLOR_LGRAY, COLOR_BLACK, 1);
        vTaskDelay(pdMS_TO_TICKS(800));
        if (ntpSynced) {
          printCenter(y, "Cloud & Time: OK", COLOR_GREEN, COLOR_BLACK, 1);
        } else {
          printCenter(y, "Cloud & Time: WAIT", COLOR_YELLOW, COLOR_BLACK, 1);
          all_ok = false;
        }
        y += 30;

        // 5. Kesimpulan Cek Semua Sistem
        if (all_ok) {
          // Nada Sukses (Naik)
          tone(BUZZER_PIN, NOTE_C5, 100); vTaskDelay(pdMS_TO_TICKS(130));
          tone(BUZZER_PIN, NOTE_E5, 100); vTaskDelay(pdMS_TO_TICKS(130));
          tone(BUZZER_PIN, NOTE_G5, 100); vTaskDelay(pdMS_TO_TICKS(130));
          tone(BUZZER_PIN, NOTE_C6, 200); vTaskDelay(pdMS_TO_TICKS(200));
          noTone(BUZZER_PIN);

          display.fillRoundRect(CENTER_X - 55, y, 110, 24, 4, COLOR_GREEN);
          display.setTextColor(COLOR_BLACK, COLOR_GREEN);
          display.setTextSize(2);
          display.setCursor(CENTER_X - (7*12)/2, y+4);
          display.print("ALL OK!");
        } else {
          // Nada Error (Turun)
          tone(BUZZER_PIN, NOTE_E5, 300); vTaskDelay(pdMS_TO_TICKS(350));
          tone(BUZZER_PIN, NOTE_C5, 400); vTaskDelay(pdMS_TO_TICKS(400));
          noTone(BUZZER_PIN);

          display.fillRoundRect(CENTER_X - 65, y, 130, 24, 4, COLOR_RED);
          display.setTextColor(COLOR_WHITE, COLOR_RED);
          display.setTextSize(2);
          display.setCursor(CENTER_X - (10*12)/2, y+4);
          display.print("SYSTEM ERR");
        }
        
        vTaskDelay(pdMS_TO_TICKS(3500)); 
        
        currentState = IDLE_SENSING; 
        forceRedraw = true;
      }
      continue;
    }

    // ===== BOOT =====
    if (currentState == BOOTING) {
      if (forceRedraw) {
        clearTextBuffer();
        display.fillScreen(COLOR_BLACK);
        
        display.setTextSize(2);
        display.setTextColor(COLOR_WHITE);
        display.setCursor(CENTER_X - (9*12)/2, 72);
        display.print("AtmoBadge");

        display.setTextSize(1);
        display.setTextColor(COLOR_TEAL);
        display.setCursor(CENTER_X - (16*6)/2, 94);
        display.print("~ NEON EDITION ~");

        display.setTextColor(COLOR_LGRAY);
        display.setCursor(CENTER_X - (12*6)/2, 148);
        display.print("Warming up..");
        
        int barX = CENTER_X - 65;
        display.fillRect(barX, 162, 130, 8, COLOR_DGRAY);
        display.drawRect(barX, 162, 130, 8, COLOR_LGRAY);
        lastBarLen = 0;
      }

      float pct = constrain((float)(millis()) / WAKTU_PEMANASAN, 0.0f, 1.0f);
      int barLen = (int)(130 * pct);
      int barX = CENTER_X - 65;
      
      for (int bx = lastBarLen; bx < barLen; bx++) {
        float r = (float)bx/130.0f;
        uint16_t c = (r<0.5f)?COLOR_CYAN:(r<0.85f)?COLOR_YELLOW:COLOR_GREEN;
        display.drawFastVLine(barX+bx, 163, 6, c);
      }
      lastBarLen = barLen;

      char pct_str[6]; sprintf(pct_str,"%d%%",(int)(pct*100));
      printCenter(174, pct_str, COLOR_WHITE, COLOR_BLACK, 1);

      for (int i=0;i<5;i++){
        float a=((bootAngle-i*20+360)%360)*PI/180.0f;
        int ox=CENTER_X+(int)(48*cosf(a));
        int oy=42+(int)(18*sinf(a));
        display.fillCircle(ox,oy,5,COLOR_BLACK);
      }
      bootAngle=(bootAngle+10)%360;
      uint16_t dc[]={COLOR_CYAN,COLOR_YELLOW,COLOR_ORANGE,COLOR_RED,COLOR_PURPLE};
      for (int i=0;i<5;i++){
        float a=((bootAngle-i*20+360)%360)*PI/180.0f;
        int ox=CENTER_X+(int)(48*cosf(a));
        int oy=42+(int)(18*sinf(a));
        display.fillCircle(ox,oy,5,dc[i]);
      }

      if (wifiConnected){
        printCenter(120, "WiFi OK!", COLOR_GREEN, COLOR_BLACK, 1);
      } else {
        static int dotCount=0;
        static unsigned long lastDot=0;
        if (millis()-lastDot>600){ dotCount=(dotCount+1)%4; lastDot=millis(); }
        char wstr[16]="WiFi konek    ";
        for(int d=0;d<dotCount;d++) wstr[10+d]='.'; 
        wstr[10+dotCount]='\0';
        printCenter(120, wstr, COLOR_YELLOW, COLOR_BLACK, 1);
      }

      forceRedraw = false;
      vTaskDelay(pdMS_TO_TICKS(40));
      continue;
    }

    // ===== DATA PAGE =====
    if (showDataPage) {
      if (forceRedraw) {
        clearTextBuffer();
        display.fillScreen(COLOR_BLACK);
        drawDataPageFull(); 
      }
      drawDataPageUpdate(forceRedraw);
      forceRedraw=false;
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // ===== FACE PAGE =====
    else {
      FaceType newFace;
      if (currentState == ALERT_MODE) newFace = FACE_MASK;
      else if (currentTemp >= 32.0f)  newFace = FACE_HOT;
      else                            newFace = FACE_SMILE;

      if (newFace != currentFace) {
        forceRedraw = true;
        currentFace = newFace;
      }

      if (forceRedraw) {
        clearTextBuffer();
        display.fillScreen(COLOR_BLACK);
        display.drawCircle(CENTER_X,CENTER_Y,94,COLOR_WHITE);
        display.drawCircle(CENTER_X,CENTER_Y,93,COLOR_LGRAY);
        lastFrame = -1; 
        lastRing = 0;
      }

      uint16_t rc = ppmColor(currentPPM);
      if (rc != lastRing || forceRedraw){ drawHeatRing(currentPPM); lastRing = rc; }

      int frame = (millis()/600)%2;
      if (frame != lastFrame || forceRedraw){
        bool doFullFace = forceRedraw || (lastFrame == -1);
        
        if (currentFace == FACE_MASK)        drawMaskFace(frame, doFullFace);
        else if (currentFace == FACE_HOT)    drawHotFace(frame, doFullFace);
        else                                 drawSmileFace(frame, doFullFace);
        
        lastFrame = frame;
      }

      char pt[12]; sprintf(pt,"%.0f PPM",currentPPM);
      printCenter(CENTER_Y+68, pt, ppmColor(currentPPM), COLOR_BLACK, 1);

      printCenter(CENTER_Y+80, ppmLevel(currentPPM), ppmColor(currentPPM), COLOR_BLACK, 1);

      char tt[8]; sprintf(tt,"%.1f C",currentTemp);
      printCenter(CENTER_Y-82, tt, COLOR_CYAN, COLOR_BLACK, 1);

      time_t t; time(&t);
      if (t>100000){
        struct tm *ti=localtime(&t);
        char ck[6]; sprintf(ck,"%02d:%02d",ti->tm_hour,ti->tm_min);
        printCenter(12, ck, COLOR_WHITE, COLOR_BLACK, 1);
      }

      display.fillCircle(CENTER_X, 232, 4, wifiConnected ? COLOR_GREEN : COLOR_RED);

      forceRedraw = false;
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

// ============================================================
// FETCH CUACA DARI OPENWEATHERMAP
// ============================================================
void fetchWeatherOWM() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  char url[200];
  snprintf(url, sizeof(url),
    "https://api.openweathermap.org/data/2.5/weather"
    "?lat=%.4f&lon=%.4f&appid=%s&units=metric&lang=id",
    OWM_LAT, OWM_LON, OWM_API_KEY);

  http.begin(client, url);
  http.setTimeout(10000);
  int code = http.GET();
  Serial.println("[OWM] HTTP GET: " + String(code));

  if (code == 200) {
    String body = http.getString();
    int descIdx = body.indexOf("\"description\":\"");
    if (descIdx >= 0) {
      int start = descIdx + 15;
      int end   = body.indexOf("\"", start);
      if (end > start) {
        String desc = body.substring(start, end);
        desc[0] = toupper(desc[0]);
        cuacaStatus = desc;
      }
    }

    int tempIdx = body.indexOf("\"temp\":");
    float owmTemp = 0;
    if (tempIdx >= 0) owmTemp = body.substring(tempIdx + 7, tempIdx + 12).toFloat();

    int humIdx = body.indexOf("\"humidity\":");
    int owmHum = 0;
    if (humIdx >= 0) owmHum = body.substring(humIdx + 11, humIdx + 14).toInt();

    char detail[32];
    snprintf(detail, sizeof(detail), "OWM: %.1fC  Hum:%d%%", owmTemp, owmHum);
    infoCuacaDetail = String(detail);
  } else if (code == 401) {
    cuacaStatus    = "API Key Error";
    infoCuacaDetail = "Cek OWM_API_KEY";
  } else {
    cuacaStatus    = "OWM Error";
    infoCuacaDetail = "HTTP: " + String(code);
  }

  http.end();
}

// ============================================================
// TASK 3: NETWORK — WiFi + NTP + OWM + Telegram
// ============================================================
void TaskNetwork(void *p) {
  String *msgPtr;
  unsigned long lastReconnect  = 0;
  unsigned long lastWeatherFetch = 0;
  bool ntpTried = false;

  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!wifiConnected) {
        wifiConnected = true;
        Serial.println("[NET] WiFi tersambung! IP: " + WiFi.localIP().toString());
        ntpTried = false;           
        lastWeatherFetch = 0;
      }

      if (!ntpTried) {
        configTime(7*3600, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
        vTaskDelay(pdMS_TO_TICKS(2000));
        time_t t; time(&t);
        if (t > 100000) { ntpSynced = true; Serial.println("[NET] NTP OK!"); }
        ntpTried = true;
      }

      if (millis() - lastWeatherFetch >= OWM_INTERVAL_MS || lastWeatherFetch == 0) {
        lastWeatherFetch = millis();
        fetchWeatherOWM();
      }

    } else {
      if (wifiConnected) {
        wifiConnected = false;
        Serial.println("[NET] WiFi putus.");
      }
      cuacaStatus    = "Offline";
      infoCuacaDetail = "Tidak ada WiFi";

      if (millis() - lastReconnect > 20000) {
        lastReconnect = millis();
        Serial.println("[NET] Mencoba reconnect WiFi...");
        WiFi.disconnect(true);
        vTaskDelay(pdMS_TO_TICKS(500));
        WiFi.begin(ssid, password);
        ntpTried = false;
      }
    }

    if (xQueueReceive(telegramQueue, &msgPtr, pdMS_TO_TICKS(500)) == pdPASS) {
      if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client; client.setInsecure();
        HTTPClient http;
        String url = "https://api.telegram.org/bot" + String(telegramToken) + "/sendMessage";
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(10000);

        String msg = *msgPtr;
        msg += "\n\n📊 *DATA SENSOR:*";
        msg += "\n🌡 Suhu: "    + String(currentTemp, 1) + "C";
        msg += "\n💧 Lembap: "  + String(currentHum, 0) + "%";
        msg += "\n🌀 Tekanan: " + String(currentPress, 0) + " hPa";
        msg += "\n☁ Cuaca: "   + cuacaStatus;
        msg += "\n💨 Gas: "     + String(currentPPM, 0) + " PPM [" + String(ppmLevel(currentPPM)) + "]";
        if (bufferFull) msg += "\n🔮 Pred 5min: " + String(predictedPPM, 0) + " PPM";
        msg += "\n📍 " + infoCuacaDetail;
        msg.replace("\"", "'");

        String payload = "{\"chat_id\":\"" + String(telegramChatId) +
                         "\",\"text\":\"" + msg +
                         "\",\"parse_mode\":\"Markdown\"}";
        int code = http.POST(payload);
        Serial.println("[NET] Telegram HTTP: " + String(code));
        http.end();
      } else {
        Serial.println("[NET] WiFi offline, pesan dibuang.");
      }
      delete msgPtr;
    }
  }
}

// ============================================================
// TOMBOL (Sistem Input Utama & Game)
// ============================================================
void aksiKlik1() {
  lastActivityTime=millis();
  
  // Abaikan handler OneButton saat nge-game (sudah di-handle secara raw agar instan)
  if (currentState == GAME_MODE) return;

  // Kalau normal: 1 Klik = Pindah Menu Data / Wajah
  if (isIdleMode){ isIdleMode=false; forceRedraw=true; return; }
  showDataPage=!showDataPage;
  forceRedraw=true;
}

void aksiKlik2() {
  lastActivityTime=millis();
  if (currentState == GAME_MODE) return; // Disable saat nge-game

  if (isIdleMode){ isIdleMode=false; forceRedraw=true; return; }
  String *msg=new String("📊 *Laporan Manual AtmoBadge*");
  xQueueSend(telegramQueue,&msg,0);
}

void aksiMultiKlik() {
  int clicks = btn.getNumberClicks();
  if (currentState == GAME_MODE) return; // Disable saat nge-game

  if (clicks == 3 && currentState != BOOTING) {
    // 3 Klik = Masuk ke Diagnostic / Auto Check
    lastActivityTime = millis();
    if (isIdleMode) { isIdleMode = false; }
    currentState = CHECK_MODE;
    forceRedraw = true;
    
  } else if (clicks == 4 && currentState != BOOTING) {
    // 4 Klik = Masuk ke GAME MODE (Hidden Feature)
    lastActivityTime = millis();
    if (isIdleMode) { isIdleMode = false; }
    currentState = GAME_MODE;
    
    // Reset Data Game Awal
    birdY = 120; birdV = 0; pipeX = 240; gameScore = 0; gameOver = false;
    pipeGapY = random(70, 170);
    forceRedraw = true;
  }
}

void aksiTahan() {
  lastActivityTime=millis();

  // Kalau lagi main Game, tahan tombol untuk KELUAR dari game
  if (currentState == GAME_MODE) {
    currentState = IDLE_SENSING;
    forceRedraw = true;
    return;
  }

  // Normal: tahan tombol untuk masuk Sleep / AOD
  isIdleMode=!isIdleMode;
  forceRedraw=true;
}

// ============================================================
// TASK 4: MARIO BROS (Hanya waktu booting)
// ============================================================
void TaskMario(void *p) {
  int totalNotes = sizeof(marioMelody) / sizeof(marioMelody[0]);

  while (currentState == BOOTING) {
    for (int i = 0; i < totalNotes; i++) {
      if (currentState != BOOTING) break; 

      int noteDur = 1000 / marioDurations[i];

      if (marioMelody[i] == 0) {
        noTone(BUZZER_PIN);
        vTaskDelay(pdMS_TO_TICKS(noteDur));
      } else {
        tone(BUZZER_PIN, marioMelody[i], noteDur);
        vTaskDelay(pdMS_TO_TICKS(noteDur * 1.30));
        noTone(BUZZER_PIN);
      }
    }

    if (currentState == BOOTING) {
      vTaskDelay(pdMS_TO_TICKS(1500)); 
    }
  }

  noTone(BUZZER_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  vTaskDelete(NULL); 
}
