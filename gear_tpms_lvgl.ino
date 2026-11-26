/*
  Gear Indicator + TPMS Monitor (รวมสองโปรเจ็ค)
  บอร์ด: ESP32 (รุ่นคลาสสิก)   จอ: ILI9488 3.5" 480x320 แนวนอน   กราฟิก: LVGL 9.1 ขึ้นไป

  ซ้ายจอ  = TPMS รถมองจากด้านบน + 4 ล้อ (สถานะ แรงดัน อุณหภูมิ แบตเซนเซอร์)
  ขวาจอ   = ค่าฮอลล์ 3 ตัว (H0/H1/H2) + ตัวเลขเกียร์ใหญ่ (N, 1-5, R)
  ไฟ WS2812B 7 ดวงแสดงเกียร์ด้วย ปรับความสว่างตาม LDR

  ----- ไลบรารีที่ต้องติดตั้ง (Library Manager) -----
    lvgl (เวอร์ชัน 9.1 ขึ้นไป)   TFT_eSPI (by Bodmer)   Adafruit NeoPixel
  ต้องวางไฟล์ lv_conf.h (แถมมาด้วย) ไว้ในโฟลเดอร์ libraries ข้างๆ โฟลเดอร์ lvgl

  ----- ตั้งค่า TFT_eSPI (ไฟล์ User_Setup.h ในโฟลเดอร์ไลบรารี) -----
    ใช้ไฟล์ User_Setup.h ที่แถมมา (ILI9488 / MOSI 23 / SCLK 18 / CS 5 / DC 2 / RST 4 / MISO ไม่ต่อ = -1 / SPI 27MHz)
    ขาจอต้องตรงกับที่ต่อสายจริง ถ้าต่อไม่เหมือนให้แก้ตัวเลขในไฟล์ User_Setup.h นั้น (ไม่ใช่ในไฟล์นี้)
    ขาอื่นที่ sketch นี้ใช้ (ต้องไม่ชนกับขาจอ): ไฟ WS2812B 13 | LDR 26 | ฮอลล์ 34/35/32
    ปุ่ม 3 ตัว: MENU = GPIO 27, NEXT = GPIO 25, OK = GPIO 33  (ต่อปุ่มระหว่างขา GPIO กับ GND ใช้ pull-up ในตัว ไม่ต้องใส่ตัวต้านทาน)

  ----- ตั้งค่า Arduino IDE -----
    Board: "ESP32 Dev Module"   Partition Scheme: "Huge APP (3MB No OTA)"

  ----- ปุ่ม 3 ตัว -----
    หน้าจอปกติ : MENU = เปิดเมนู | NEXT = สลับหน่วย psi/bar (ทางลัด)
    ในเมนู     : NEXT = เลื่อนไปรายการถัดไป | OK = เลือก | MENU = ปิดเมนู
      รายการเมนู: Pair TPMS sensors (จับคู่เซนเซอร์) / Unit: PSI-BAR / Mode: DEMO-REAL / Close
    ระหว่างจับคู่: NEXT = ข้ามล้อนี้ | MENU = ยกเลิก (ค่าเดิมคืนกลับ)
    โหมด DEMO/REAL จำไว้ในบอร์ด เปลี่ยนแล้วบอร์ดรีสตาร์ทเอง ไม่ต้องอัปโหลดโค้ดใหม่ ค่าเกียร์ที่เรียนรู้ไว้ไม่หาย

  ----- คำสั่ง Serial (115200, ท้ายบรรทัดเป็น Newline) -----
    คำสั่งเกียร์ (ดูตัวอักษรแรกของบรรทัด เหมือนโค้ดเดิม):
      n 1 2 3 4 5 r  เรียนรู้ตำแหน่งเกียร์   p ดูค่าตอนนี้   l ดูตาราง
      b ดูค่า LDR   s เปิด/ปิดพิมพ์ต่อเนื่อง   x ลบค่าเกียร์ที่เรียนรู้
    คำสั่ง TPMS (พิมพ์ทั้งคำ):
      show | lo 29 | warn 32 | hi 46 | reset   (ค่าเป็นหน่วยที่แสดงบนจอ)
      pair | unit psi / unit bar | mode demo / mode real
    ล็อกเกียร์: learn off (ล็อกกันพิมพ์พลาด) | learn on (เปิดเรียนรู้) | learn (ดูสถานะ)
*/

/* ================= OPTIONS ================= */
#define DEMO_DEFAULT   1    // โหมด TPMS ตอนยังไม่เคยตั้งค่า: 1 = ข้อมูลจำลอง (DEMO) | 0 = เซนเซอร์จริง (REAL)
                            // เปลี่ยนภายหลังได้จากเมนูบนจอหรือคำสั่ง "mode real" ไม่ต้องอัปโหลดใหม่
#define USE_TFT        1    // 1 = ใช้จอ TFT | 0 = ไม่ใช้จอ (ถอดจอถาวร) ไฟ WS2812B และ TPMS ยังทำงานต่อ
#define USE_LED        1    // 1 = ใช้ไฟ WS2812B | 0 = ไม่ใช้
#define DEBUG_MODE     1    // 1 = เปิด Serial (ใช้ดูค่า/เรียนรู้ผ่าน USB) | เรียนรู้เกียร์ผ่านหน้าเว็บ WiFi ได้โดยไม่ต้องใช้ Serial
#define USE_CAR_IMAGE  0    // 1 = ใช้รูปรถจากไฟล์ car_img.c แทนรถที่วาดด้วยโค้ด (ดูคำอธิบายท้ายไฟล์)
#define USE_BACKGROUND 0    // 1 = ใช้รูปพื้นหลังจากไฟล์ bg_img.c (480x320) แผงข้อมูลจะโปร่งใสให้เห็นพื้นหลัง
#define TFT_ROTATION   1    // 1 = แนวนอน (ถ้าภาพกลับหัวให้เปลี่ยนเป็น 3)
#define WIFI_SSID       "GearTPMS"     // ชื่อ WiFi ที่บอร์ดปล่อย (เปิดจากเมนู MENU -> WiFi)
#define WIFI_PASS       "12345678"     // รหัสผ่าน (ต้องยาว 8 ตัวขึ้นไป ควรเปลี่ยน)
#define WIFI_IDLE_MS    (15UL * 60UL * 1000UL)   // ไม่มีการกดใช้งานบนหน้าเว็บนานเท่านี้ WiFi ปิดเอง
#define WIFI_MIN_HEAP   60000          // หน่วยความจำว่างต่ำกว่านี้ ไม่ยอมเปิด WiFi (กันบอร์ดค้างตอนใช้ร่วมกับ BLE)
#define FW_VERSION      "wifi-2"
#define MAX_RUNS        10             // เก็บข้อมูลการเรียนรู้ได้สูงสุดกี่รอบต่อเกียร์ (เต็มแล้วรอบเก่าสุดถูกแทน)
#define CAP_DEFAULT_S   10             // เก็บค่าต่อรอบกี่วินาที (หน้าเว็บเลือก 3-30 วินาทีได้)
#define RESET_SETTINGS 0    // 1 = ล้างค่า TPMS ที่จำไว้ตอนเปิดเครื่อง (อัปโหลดครั้งเดียวแล้วเปลี่ยนกลับเป็น 0)

#include <WiFi.h>          // ต้องอยู่ก่อน TFT_eSPI (ไม่งั้น WebServer.h หา FS ไม่เจอ)
#include <WebServer.h>
#if USE_TFT
#include <lvgl.h>
#include <TFT_eSPI.h>
#endif
#include <Preferences.h>
#if USE_LED
#include <Adafruit_NeoPixel.h>
#endif
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "web_ui.h"             // หน้าเว็บเรียนรู้เกียร์ / ตั้งค่า TPMS (ไฟล์ web_ui.h วางโฟลเดอร์เดียวกับ .ino)

#define SCR_W 480
#define SCR_H 320

#if USE_TFT
TFT_eSPI tft = TFT_eSPI();
#endif
Preferences prefsGear;
Preferences prefsTpms;

/* ================= PINS ================= */
#define BTN_MENU  27    // ปุ่ม MENU (ต่อระหว่างขานี้กับ GND กดแล้ว = LOW)
#define BTN_NEXT  25    // ปุ่ม NEXT
#define BTN_OK    33    // ปุ่ม OK
#define MENU_N     5    // จำนวนรายการในเมนู

#define NH 3
#define HALL0_PIN 34    // ตัวบน
#define HALL1_PIN 35    // ตัวกลาง
#define HALL2_PIN 32    // ตัวล่าง
const uint8_t hallPin[NH] = {HALL0_PIN, HALL1_PIN, HALL2_PIN};

#define LED_PIN        13
#define LED_COUNT      7
#define LED_BRIGHT_MAX 255
#define LED_BRIGHT_MIN 25   // ประมาณ 10%

#define LDR_PIN        26
#define LDR_ANALOG     1     // 1 = ขา AO (ค่าต่อเนื่อง) | 0 = ขา DO (สว่าง/มืดอย่างเดียว)
#define LDR_INVERT     1     // 1 = โมดูลนี้ค่าสูงเมื่อมืด (กลับค่าให้ แสงน้อย=ไฟหรี่ แสงเยอะ=ไฟสว่าง) | 0 = ค่าสูงเมื่อสว่าง
#define LDR_DARK_VAL   500
#define LDR_BRIGHT_VAL 3000

#if USE_LED
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

/* ================= GEAR CONFIG ================= */
#define ADC_ATTEN       ADC_11db
#define AVG_SAMPLES     16
#define FILTER_ALPHA    0.3f
#define LEARN_SAMPLES   40
#define GEAR_STABLE_MS  350   // เกียร์ทั่วไปต้องนิ่งนานเท่านี้ก่อนขึ้นจอ
#define GEAR_WEAK_MS    700   // เกียร์ 5 และ R (สัญญาณอ่อน อยู่บนทางผ่านของ 3/4 ไป N) ต้องนิ่งนานกว่า
#define PATH_N_DIST     100.0f // ถือว่า "ผ่านช่อง N" เมื่อค่าอยู่ห่างจาก N ไม่เกินนี้
#define PATH_TIMEOUT_MS 4000   // ถ้ายืนยันเกียร์ใหม่ได้นานเกินนี้ ให้เชื่อค่าที่อ่าน แม้ไม่เห็นว่าผ่าน N
#define GEAR_HYST       30.0f // เกียร์ปัจจุบันถูกแทนที่ก็ต่อเมื่อเกียร์ใหม่ใกล้กว่าอย่างน้อยเท่านี้
#define UNKNOWN_DIST    250.0f
#define MIN_SEP         30.0f
#define DEBUG_INTERVAL  100
#define PANEL_INTERVAL  100

/* ================= TPMS CONFIG ================= */
// ตัวหารแปลงค่าแรงดันดิบเป็น kPa (ส่วนใหญ่ 1000) ถ้าค่าบนจอต่างจากจริง 10 เท่า ให้ลองเปลี่ยนเป็น 100
const float PRESSURE_DIV = 1000.0;

// MAC ของเซนเซอร์ (ตัวพิมพ์เล็ก) ถ้าไม่ใส่ จะใช้ค่าที่จับคู่ไว้ด้วยปุ่ม
const char* SENSOR_MAC[4] = { "", "", "", "" };

const float DEF_LOW = 200.0, DEF_WARN = 220.0, DEF_HIGH = 320.0;   // หน่วย kPa
float LOW_KPA   = DEF_LOW;
float WARN_KPA  = DEF_WARN;
float HIGH_KPA  = DEF_HIGH;
const float TEMP_HIGH_C = 70.0;
const int BATT_LOW_PCT = 20;
const uint32_t STALE_MS = 5UL * 60UL * 1000UL;

/* ================= COLORS (RGB888 แปลงจากสี RGB565 ของโค้ดเดิม) ================= */
// --- สีของโปรเจ็ค TPMS เดิม ---
#define COL_BG       0x080808   // C_BG     0x0841
#define COL_PANEL    0x101410   // C_PANEL  0x10A2
#define COL_DIM      0x63655A   // C_DIM    0x632B
#define COL_GRAY     0x7B797B   // C_GRAY   0x7BCF
#define COL_GREEN    0x00FF00   // C_GREEN  0x07E0
#define COL_YELLOW   0xFFD600   // C_YELLOW 0xFEA0
#define COL_RED      0xFF0000   // C_RED    0xF800
#define COL_WHITE    0xFFFFFF   // C_WHITE  0xFFFF
#define COL_AMBER    0xFFFF00   // C_AMBER  0xFFE0
// --- สีของโปรเจ็คเกียร์เดิม (สี TFT_eSPI) ---
#define COL_DKGREY   0x7B7D7B   // TFT_DARKGREY 0x7BEF (ป้ายฮอลล์)
#define COL_G_ORANGE 0xFFB400   // TFT_ORANGE   0xFDA0 (เกียร์ 1-5)
#define COL_G_CYAN   0x00FFFF   // TFT_CYAN     0x07FF (เกียร์ R)
#define COL_G_YELLOW 0xFFFF00   // TFT_YELLOW   0xFFE0 (LEARNED บางส่วน)
// เกียร์ N = COL_GREEN, LEARNED ครบ = COL_GREEN, ยังไม่เรียนรู้ = COL_RED (เหมือนเดิม)
// --- สีของส่วนที่มีเฉพาะในแบบที่วาด (ไม่มีในโค้ดเดิม) ---
#define COL_DESIGN_CYAN 0x00D9E0   // ตัวอักษร TPMS และเส้นแบ่งกลางจอ

/* ================= GLOBAL (GEAR) ================= */
int currentGear = 999;     // -1 = R, 0 = N, 1-5 = เกียร์ 1-5, 999 = ยังไม่เริ่ม
int lastCand = 999;
unsigned long candSince = 0;
bool streamOn = false;
bool learnEnabled = true;   // false = ล็อกการเรียนรู้เกียร์ (กันพิมพ์พลาดทับค่า) จำไว้ในบอร์ด

uint8_t ledBright = LED_BRIGHT_MAX;
float lightF = 0;

// ตำแหน่งในตาราง: 0 = N, 1-5 = เกียร์ 1-5, 6 = R
int16_t vecs[7][NH];                 // ค่ากลางของแต่ละเกียร์ (มัธยฐานของค่าเฉลี่ยทุกรอบ) ใช้จับคู่เกียร์
struct Run { int16_t mean[NH], lo[NH], hi[NH]; };   // 1 รอบ: ค่าเฉลี่ย / ต่ำสุด / สูงสุด ของฮอลล์แต่ละตัว
Run runs[7][MAX_RUNS];
struct LearnResult {
  int  spread[NH];     // ค่าแกว่ง (สูงสุด-ต่ำสุด) ของแต่ละฮอลล์ระหว่างเก็บ
  int  nearIdx;        // ตำแหน่งในตารางของเกียร์ที่ใกล้ที่สุด (-1 = ไม่มี)
  float nearDist;
};

uint8_t runCnt[7] = {0, 0, 0, 0, 0, 0, 0};
uint8_t learnedMask = 0;
int   hraw[NH];
float hf[NH];

struct Match {
  bool  valid;
  int   gear;
  float d1;
  int   gear2;
  float d2;
};

/* ================= GLOBAL (TPMS) ================= */
struct Tire {
  float kpa = NAN;
  float tempC = NAN;
  int batt = -1;
  bool alarm = false;
  uint32_t last = 0;
  volatile bool dirty = true;
  char mac[18] = "";
};
Tire tires[4];
bool demoMode = DEMO_DEFAULT;        // true = ข้อมูลจำลอง, false = เซนเซอร์จริง (จำไว้ในบอร์ด)
char oldMac[4][18];                  // สำรอง MAC ไว้คืนค่าถ้ายกเลิกการจับคู่
bool useBar = false;                 // false = psi (ค่าเริ่มต้น), กดปุ่มสั้นเพื่อสลับเป็น bar

volatile int pairingSlot = -1;       // -1 = ไม่ได้จับคู่อยู่, 0-3 = กำลังรอล้อนี้
volatile bool pairUiDirty = false;
volatile bool needSave = false;
const int PAIR_RSSI = -60;
const char* LABEL_LONG[4] = { "Front left", "Front right", "Rear left", "Rear right" };

enum { ST_NODATA, ST_OK, ST_WARN, ST_LOW, ST_HIGH, ST_HOT, ST_BATT, ST_ALARM };
const char* STATUS_TITLE[8] = { "No signal", "Normal", "Check", "Low press",
                                "High press", "High temp", "Low batt", "Alarm" };

int tireStatus(const Tire& t) {
  if (isnan(t.kpa) || millis() - t.last > STALE_MS) return ST_NODATA;
  if (t.alarm) return ST_ALARM;
  if (t.kpa > HIGH_KPA) return ST_HIGH;
  if (t.kpa < LOW_KPA) return ST_LOW;
  if (!isnan(t.tempC) && t.tempC > TEMP_HIGH_C) return ST_HOT;
  if (t.batt >= 0 && t.batt < BATT_LOW_PCT) return ST_BATT;
  if (t.kpa < WARN_KPA) return ST_WARN;
  return ST_OK;
}

uint32_t statusColor(int s) {
  switch (s) {
    case ST_OK: return COL_GREEN;
    case ST_WARN:
    case ST_BATT: return COL_YELLOW;
    case ST_LOW:
    case ST_HIGH:
    case ST_HOT:
    case ST_ALARM: return COL_RED;
    default: return COL_GRAY;
  }
}

/* ========================================================= */
/* ================= LVGL DISPLAY DRIVER + UI ============== */
/* ========================================================= */

#if USE_TFT

static uint8_t* drawBuf = nullptr;      // จองจาก heap ตอนเริ่มเครื่อง (ไม่กินพื้นที่ DRAM แบบ static)
static uint32_t drawBufBytes = 0;

static void flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* px) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)px, w * h, true);   // true = สลับไบต์ให้เอง
  tft.endWrite();
  lv_display_flush_ready(disp);
}

static uint32_t tickCb() { return millis(); }

// ---- ตัวช่วยสร้าง widget ----
lv_obj_t* mkRect(lv_obj_t* parent, int x, int y, int w, int h, int radius, uint32_t color) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

lv_obj_t* mkLabel(lv_obj_t* parent, int x, int y, const char* text, const lv_font_t* font, uint32_t color) {
  lv_obj_t* l = lv_label_create(parent);
  lv_label_set_text(l, text);
  lv_obj_set_pos(l, x, y);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
  return l;
}

// ---- โครง TPMS ----
struct PanelUI {
  lv_obj_t *box, *status, *press, *temp, *battBox, *battNub, *battFill, *pct, *tire;
};
PanelUI P[4];
lv_obj_t* unitLbl;
lv_obj_t *menuBox, *menuHi, *menuItem[MENU_N], *menuNote;

// ตำแหน่งแผง (จาก YAML) และยางบนรูปรถ
const int PNL_W = 65, PNL_H = 95;
const int PNL_X[4] = { 10, 184, 10, 184 };
const int PNL_Y[4] = { 18, 18, 217, 217 };
const int CAR_CX = 130;
const int TIRE_X[4] = { 86, 162, 86, 162 };
const int TIRE_Y[4] = { 52, 52, 215, 215 };
const int TIRE_W = 12, TIRE_H = 40;

// ---- โครงเกียร์ ----
const int GCX = 392;           // กึ่งกลางแนวนอนของตัวเลขเกียร์
const int GY0 = 66;            // ขอบบน
const int G_HL = 90;           // ความยาวส่วนนอน
const int G_T  = 22;           // ความหนาส่วน
const int G_GAP = 4;
const int G_VL = 76;           // ความยาวส่วนตั้ง
const int G_XL = GCX - G_HL / 2 - G_GAP - G_T;     // ขอบซ้ายสุด
const int G_XR = GCX + G_HL / 2 + G_GAP;            // x เริ่มของส่วนตั้งขวา
const int G_H  = 3 * G_T + 2 * G_VL + 4 * G_GAP;    // ความสูงรวม

lv_obj_t* seg[7];              // A B C D E F G
lv_obj_t* nBarL; lv_obj_t* nBarR; lv_obj_t* nDiag;
lv_obj_t* rBar; lv_obj_t* rTop; lv_obj_t* rMid; lv_obj_t* rRight; lv_obj_t* rLeg;
lv_obj_t* hallVal[NH];
lv_obj_t* learnedLbl;

static const bool SEG_TABLE[6][7] = {   // A B C D E F G
  {0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1}
};

static lv_point_precise_t nDiagPts[2] = { {0, 0}, {112, 216} };
static lv_point_precise_t rLegPts[2]  = { {0, 0}, {82, 98} };

// ---- overlay ตอนจับคู่ ----
lv_obj_t* pairBox; lv_obj_t* pairTitle;

void buildCar(lv_obj_t* scr) {
#if USE_CAR_IMAGE
  LV_IMAGE_DECLARE(car_img);
  lv_obj_t* img = lv_image_create(scr);
  lv_image_set_src(img, &car_img);
  lv_obj_set_pos(img, 0, 18);
#else
  mkRect(scr, CAR_CX - 32, 26, 64, 252, 26, 0xD01414);               // ตัวถัง
  mkRect(scr, CAR_CX - 5, 26, 3, 252, 0, 0x4A4A4A);                  // แถบกลาง
  mkRect(scr, CAR_CX + 2, 26, 3, 252, 0, 0x4A4A4A);
  mkRect(scr, CAR_CX - 25, 100, 50, 32, 6, 0x0C0C0C);                // กระจกหน้า
  mkRect(scr, CAR_CX - 27, 132, 54, 56, 8, 0x1A1A1A);                // หลังคา
  mkRect(scr, CAR_CX - 23, 188, 46, 28, 6, 0x0C0C0C);                // กระจกหลัง
  mkRect(scr, CAR_CX - 26, 32, 10, 4, 1, 0xFFC107);                  // ไฟหน้า
  mkRect(scr, CAR_CX + 16, 32, 10, 4, 1, 0xFFC107);
  mkRect(scr, CAR_CX - 24, 268, 10, 4, 1, 0x7A0000);                 // ไฟท้าย
  mkRect(scr, CAR_CX + 14, 268, 10, 4, 1, 0x7A0000);
#endif
}

void buildTpms(lv_obj_t* scr) {
  buildCar(scr);

  // ป้ายหน่วย PSI/BAR (ที่ว่างด้านหน้ารถ)
  unitLbl = mkLabel(scr, CAR_CX - 50, 3, "PSI", &lv_font_montserrat_16, COL_GRAY);
  lv_obj_set_width(unitLbl, 100);
  lv_obj_set_style_text_align(unitLbl, LV_TEXT_ALIGN_CENTER, 0);

  for (int i = 0; i < 4; i++) {
    // ยาง (สีตามสถานะ) + ลายดอกยาง 3 เส้น
    P[i].tire = mkRect(scr, TIRE_X[i], TIRE_Y[i], TIRE_W, TIRE_H, 3, COL_GRAY);
    for (int k = 1; k < 4; k++) mkRect(P[i].tire, 2, k * (TIRE_H / 4), TIRE_W - 4, 1, 0, COL_BG);

    // แผงข้อมูล (พื้นเทาเข้ม กรอบสีตามสถานะ)
    lv_obj_t* b = mkRect(scr, PNL_X[i], PNL_Y[i], PNL_W, PNL_H, 8, COL_PANEL);
    if (USE_BACKGROUND) lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);   // ให้เห็นพื้นหลังทะลุแผง
    lv_obj_set_style_border_width(b, 2, 0);
    lv_obj_set_style_border_color(b, lv_color_hex(COL_GRAY), 0);
    P[i].box = b;

    const int IW = PNL_W - 4;   // ความกว้างด้านในกรอบ

    P[i].status = mkLabel(b, 0, 3, "No signal", &lv_font_montserrat_10, COL_GRAY);
    lv_obj_set_width(P[i].status, IW);
    lv_label_set_long_mode(P[i].status, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(P[i].status, LV_TEXT_ALIGN_CENTER, 0);

    P[i].press = mkLabel(b, 0, 22, "--", &lv_font_montserrat_16, COL_GRAY);
    lv_obj_set_width(P[i].press, IW);
    lv_obj_set_style_text_align(P[i].press, LV_TEXT_ALIGN_CENTER, 0);

    mkRect(b, 6, 45, IW - 12, 1, 0, COL_DIM);   // เส้นคั่น

    P[i].temp = mkLabel(b, 0, 50, "--", &lv_font_montserrat_12, COL_WHITE);
    lv_obj_set_width(P[i].temp, IW);
    lv_obj_set_style_text_align(P[i].temp, LV_TEXT_ALIGN_CENTER, 0);

    // แบตเตอรี่ของเซนเซอร์ (วาดเหมือนโค้ดเดิม: กรอบ 16x9 + หัว + แถบเติม)
    P[i].battBox = mkRect(b, 5, 71, 16, 9, 0, COL_PANEL);
    if (USE_BACKGROUND) lv_obj_set_style_bg_opa(P[i].battBox, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(P[i].battBox, 1, 0);
    lv_obj_set_style_border_color(P[i].battBox, lv_color_hex(COL_GRAY), 0);
    P[i].battNub = mkRect(b, 21, 74, 2, 3, 0, COL_GRAY);
    P[i].battFill = mkRect(b, 7, 73, 12, 5, 0, COL_GRAY);
    P[i].pct = mkLabel(b, 27, 69, "--", &lv_font_montserrat_12, COL_GRAY);
  }

  mkLabel(scr, 99, 286, "TPMS", &lv_font_montserrat_24, COL_DESIGN_CYAN);
}

void buildGear(lv_obj_t* scr) {
  // เส้นแบ่งกลางจอ
  mkRect(scr, 260, 18, 2, 292, 0, COL_DESIGN_CYAN);

  mkLabel(scr, 322, 5, "Gear Indicator\n       By Bell", &lv_font_montserrat_16, COL_WHITE);

  // ค่าฮอลล์ (H0 บน / H1 กลาง / H2 ล่าง) สีตามโค้ดเดิม: ป้ายเทา ค่าขาว
  const int hy[NH] = {46, 152, 257};
  const char* hn[NH] = {"H0", "H1", "H2"};
  for (int i = 0; i < NH; i++) {
    mkLabel(scr, 275, hy[i], hn[i], &lv_font_montserrat_16, COL_DKGREY);
    hallVal[i] = mkLabel(scr, 271, hy[i] + 19, "0000", &lv_font_montserrat_12, COL_WHITE);
  }

  learnedLbl = mkLabel(scr, 330, 304, "LEARNED 0/7", &lv_font_montserrat_12, COL_RED);

  // ตัวเลข 7 ส่วน (สีส้มตามโค้ดเดิม)
  int xa = GCX - G_HL / 2;
  int yA = GY0;
  int yUpV = yA + G_T + G_GAP;
  int yG = yUpV + G_VL + G_GAP;
  int yLoV = yG + G_T + G_GAP;
  int yD = yLoV + G_VL + G_GAP;
  seg[0] = mkRect(scr, xa, yA, G_HL, G_T, 4, COL_G_ORANGE);              // A
  seg[1] = mkRect(scr, G_XR, yUpV, G_T, G_VL, 4, COL_G_ORANGE);          // B
  seg[2] = mkRect(scr, G_XR, yLoV, G_T, G_VL, 4, COL_G_ORANGE);          // C
  seg[3] = mkRect(scr, xa, yD, G_HL, G_T, 4, COL_G_ORANGE);              // D
  seg[4] = mkRect(scr, G_XL, yLoV, G_T, G_VL, 4, COL_G_ORANGE);          // E
  seg[5] = mkRect(scr, G_XL, yUpV, G_T, G_VL, 4, COL_G_ORANGE);          // F
  seg[6] = mkRect(scr, xa, yG, G_HL, G_T, 4, COL_G_ORANGE);              // G

  // ตัว N (เขียว): เสาซ้าย + เสาขวา + เส้นทแยง
  int gw = G_XR + G_T - G_XL;
  nBarL = mkRect(scr, G_XL, GY0, G_T, G_H, 3, COL_GREEN);
  nBarR = mkRect(scr, G_XL + gw - G_T, GY0, G_T, G_H, 3, COL_GREEN);
  nDiag = lv_line_create(scr);
  lv_line_set_points(nDiag, nDiagPts, 2);
  lv_obj_set_pos(nDiag, G_XL + G_T + 2, GY0 + 6);
  lv_obj_set_style_line_width(nDiag, 22, 0);
  lv_obj_set_style_line_rounded(nDiag, false, 0);
  lv_obj_set_style_line_color(nDiag, lv_color_hex(COL_GREEN), 0);

  // ตัว R (ฟ้า): เสาซ้าย + หัว (บน/กลาง/ขวา) + ขาทแยง
  int bowlMidY = GY0 + (G_H - G_T) / 2 - 10;
  rBar   = mkRect(scr, G_XL, GY0, G_T, G_H, 3, COL_G_CYAN);
  rTop   = mkRect(scr, G_XL, GY0, gw, G_T, 3, COL_G_CYAN);
  rMid   = mkRect(scr, G_XL, bowlMidY, gw, G_T, 3, COL_G_CYAN);
  rRight = mkRect(scr, G_XL + gw - G_T, GY0, G_T, bowlMidY + G_T - GY0, 3, COL_G_CYAN);
  rLeg   = lv_line_create(scr);
  lv_line_set_points(rLeg, rLegPts, 2);
  lv_obj_set_pos(rLeg, G_XL + G_T + 24, bowlMidY + G_T + 4);
  lv_obj_set_style_line_width(rLeg, 22, 0);
  lv_obj_set_style_line_rounded(rLeg, false, 0);
  lv_obj_set_style_line_color(rLeg, lv_color_hex(COL_G_CYAN), 0);
}

void buildMenu(lv_obj_t* scr) {
  menuBox = mkRect(scr, 100, 50, 280, 224, 10, COL_PANEL);
  lv_obj_set_style_bg_opa(menuBox, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(menuBox, 2, 0);
  lv_obj_set_style_border_color(menuBox, lv_color_hex(COL_DESIGN_CYAN), 0);
  mkLabel(menuBox, 12, 6, "MENU", &lv_font_montserrat_16, COL_DESIGN_CYAN);
  menuHi = mkRect(menuBox, 8, 32, 260, 28, 5, 0x2A3A3A);              // แถบไฮไลต์รายการที่เลือก
  for (int i = 0; i < MENU_N; i++)
    menuItem[i] = mkLabel(menuBox, 18, 36 + i * 30, "", &lv_font_montserrat_16, COL_WHITE);
  menuNote = mkLabel(menuBox, 12, 190, "", &lv_font_montserrat_12, COL_GRAY);
  lv_obj_set_width(menuNote, 256);
  lv_obj_add_flag(menuBox, LV_OBJ_FLAG_HIDDEN);
}

void buildPairOverlay(lv_obj_t* scr) {
  pairBox = mkRect(scr, 110, 110, 260, 90, 10, COL_PANEL);
  lv_obj_set_style_border_width(pairBox, 2, 0);
  lv_obj_set_style_border_color(pairBox, lv_color_hex(COL_AMBER), 0);
  pairTitle = mkLabel(pairBox, 0, 10, "", &lv_font_montserrat_24, COL_AMBER);
  lv_obj_set_width(pairTitle, 256);
  lv_obj_set_style_text_align(pairTitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_t* h1 = mkLabel(pairBox, 0, 46, "hold sensor near screen", &lv_font_montserrat_12, COL_WHITE);
  lv_obj_set_width(h1, 256);
  lv_obj_set_style_text_align(h1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_t* h2 = mkLabel(pairBox, 0, 62, "NEXT = skip    MENU = cancel", &lv_font_montserrat_12, COL_GRAY);
  lv_obj_set_width(h2, 256);
  lv_obj_set_style_text_align(h2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_add_flag(pairBox, LV_OBJ_FLAG_HIDDEN);
}

void buildUI() {
  lv_obj_t* scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
#if USE_BACKGROUND
  LV_IMAGE_DECLARE(bg_img);
  lv_obj_t* bg = lv_image_create(scr);
  lv_image_set_src(bg, &bg_img);
  lv_obj_set_pos(bg, 0, 0);
#endif
  buildTpms(scr);
  buildGear(scr);
  buildMenu(scr);
  buildPairOverlay(scr);     // สร้างหลังสุด = อยู่บนสุด
}

void uiInit() {
  tft.init();
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(TFT_BLACK);

  lv_init();
  lv_tick_set_cb(tickCb);
  lv_display_t* disp = lv_display_create(SCR_W, SCR_H);
  lv_display_set_flush_cb(disp, flushCb);
  uint32_t rows = 30;                     // 30 แถวต่อรอบ (~29 KB) ถ้าจองไม่ได้ลองลดลงครึ่งหนึ่ง
  while (rows >= 8 && !(drawBuf = (uint8_t*)malloc(SCR_W * rows * 2))) rows /= 2;
  if (!drawBuf) {
#if DEBUG_MODE == 1
    Serial.println("แรมไม่พอสำหรับบัฟเฟอร์จอ");
#endif
    for (;;) delay(1000);
  }
  drawBufBytes = SCR_W * rows * 2;
  lv_display_set_buffers(disp, drawBuf, nullptr, drawBufBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
  buildUI();
}

void uiTick() { lv_timer_handler(); }

void uiUnitLabel() {
  char b[16];
  snprintf(b, sizeof(b), "%s%s", useBar ? "BAR" : "PSI", demoMode ? "  DEMO" : "");
  lv_label_set_text(unitLbl, b);
  lv_obj_set_style_text_color(unitLbl, lv_color_hex(demoMode ? COL_YELLOW : COL_GRAY), 0);
}

void uiMenuRender(int sel, const char* const* items, const char* note) {
  for (int i = 0; i < MENU_N; i++) {
    lv_label_set_text(menuItem[i], items[i]);
    lv_obj_set_style_text_color(menuItem[i], lv_color_hex(i == sel ? COL_WHITE : COL_GRAY), 0);
  }
  lv_obj_set_y(menuHi, 32 + sel * 30);
  lv_label_set_text(menuNote, note);
}

void uiMenuVisible(bool v) {
  if (v) lv_obj_remove_flag(menuBox, LV_OBJ_FLAG_HIDDEN);
  else   lv_obj_add_flag(menuBox, LV_OBJ_FLAG_HIDDEN);
}

void uiPairShow(int slot) {
  lv_label_set_text(pairTitle, LABEL_LONG[slot]);
  lv_obj_remove_flag(pairBox, LV_OBJ_FLAG_HIDDEN);
}

void uiPairHide() { lv_obj_add_flag(pairBox, LV_OBJ_FLAG_HIDDEN); }

// ---- อัปเดตแผง TPMS (สีและเงื่อนไขเหมือนโค้ดเดิม) ----
void refreshPanel(int i) {
  Tire& t = tires[i];
  int st = tireStatus(t);
  uint32_t c = statusColor(st);
  uint32_t vc = (st == ST_HOT || st == ST_BATT) ? COL_GREEN : c;

  lv_obj_set_style_border_color(P[i].box, lv_color_hex(c), 0);
  lv_obj_set_style_bg_color(P[i].tire, lv_color_hex(c), 0);

  lv_label_set_text(P[i].status, STATUS_TITLE[st]);
  lv_obj_set_style_text_color(P[i].status, lv_color_hex(st == ST_OK ? COL_WHITE : c), 0);

  char buf[16];
  if (isnan(t.kpa))  snprintf(buf, sizeof(buf), "--");
  else if (useBar)   snprintf(buf, sizeof(buf), "%.2f", t.kpa / 100.0f);
  else               snprintf(buf, sizeof(buf), "%.1f", t.kpa * 0.145038f);
  lv_label_set_text(P[i].press, buf);
  lv_obj_set_style_text_color(P[i].press, lv_color_hex(vc), 0);

  if (isnan(t.tempC)) snprintf(buf, sizeof(buf), "--");
  else                snprintf(buf, sizeof(buf), "%.0f\xC2\xB0" "C", t.tempC);   // ตามด้วยองศา C
  lv_label_set_text(P[i].temp, buf);
  lv_obj_set_style_text_color(P[i].temp, lv_color_hex(st == ST_HOT ? COL_RED : COL_WHITE), 0);

  uint32_t bc = (t.batt >= 0 && t.batt < BATT_LOW_PCT) ? COL_YELLOW : COL_GRAY;
  lv_obj_set_style_border_color(P[i].battBox, lv_color_hex(bc), 0);
  lv_obj_set_style_bg_color(P[i].battNub, lv_color_hex(bc), 0);
  lv_obj_set_style_bg_color(P[i].battFill, lv_color_hex(bc), 0);
  if (t.batt > 0) {
    int w = constrain(t.batt, 0, 100) * 12 / 100;
    if (w < 1) w = 1;
    lv_obj_set_width(P[i].battFill, w);
    lv_obj_remove_flag(P[i].battFill, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(P[i].battFill, LV_OBJ_FLAG_HIDDEN);
  }

  if (t.batt < 0) snprintf(buf, sizeof(buf), "--");
  else            snprintf(buf, sizeof(buf), "%d%%", t.batt);
  lv_label_set_text(P[i].pct, buf);
  lv_obj_set_style_text_color(P[i].pct, lv_color_hex(bc), 0);

  t.dirty = false;
}

void hideGearParts() {
  for (int i = 0; i < 7; i++) lv_obj_add_flag(seg[i], LV_OBJ_FLAG_HIDDEN);
  lv_obj_t* all[] = { nBarL, nBarR, nDiag, rBar, rTop, rMid, rRight, rLeg };
  for (lv_obj_t* o : all) lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
}

void showGearUI(int g) {
  hideGearParts();
  if (g == -1) {
    lv_obj_t* r[] = { rBar, rTop, rMid, rRight, rLeg };
    for (lv_obj_t* o : r) lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
  } else if (g == 0) {
    lv_obj_t* n[] = { nBarL, nBarR, nDiag };
    for (lv_obj_t* o : n) lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
  } else if (g >= 1 && g <= 5) {
    for (int i = 0; i < 7; i++)
      if (SEG_TABLE[g][i]) lv_obj_remove_flag(seg[i], LV_OBJ_FLAG_HIDDEN);
  }
}

void updateLearnedLabel() {
  int n = __builtin_popcount(learnedMask);
  char b[32];
  snprintf(b, sizeof(b), "LEARNED %d/7%s", n, learnEnabled ? "" : "  LOCK");
  lv_label_set_text(learnedLbl, b);
  lv_obj_set_style_text_color(learnedLbl, lv_color_hex(n == 7 ? COL_GREEN : (n == 0 ? COL_RED : COL_G_YELLOW)), 0);
}

void updateHallValues() {
  static unsigned long last = 0;
  if (millis() - last < PANEL_INTERVAL) return;
  last = millis();
  for (int i = 0; i < NH; i++) lv_label_set_text_fmt(hallVal[i], "%d", hraw[i]);
}

#else   // USE_TFT == 0 : ไม่ใช้จอ ฟังก์ชันหน้าจอทั้งหมดเป็นฟังก์ชันว่าง (เกียร์/ไฟ/TPMS ยังทำงานตามปกติ)

void uiInit() {}
void uiTick() {}
void uiUnitLabel() {}
void uiMenuRender(int, const char* const*, const char*) {}
void uiMenuVisible(bool) {}
void uiPairShow(int) {}
void uiPairHide() {}
void refreshPanel(int) {}
void showGearUI(int) {}
void updateLearnedLabel() {}
void updateHallValues() {}

#endif

/* ========================================================= */
/* ================= GEAR HELPERS ========================== */
/* ========================================================= */

int gearIdx(int g) { return (g == -1) ? 6 : g; }
int idxToGear(int i) { return (i == 6) ? -1 : i; }

int gearColumn(int g) {                // 0 = ซ้าย (1,2) | 1 = กลาง (3,4) | 2 = ขวา (5,R)
  if (g == 1 || g == 2) return 0;
  if (g == 3 || g == 4) return 1;
  return 2;
}

const char* gearName(int g) {
  static const char* const names[] = {"N", "1", "2", "3", "4", "5"};
  if (g == -1) return "R";
  if (g >= 0 && g <= 5) return names[g];
  return "?";
}

int readAvg(int i) {
  long s = 0;
  for (int k = 0; k < AVG_SAMPLES; k++) s += analogRead(hallPin[i]);
  return (int)(s / AVG_SAMPLES);
}

void readHall() {
  for (int i = 0; i < NH; i++) {
    hraw[i] = readAvg(i);
    hf[i] = hf[i] * (1.0f - FILTER_ALPHA) + hraw[i] * FILTER_ALPHA;
  }
}

void loadLearned() {
  prefsGear.begin("gear", true);
  learnedMask = prefsGear.getUChar("mask", 0);
  size_t n = prefsGear.getBytes("vec", vecs, sizeof(vecs));
  learnEnabled = prefsGear.getBool("lrn", true);
  size_t nr = prefsGear.getBytes("runs", runs, sizeof(runs));
  size_t nc = prefsGear.getBytes("rc", runCnt, sizeof(runCnt));
  prefsGear.end();
  if (n != sizeof(vecs)) {
    learnedMask = 0;
    memset(vecs, 0, sizeof(vecs));
  }
  if (nr != sizeof(runs) || nc != sizeof(runCnt)) {      // ยังไม่เคยเก็บแบบหลายรอบ (ค่าเดิมที่เรียนรู้ไว้ใช้ต่อได้)
    memset(runs, 0, sizeof(runs));
    memset(runCnt, 0, sizeof(runCnt));
  }
  for (int i = 0; i < 7; i++) if (runCnt[i] > MAX_RUNS) runCnt[i] = 0;
}

void saveLearned() {
  prefsGear.begin("gear", false);
  prefsGear.putUChar("mask", learnedMask);
  prefsGear.putBytes("vec", vecs, sizeof(vecs));
  prefsGear.end();
}

void saveLearnFlag() {
  prefsGear.begin("gear", false);
  prefsGear.putBool("lrn", learnEnabled);
  prefsGear.end();
}

void saveRuns() {
  prefsGear.begin("gear", false);
  prefsGear.putBytes("runs", runs, sizeof(runs));
  prefsGear.putBytes("rc", runCnt, sizeof(runCnt));
  prefsGear.end();
}

// ค่ากลาง = มัธยฐานของค่าเฉลี่ยทุกรอบ (รอบที่ผิดปกติ 1-2 รอบไม่ดึงค่าไปทั้งหมด) ไม่มีรอบเหลือ = ถือว่ายังไม่เรียนรู้
void recomputeCenter(int idx) {
  int n = runCnt[idx];
  if (n == 0) {
    learnedMask &= ~(1 << idx);
    memset(vecs[idx], 0, sizeof(vecs[idx]));
    return;
  }
  for (int k = 0; k < NH; k++) {
    int a[MAX_RUNS];
    for (int r = 0; r < n; r++) a[r] = runs[idx][r].mean[k];
    for (int i = 1; i < n; i++) {                       // เรียงแบบแทรก (n <= 10)
      int v = a[i], j = i - 1;
      while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; j--; }
      a[j + 1] = v;
    }
    vecs[idx][k] = (n % 2) ? a[n / 2] : (int16_t)((a[n / 2 - 1] + a[n / 2]) / 2);
  }
  learnedMask |= (1 << idx);
}

void commitRuns(int idx) {
  recomputeCenter(idx);
  saveLearned();
  saveRuns();
  updateLearnedLabel();
}

void addRun(int idx, const Run& r) {
  if (runCnt[idx] >= MAX_RUNS) {
    memmove(&runs[idx][0], &runs[idx][1], sizeof(Run) * (MAX_RUNS - 1));
    runCnt[idx] = MAX_RUNS - 1;
  }
  runs[idx][runCnt[idx]++] = r;
  commitRuns(idx);
}

void removeRun(int idx, int ri) {
  if (ri < 0 || ri >= runCnt[idx]) return;
  for (int r = ri; r < runCnt[idx] - 1; r++) runs[idx][r] = runs[idx][r + 1];
  runCnt[idx]--;
  commitRuns(idx);
}

void clearGear(int idx) {
  runCnt[idx] = 0;
  memset(runs[idx], 0, sizeof(runs[idx]));
  commitRuns(idx);
}

/* ===== เก็บค่าแบบต่อเนื่องหลายวินาที (ทำในลูปหลัก ไม่บล็อก ใช้ได้ตอนขับจริง) ===== */
struct Cap {
  bool on;
  int idx;
  uint32_t t0, dur, n;
  double sum[NH];
  int mn[NH], mx[NH];
  uint16_t seq;        // นับจำนวนรอบที่เก็บเสร็จ (ให้หน้าเว็บรู้ว่ามีรอบใหม่)
  bool lastOk;
  int lastIdx;
};
Cap cap = {};

bool capStart(int idx, uint32_t sec) {
  if (cap.on || !learnEnabled || idx < 0 || idx > 6) return false;
  if (sec < 3) sec = 3;
  if (sec > 30) sec = 30;
  cap.on = true;
  cap.idx = idx;
  cap.t0 = millis();
  cap.dur = sec * 1000UL;
  cap.n = 0;
  for (int k = 0; k < NH; k++) { cap.sum[k] = 0; cap.mn[k] = 99999; cap.mx[k] = -1; }
  return true;
}

void capCancel() { cap.on = false; }

void capSample() {
  if (!cap.on) return;
  for (int k = 0; k < NH; k++) {
    int v = hraw[k];
    cap.sum[k] += v;
    if (v < cap.mn[k]) cap.mn[k] = v;
    if (v > cap.mx[k]) cap.mx[k] = v;
  }
  cap.n++;
  if (millis() - cap.t0 >= cap.dur) {
    cap.on = false;
    cap.lastIdx = cap.idx;
    cap.lastOk = cap.n >= 20;
    if (cap.lastOk) {
      Run r;
      for (int k = 0; k < NH; k++) {
        r.mean[k] = (int16_t)(cap.sum[k] / cap.n + 0.5);
        r.lo[k] = (int16_t)cap.mn[k];
        r.hi[k] = (int16_t)cap.mx[k];
      }
      addRun(cap.idx, r);
    }
    cap.seq++;
  }
}

Match matchGear() {
  Match m;
  m.valid = false;
  m.gear = 0;
  m.d1 = 1e9f;
  m.gear2 = 0;
  m.d2 = 1e9f;

  for (int idx = 0; idx < 7; idx++) {
    if (!(learnedMask & (1 << idx))) continue;
    float d = 0;
    for (int i = 0; i < NH; i++) {
      float diff = hf[i] - vecs[idx][i];
      d += diff * diff;
    }
    d = sqrtf(d);
    if (d < m.d1) {
      m.d2 = m.d1;
      m.gear2 = m.gear;
      m.d1 = d;
      m.gear = idxToGear(idx);
      m.valid = true;
    } else if (d < m.d2) {
      m.d2 = d;
      m.gear2 = idxToGear(idx);
    }
  }
  return m;
}

/* ========================================================= */
/* ================= LIGHT SENSOR + WS2812B ================ */
/* ========================================================= */

int readLight() {
#if LDR_ANALOG
  int v = analogRead(LDR_PIN);
#if LDR_INVERT
  v = 4095 - v;
#endif
  return v;
#else
  bool bright = digitalRead(LDR_PIN) ? true : false;
#if LDR_INVERT
  bright = !bright;
#endif
  return bright ? 4095 : 0;
#endif
}

uint8_t brightFromLight(float l) {
  float t = (l - LDR_DARK_VAL) / (float)(LDR_BRIGHT_VAL - LDR_DARK_VAL);
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  return (uint8_t)(LED_BRIGHT_MIN + t * (LED_BRIGHT_MAX - LED_BRIGHT_MIN));
}

#if USE_LED
const bool LED_SEGMENT[6][7] = {
  {0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1}
};

void renderLED(int g) {
  strip.setBrightness(ledBright);
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, 0);

  if (g == -1) {                      // R = ฟ้า
    static const int s[] = {0, 1, 2, 4, 5, 6};
    for (int i : s) strip.setPixelColor(i, strip.Color(0, 150, 255));
  } else if (g == 0) {                // N = เขียว
    static const int s[] = {0, 1, 2, 4, 5};
    for (int i : s) strip.setPixelColor(i, strip.Color(0, 255, 0));
  } else if (g >= 1 && g <= 5) {      // 1-3 = แดง, 4-5 = ม่วง
    uint32_t c = (g <= 3) ? strip.Color(255, 0, 0) : strip.Color(180, 0, 255);
    for (int i = 0; i < 7; i++)
      if (LED_SEGMENT[g][i]) strip.setPixelColor(i, c);
  }
  strip.show();
}
#endif

bool wifiStart();
void wifiStop();
extern bool wifiOn;        // ตอนเปิด WiFi ขา LDR (ADC2) อ่านไม่ได้ จึงค้างความสว่างไว้
void updateBrightness() {
  if (wifiOn) return;
  static unsigned long last = 0;
  if (millis() - last < 50) return;
  last = millis();

  lightF = lightF * 0.9f + readLight() * 0.1f;
  int b = brightFromLight(lightF);

  int diff = b - (int)ledBright;
  if (diff < 0) diff = -diff;
  if (diff >= 4 || ((b == LED_BRIGHT_MIN || b == LED_BRIGHT_MAX) && b != (int)ledBright)) {
    ledBright = (uint8_t)b;
#if USE_LED
    renderLED(currentGear);
#endif
  }
}

void showGear(int g) {
  showGearUI(g);
#if USE_LED
  renderLED(g);
#endif
}

/* ========================================================= */
/* ================= TPMS: SETTINGS + PAIRING ============== */
/* ========================================================= */

void loadSettings() {
  prefsTpms.begin("tpms", false);
#if RESET_SETTINGS
  prefsTpms.clear();
#endif
  LOW_KPA = prefsTpms.getFloat("lo", DEF_LOW);
  WARN_KPA = prefsTpms.getFloat("wa", DEF_WARN);
  HIGH_KPA = prefsTpms.getFloat("hi", DEF_HIGH);
  useBar = prefsTpms.getBool("bar", false);
  demoMode = prefsTpms.getBool("demo", DEMO_DEFAULT);
  for (int i = 0; i < 4; i++) {
    char k[4];
    snprintf(k, sizeof(k), "m%d", i);
    String s = prefsTpms.getString(k, "");
    strncpy(tires[i].mac, s.c_str(), sizeof(tires[i].mac) - 1);
  }
}

void saveThresholds() {
  prefsTpms.putFloat("lo", LOW_KPA);
  prefsTpms.putFloat("wa", WARN_KPA);
  prefsTpms.putFloat("hi", HIGH_KPA);
}

void savePairs() {
  for (int i = 0; i < 4; i++) {
    char k[4];
    snprintf(k, sizeof(k), "m%d", i);
    prefsTpms.putString(k, tires[i].mac);
  }
}

/* ================= ปุ่ม MENU / NEXT / OK + เมนู ================= */
enum { B_MENU, B_NEXT, B_OK, NBTN };
const uint8_t btnPin[NBTN] = { BTN_MENU, BTN_NEXT, BTN_OK };
bool btnRaw[NBTN] = { false, false, false };
bool btnStable[NBTN] = { false, false, false };
uint32_t btnT[NBTN] = { 0, 0, 0 };

bool menuOpen = false;
int  menuSel = 0;
char menuMsg[40] = "";

void menuClose() {
  menuOpen = false;
  uiMenuVisible(false);
}

// เริ่มจับคู่เซนเซอร์ (ได้เฉพาะโหมด REAL) คืน false ถ้าเริ่มไม่ได้
bool startPairing() {
  if (demoMode) return false;
  if (menuOpen) menuClose();
  for (int i = 0; i < 4; i++) {
    strncpy(oldMac[i], tires[i].mac, sizeof(oldMac[i]) - 1);
    oldMac[i][sizeof(oldMac[i]) - 1] = 0;
    tires[i].mac[0] = 0;
    tires[i].kpa = NAN;
    tires[i].tempC = NAN;
    tires[i].batt = -1;
    tires[i].alarm = false;
    tires[i].last = 0;
    tires[i].dirty = true;
  }
  pairingSlot = 0;
  pairUiDirty = true;
  return true;
}

void cancelPairing() {
  pairingSlot = -1;
  pairUiDirty = false;
  needSave = false;
  for (int i = 0; i < 4; i++) {
    strncpy(tires[i].mac, oldMac[i], sizeof(tires[i].mac) - 1);
    tires[i].mac[sizeof(tires[i].mac) - 1] = 0;
    tires[i].dirty = true;
  }
  savePairs();
  uiPairHide();
}

void setUnit(bool bar) {
  useBar = bar;
  prefsTpms.putBool("bar", useBar);
  uiUnitLabel();
  for (int i = 0; i < 4; i++) tires[i].dirty = true;
}

void menuRefresh() {
  static char l1[24], l2[24];
  snprintf(l1, sizeof(l1), "Unit: %s", useBar ? "BAR" : "PSI");
  snprintf(l2, sizeof(l2), "Mode: %s", demoMode ? "DEMO" : "REAL");
  static char l3[24];
  snprintf(l3, sizeof(l3), "WiFi: %s", wifiOn ? "ON" : "OFF");
  const char* items[MENU_N] = { "Pair TPMS sensors", l1, l2, l3, "Close" };
  uiMenuRender(menuSel, items, menuMsg);
}

void menuOpenNow() {
  menuOpen = true;
  menuSel = 0;
  snprintf(menuMsg, sizeof(menuMsg), "NEXT=move  OK=select  MENU=close");
  menuRefresh();
  uiMenuVisible(true);
}

// เปลี่ยนโหมด DEMO/REAL แล้วรีสตาร์ท (จำค่าไว้ในบอร์ด ค่าเกียร์ที่เรียนรู้ไม่หาย)
void setModeAndRestart(bool demo) {
  prefsTpms.putBool("demo", demo);
#if DEBUG_MODE == 1
  Serial.println(demo ? ">> โหมด DEMO (ข้อมูลจำลอง) กำลังรีสตาร์ท..." : ">> โหมด REAL (เซนเซอร์จริง) กำลังรีสตาร์ท...");
#endif
  snprintf(menuMsg, sizeof(menuMsg), "Restarting...");
  if (menuOpen) menuRefresh();
  uiTick();
  delay(400);
  ESP.restart();
}

void menuActivate() {
  switch (menuSel) {
    case 0:
      if (demoMode) {
        snprintf(menuMsg, sizeof(menuMsg), "DEMO mode: set Mode to REAL first");
        menuRefresh();
      } else {
        startPairing();      // ปิดเมนูให้เอง
      }
      break;
    case 1:
      setUnit(!useBar);
      menuMsg[0] = 0;
      menuRefresh();
      break;
    case 2:
      setModeAndRestart(!demoMode);
      break;
    case 3:
      if (wifiOn) {
        wifiStop();
        menuMsg[0] = 0;
      } else if (!wifiStart()) {
        snprintf(menuMsg, sizeof(menuMsg), "WiFi: not enough memory");
      } else {
        snprintf(menuMsg, sizeof(menuMsg), "%s / %s  192.168.4.1", WIFI_SSID, WIFI_PASS);
      }
      menuRefresh();
      break;
    default:
      menuClose();
      break;
  }
}

// คืนค่าปุ่มที่ "เพิ่งถูกกด" (B_MENU/B_NEXT/B_OK) หรือ -1 ถ้าไม่มี (กันสั่น 30 ms)
int pollButtons() {
  int ev = -1;
  uint32_t now = millis();
  for (int i = 0; i < NBTN; i++) {
    bool raw = (digitalRead(btnPin[i]) == LOW);
    if (raw != btnRaw[i]) {
      btnRaw[i] = raw;
      btnT[i] = now;
    }
    if (now - btnT[i] >= 30 && raw != btnStable[i]) {
      if (raw && ev >= 0) continue;       // มีอีกปุ่มถูกกดรอบเดียวกัน เก็บไว้รอบหน้า
      btnStable[i] = raw;
      if (raw) ev = i;
    }
  }
  return ev;
}

void onButton(int b) {
  if (pairingSlot >= 0) {                 // ระหว่างจับคู่
    if (b == B_NEXT) {
      pairingSlot = pairingSlot + 1;      // ข้ามล้อนี้
      pairUiDirty = true;
    } else if (b == B_MENU) {
      cancelPairing();
    }
    return;
  }
  if (menuOpen) {
    if (b == B_MENU) {
      menuClose();
    } else if (b == B_NEXT) {
      menuSel = (menuSel + 1) % MENU_N;
      menuMsg[0] = 0;
      menuRefresh();
    } else {
      menuActivate();
    }
    return;
  }
  if (b == B_MENU) menuOpenNow();
  else if (b == B_NEXT) setUnit(!useBar);  // ทางลัด: สลับ psi/bar
}

/* ========================================================= */
/* ================= SERIAL COMMANDS ======================= */
/* ========================================================= */

float kpaToUnit(float kpa) { return useBar ? kpa / 100.0f : kpa * 0.145038f; }
float unitToKpa(float v)   { return useBar ? v * 100.0f : v / 0.145038f; }

/* ================= เรียนรู้เกียร์ (ใช้ทั้งจาก Serial และหน้าเว็บ) ================= */
LearnResult learnGearCore(int g) {
  int idx = gearIdx(g);
  long sum[NH];
  int mn[NH], mx[NH];
  for (int i = 0; i < NH; i++) {
    sum[i] = 0;
    mn[i] = 99999;
    mx[i] = -1;
  }

  for (int s = 0; s < LEARN_SAMPLES; s++) {
    for (int i = 0; i < NH; i++) {
      int v = readAvg(i);
      sum[i] += v;
      if (v < mn[i]) mn[i] = v;
      if (v > mx[i]) mx[i] = v;
    }
    uiTick();          // ให้จอยังรีเฟรชระหว่างเก็บค่า
    delay(5);
  }

  LearnResult r;
  Run run;
  for (int i = 0; i < NH; i++) {
    run.mean[i] = (int16_t)(sum[i] / LEARN_SAMPLES);
    run.lo[i] = (int16_t)mn[i];
    run.hi[i] = (int16_t)mx[i];
    r.spread[i] = mx[i] - mn[i];
  }
  runCnt[idx] = 0;                 // คำสั่ง Serial = เรียนรู้ใหม่ทับ (เหลือ 1 รอบ) ส่วนหน้าเว็บเพิ่มทีละรอบ
  addRun(idx, run);
  for (int i = 0; i < NH; i++) hf[i] = vecs[idx][i];

  r.nearIdx = -1;
  r.nearDist = 1e9f;
  for (int o = 0; o < 7; o++) {
    if (o == idx || !(learnedMask & (1 << o))) continue;
    float d = 0;
    for (int i = 0; i < NH; i++) {
      float diff = (float)vecs[idx][i] - vecs[o][i];
      d += diff * diff;
    }
    d = sqrtf(d);
    if (d < r.nearDist) {
      r.nearDist = d;
      r.nearIdx = o;
    }
  }
  return r;
}

void clearLearnedCore() {
  learnedMask = 0;
  memset(vecs, 0, sizeof(vecs));
  memset(runs, 0, sizeof(runs));
  memset(runCnt, 0, sizeof(runCnt));
  saveLearned();
  saveRuns();
  updateLearnedLabel();
}

#if DEBUG_MODE == 1

void printSettings() {
  const char* u = useBar ? "bar" : "psi";
  Serial.printf("lo   = %.2f %s\nwarn = %.2f %s\nhi   = %.2f %s\n",
                kpaToUnit(LOW_KPA), u, kpaToUnit(WARN_KPA), u, kpaToUnit(HIGH_KPA), u);
  Serial.printf("mode = %s\n", demoMode ? "DEMO" : "REAL");
}

void printHelp() {
  Serial.println(">> เกียร์ (ตัวอักษรเดียว): n 1 2 3 4 5 r = เรียนรู้เกียร์นั้น (เข้าเกียร์ รอ 1-2 วินาที แล้วพิมพ์)");
  Serial.println("   p = ดูค่าตอนนี้ | l = ดูตาราง | b = ดูค่า LDR | s = พิมพ์ต่อเนื่อง | x = ลบค่าที่เรียนรู้");
  Serial.println("   learn on / learn off = เปิด/ล็อกการเรียนรู้เกียร์ (ล็อกไว้กันพิมพ์พลาดทับค่า)");
  Serial.println(">> TPMS (พิมพ์ทั้งคำ): show | lo <ค่า> | warn <ค่า> | hi <ค่า> | reset");
  Serial.println("   pair = จับคู่เซนเซอร์ | unit psi / unit bar | mode demo / mode real (รีสตาร์ทเอง)");
  Serial.println("   wifi on / wifi off = เปิด/ปิด WiFi สำหรับหน้าเว็บ (หรือ MENU -> WiFi บนจอ)");
}

void printTable() {
  Serial.println("เกียร์   H0(บน)  H1(กลาง)  H2(ล่าง)");
  for (int idx = 0; idx < 7; idx++) {
    const char* nm = gearName(idxToGear(idx));
    if (learnedMask & (1 << idx)) {
      Serial.printf("  %s      %4d     %4d      %4d\n", nm, vecs[idx][0], vecs[idx][1], vecs[idx][2]);
    } else {
      Serial.printf("  %s      (ยังไม่ได้เรียนรู้)\n", nm);
    }
  }
}

void printNow() {
  Serial.printf("NOW  H0=%4d H1=%4d H2=%4d", hraw[0], hraw[1], hraw[2]);
  Match m = matchGear();
  if (!m.valid) {
    Serial.println(" | ยังไม่มีเกียร์ที่เรียนรู้");
    return;
  }
  Serial.printf(" | ใกล้สุด=%s (ห่าง %.0f)", gearName(m.gear), m.d1);
  if (m.d2 < 1e8f) Serial.printf("  รองลงมา=%s (ห่าง %.0f)", gearName(m.gear2), m.d2);
  Serial.printf(" | แสดง=%s\n", gearName(currentGear));
}

void printLight() {
  Serial.printf("LIGHT raw=%d filtered=%.0f -> brightness=%d/255 (%d%%) | มืด=%d สว่าง=%d\n",
                readLight(), lightF, ledBright, (ledBright * 100) / 255, LDR_DARK_VAL, LDR_BRIGHT_VAL);
}

void learnGear(int g) {
  LearnResult r = learnGearCore(g);
  int idx = gearIdx(g);
  Serial.printf(">> เรียนรู้ %s แล้ว: H0=%d H1=%d H2=%d | แกว่ง(สูง-ต่ำ) %d %d %d\n",
                gearName(g), vecs[idx][0], vecs[idx][1], vecs[idx][2],
                r.spread[0], r.spread[1], r.spread[2]);
  int spreadMax = r.spread[0];
  if (r.spread[1] > spreadMax) spreadMax = r.spread[1];
  if (r.spread[2] > spreadMax) spreadMax = r.spread[2];
  if (spreadMax > 20) Serial.println("   คำเตือน: ค่าแกว่งมาก (คันเกียร์ยังขยับอยู่หรือเปล่า) ลองรอให้นิ่งแล้วพิมพ์ซ้ำ");
  if (r.nearIdx >= 0) {
    if (r.nearDist < MIN_SEP) {
      Serial.printf("   คำเตือน: ใกล้เกียร์ %s แค่ %.0f (ควรเกิน %.0f) อาจแยกไม่ออก ต้องปรับตำแหน่งแม่เหล็กหรือฮอลล์\n",
                    gearName(idxToGear(r.nearIdx)), r.nearDist, MIN_SEP);
    } else {
      Serial.printf("   OK ห่างจากเกียร์ที่ใกล้สุด (%s) %.0f\n", gearName(idxToGear(r.nearIdx)), r.nearDist);
    }
  }
  Serial.printf("   เรียนรู้แล้ว %d/7 เกียร์\n", __builtin_popcount(learnedMask));
}

void clearLearned() {
  clearLearnedCore();
  Serial.println(">> ลบค่าที่เรียนรู้ทั้งหมดแล้ว");
}

void runTpmsCommand(char* cmd) {
  char name[12] = "", arg[12] = "";
  int n = sscanf(cmd, "%11s %11s", name, arg);
  if (n < 1) return;
  float v = (n == 2) ? (float)atof(arg) : 0.0f;
  if (strcmp(name, "show") == 0) {
    printSettings();
  } else if (strcmp(name, "reset") == 0) {
    prefsTpms.clear();
    Serial.println("cleared, restarting");
    delay(200);
    ESP.restart();
  } else if (strcmp(name, "pair") == 0) {
    if (startPairing()) Serial.println(">> เริ่มจับคู่: ถือเซนเซอร์ล้อหน้าซ้ายไว้ใกล้บอร์ด | NEXT = ข้ามล้อ | MENU = ยกเลิก");
    else Serial.println(">> ตอนนี้เป็นโหมด DEMO จับคู่ไม่ได้ ให้พิมพ์ mode real ก่อน");
  } else if (strcmp(name, "wifi") == 0) {
    if (n == 2 && !strcasecmp(arg, "on")) {
      if (!wifiStart()) Serial.println(">> เปิด WiFi ไม่ได้ (หน่วยความจำไม่พอ หรือเริ่มไม่สำเร็จ)");
    } else if (n == 2 && !strcasecmp(arg, "off")) {
      wifiStop();
    } else {
      Serial.printf(">> WiFi = %s (เปลี่ยน: wifi on | wifi off) หน่วยความจำว่าง %u\n", wifiOn ? "เปิด" : "ปิด", (unsigned)ESP.getFreeHeap());
    }
  } else if (strcmp(name, "learn") == 0) {
    if (n == 2 && !strcasecmp(arg, "on")) {
      learnEnabled = true;
      saveLearnFlag();
      updateLearnedLabel();
      Serial.println(">> เปิดการเรียนรู้เกียร์แล้ว (พิมพ์ n 1 2 3 4 5 r) เสร็จแล้วพิมพ์ learn off เพื่อล็อก");
    } else if (n == 2 && !strcasecmp(arg, "off")) {
      learnEnabled = false;
      saveLearnFlag();
      updateLearnedLabel();
      Serial.println(">> ล็อกการเรียนรู้เกียร์แล้ว ค่าที่เรียนรู้ไว้จะไม่ถูกทับ (พิมพ์ learn on เพื่อเปิดกลับ)");
    } else {
      Serial.printf(">> การเรียนรู้เกียร์ = %s (เปลี่ยน: learn on | learn off)\n", learnEnabled ? "เปิด" : "ล็อก");
    }
  } else if (strcmp(name, "unit") == 0) {
    if (n == 2 && !strcasecmp(arg, "psi")) setUnit(false);
    else if (n == 2 && !strcasecmp(arg, "bar")) setUnit(true);
    else setUnit(!useBar);
    Serial.printf(">> หน่วย = %s\n", useBar ? "bar" : "psi");
  } else if (strcmp(name, "mode") == 0) {
    if (n == 2 && !strcasecmp(arg, "demo")) setModeAndRestart(true);
    else if (n == 2 && !strcasecmp(arg, "real")) setModeAndRestart(false);
    else Serial.printf(">> โหมดตอนนี้ = %s (เปลี่ยน: mode demo | mode real)\n", demoMode ? "DEMO" : "REAL");
  } else if (n == 2 && (strcmp(name, "lo") == 0 || strcmp(name, "warn") == 0 || strcmp(name, "hi") == 0)) {
    float kpa = unitToKpa(v);
    float lo = LOW_KPA, wa = WARN_KPA, hi = HIGH_KPA;
    if (strcmp(name, "lo") == 0) lo = kpa;
    else if (strcmp(name, "warn") == 0) wa = kpa;
    else hi = kpa;
    if (lo > 0 && lo < wa && wa < hi && hi < 1000) {
      LOW_KPA = lo; WARN_KPA = wa; HIGH_KPA = hi;
      saveThresholds();
      for (int i = 0; i < 4; i++) tires[i].dirty = true;
      Serial.println("saved");
      printSettings();
    } else {
      Serial.println("ไม่รับค่า: ต้องเรียง lo < warn < hi");
    }
  } else {
    printHelp();
  }
}

void processCommand(char* s) {
  // ตัดช่องว่างหัวท้าย
  while (*s == ' ') s++;
  int len = strlen(s);
  while (len > 0 && s[len - 1] == ' ') s[--len] = 0;
  if (len == 0) return;

  // คำสั่ง TPMS (ต้องตรงทั้งคำ): show | lo | warn | hi | reset | pair | unit | mode
  char word[12] = "";
  sscanf(s, "%11s", word);
  if (!strcmp(word, "show") || !strcmp(word, "lo") || !strcmp(word, "warn") ||
      !strcmp(word, "hi") || !strcmp(word, "reset") || !strcmp(word, "pair") ||
      !strcmp(word, "unit") || !strcmp(word, "mode") || !strcmp(word, "learn") || !strcmp(word, "wifi")) {
    runTpmsCommand(s);
    return;
  }

  if (len > 1) {            // คำอื่นที่ไม่รู้จัก ไม่ตีความเป็นคำสั่งเกียร์ (กันพิมพ์พลาด เช่น "restart" ไปเรียนรู้ R)
    printHelp();
    return;
  }

  char c = tolower(s[0]);   // ตัวอักษรเดียว = คำสั่งเกียร์
  if (!learnEnabled && (c == 'n' || c == '0' || (c >= '1' && c <= '5') || c == 'r' || c == 'x')) {
    Serial.println(">> การเรียนรู้เกียร์ถูกล็อกอยู่ พิมพ์ learn on เพื่อเปิดก่อน");
    return;
  }
  if (c == 'n' || c == '0') learnGear(0);
  else if (c >= '1' && c <= '5') learnGear(c - '0');
  else if (c == 'r') learnGear(-1);
  else if (c == 'p') printNow();
  else if (c == 'l') printTable();
  else if (c == 'b') printLight();
  else if (c == 's') {
    streamOn = !streamOn;
    Serial.println(streamOn ? ">> STREAM ON (พิมพ์ต่อเนื่อง)" : ">> STREAM OFF");
  } else if (c == 'x') clearLearned();
  else printHelp();
}

// ประมวลผลเมื่อเจอขึ้นบรรทัดใหม่ หรือไม่มีข้อมูลมาเพิ่ม 300 ms (รองรับแอพ Serial USB Terminal)
void handleSerial() {
  static char buf[32];
  static uint8_t n = 0;
  static unsigned long lastChar = 0;

  while (Serial.available()) {
    char ch = Serial.read();
    lastChar = millis();
    if (ch == '\n' || ch == '\r') {
      if (n) { buf[n] = 0; processCommand(buf); n = 0; }
    } else if (n < sizeof(buf) - 1) {
      buf[n++] = ch;
    }
  }

  if (n && millis() - lastChar > 300) {
    buf[n] = 0;
    processCommand(buf);
    n = 0;
  }
}

#endif

/* ========================================================= */
/* ================= WiFi + หน้าเว็บ ======================== */
/* ========================================================= */
// บอร์ดปล่อย WiFi ชื่อ WIFI_SSID (ไม่ต่ออินเทอร์เน็ต) มือถือเชื่อมแล้วเปิด http://192.168.4.1
// หน้าเว็บ (web_ui.h) ใช้ API: GET /api/state, POST /api/learn /clear /lock /unit /mode /tpms /pair /wifi

WebServer server(80);
bool wifiOn = false;
bool wifiStopReq = false;
uint32_t wifiLastAct = 0;

void wifiTouch() { wifiLastAct = millis(); }

bool wifiStart() {
  if (wifiOn) return true;
  if (ESP.getFreeHeap() < WIFI_MIN_HEAP) return false;
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(WIFI_SSID, WIFI_PASS, 6, 0, 2)) {      // ช่อง 6, ไม่ซ่อนชื่อ, ต่อได้ 2 เครื่อง
    WiFi.mode(WIFI_OFF);
    return false;
  }
  server.begin();
  wifiOn = true;
  wifiStopReq = false;
  wifiTouch();
#if DEBUG_MODE == 1
  Serial.printf(">> WiFi เปิด: %s / %s  http://%s\n", WIFI_SSID, WIFI_PASS, WiFi.softAPIP().toString().c_str());
#endif
  return true;
}

void wifiStop() {
  if (!wifiOn) return;
  server.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiOn = false;
  wifiStopReq = false;
  if (menuOpen) menuRefresh();
#if DEBUG_MODE == 1
  Serial.println(">> WiFi ปิดแล้ว");
#endif
}

static void sendJson(const String& j) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", j);
}

static void jNum(String& j, float v, int dec) {
  if (isnan(v)) { j += "null"; return; }
  char b[20];
  snprintf(b, sizeof(b), "%.*f", dec, v);
  j += b;
}

void hRoot() {
  wifiTouch();
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html; charset=utf-8", WEB_UI, sizeof(WEB_UI) - 1);
}

void hState() {
  String j;
  j.reserve(1800);
  char b[360];
  j += "{\"h\":[";
  for (int i = 0; i < NH; i++) { snprintf(b, sizeof(b), "%s%d", i ? "," : "", (int)(hf[i] + 0.5f)); j += b; }
  j += "],\"g\":";
  if (currentGear == 999) j += "null"; else { snprintf(b, sizeof(b), "%d", currentGear); j += b; }
  snprintf(b, sizeof(b), ",\"m\":%u,\"lrn\":%s,\"v\":[", (unsigned)learnedMask, learnEnabled ? "true" : "false");
  j += b;
  for (int g = 0; g < 7; g++) {
    snprintf(b, sizeof(b), "%s[%d,%d,%d]", g ? "," : "", vecs[g][0], vecs[g][1], vecs[g][2]);
    j += b;
  }
  int ps = pairingSlot;
  if (ps >= 4) ps = -1;
  snprintf(b, sizeof(b), "],\"rc\":[%d,%d,%d,%d,%d,%d,%d],\"ln\":{\"on\":%d,\"i\":%d,\"t\":%lu,\"d\":%lu,\"n\":%lu,\"seq\":%u,\"ok\":%d,\"li\":%d,\"lo\":[%d,%d,%d],\"hi\":[%d,%d,%d]}",
           runCnt[0], runCnt[1], runCnt[2], runCnt[3], runCnt[4], runCnt[5], runCnt[6],
           cap.on ? 1 : 0, cap.idx, cap.on ? (unsigned long)(millis() - cap.t0) : 0UL, (unsigned long)cap.dur, (unsigned long)cap.n,
           (unsigned)cap.seq, cap.lastOk ? 1 : 0, cap.lastIdx,
           cap.on && cap.n ? cap.mn[0] : 0, cap.on && cap.n ? cap.mn[1] : 0, cap.on && cap.n ? cap.mn[2] : 0,
           cap.on && cap.n ? cap.mx[0] : 0, cap.on && cap.n ? cap.mx[1] : 0, cap.on && cap.n ? cap.mx[2] : 0);
  j += b;
  snprintf(b, sizeof(b), ",\"tp\":{\"demo\":%s,\"bar\":%s,\"lo\":", demoMode ? "true" : "false", useBar ? "true" : "false");
  j += b;
  jNum(j, LOW_KPA, 1); j += ",\"wa\":"; jNum(j, WARN_KPA, 1); j += ",\"hi\":"; jNum(j, HIGH_KPA, 1);
  snprintf(b, sizeof(b), ",\"pair\":%d,\"tires\":[", ps);
  j += b;
  for (int i = 0; i < 4; i++) {
    const Tire& t = tires[i];
    if (i) j += ",";
    j += "{\"k\":"; jNum(j, t.kpa, 1);
    j += ",\"c\":"; jNum(j, t.tempC, 1);
    snprintf(b, sizeof(b), ",\"b\":%d,\"s\":%d,\"m\":\"%s\",\"a\":%lu}", t.batt, tireStatus(t), t.mac,
             t.last ? (unsigned long)((millis() - t.last) / 1000) : 0UL);
    j += b;
  }
  long remain = (long)(WIFI_IDLE_MS - (millis() - wifiLastAct)) / 1000;
  if (remain < 0) remain = 0;
  snprintf(b, sizeof(b), "]},\"sys\":{\"ssid\":\"%s\",\"pw\":\"%s\",", WIFI_SSID, WIFI_PASS);
  j += b;
  snprintf(b, sizeof(b), "\"ip\":\"192.168.4.1\",\"cl\":%d,\"off\":%ld,\"heap\":%u,\"up\":%lu,\"ver\":\"%s\"}}",
           WiFi.softAPgetStationNum(), remain, (unsigned)ESP.getFreeHeap(), millis() / 1000UL, FW_VERSION);
  j += b;
  sendJson(j);
}

void hLearn() {              // เริ่มเก็บค่า 1 รอบ (ต่อเนื่อง s วินาที) เพิ่มเข้าชุดข้อมูลของเกียร์นั้น
  wifiTouch();
  if (!learnEnabled) { sendJson("{\"ok\":false,\"err\":\"locked\"}"); return; }
  int i = server.arg("i").toInt();
  int sec = server.hasArg("s") ? server.arg("s").toInt() : CAP_DEFAULT_S;
  if (i < 0 || i > 6) { sendJson("{\"ok\":false,\"err\":\"index\"}"); return; }
  if (!capStart(i, sec)) { sendJson("{\"ok\":false,\"err\":\"busy\"}"); return; }
  sendJson("{\"ok\":true}");
}

void hLearnCancel() {
  wifiTouch();
  capCancel();
  sendJson("{\"ok\":true}");
}

void hGears() {              // สรุปต่อเกียร์: จำนวนรอบ, ค่ากลาง, ช่วงต่ำสุด-สูงสุดรวมทุกรอบ
  String j;
  j.reserve(900);
  char b[160];
  j += "{\"g\":[";
  for (int g = 0; g < 7; g++) {
    int mn[NH], mx[NH];
    for (int k = 0; k < NH; k++) {
      mn[k] = vecs[g][k];
      mx[k] = vecs[g][k];
      for (int r = 0; r < runCnt[g]; r++) {
        if (runs[g][r].lo[k] < mn[k]) mn[k] = runs[g][r].lo[k];
        if (runs[g][r].hi[k] > mx[k]) mx[k] = runs[g][r].hi[k];
      }
    }
    snprintf(b, sizeof(b), "%s{\"n\":%d,\"c\":[%d,%d,%d],\"mn\":[%d,%d,%d],\"mx\":[%d,%d,%d]}", g ? "," : "",
             runCnt[g], vecs[g][0], vecs[g][1], vecs[g][2], mn[0], mn[1], mn[2], mx[0], mx[1], mx[2]);
    j += b;
  }
  j += "]}";
  sendJson(j);
}

void hRuns() {               // ข้อมูลทุกรอบของเกียร์ที่ขอ: [เฉลี่ย x3, ต่ำสุด x3, สูงสุด x3]
  int i = server.arg("i").toInt();
  if (i < 0 || i > 6) { sendJson("{\"ok\":false}"); return; }
  String j;
  j.reserve(500);
  char b[100];
  snprintf(b, sizeof(b), "{\"i\":%d,\"r\":[", i);
  j += b;
  for (int r = 0; r < runCnt[i]; r++) {
    const Run& u = runs[i][r];
    snprintf(b, sizeof(b), "%s[%d,%d,%d,%d,%d,%d,%d,%d,%d]", r ? "," : "", u.mean[0], u.mean[1], u.mean[2],
             u.lo[0], u.lo[1], u.lo[2], u.hi[0], u.hi[1], u.hi[2]);
    j += b;
  }
  j += "]}";
  sendJson(j);
}

void hRunDel() {
  wifiTouch();
  if (!learnEnabled) { sendJson("{\"ok\":false,\"err\":\"locked\"}"); return; }
  int i = server.arg("i").toInt();
  if (i < 0 || i > 6) { sendJson("{\"ok\":false}"); return; }
  removeRun(i, server.arg("r").toInt());
  sendJson("{\"ok\":true}");
}

void hGearClear() {
  wifiTouch();
  if (!learnEnabled) { sendJson("{\"ok\":false,\"err\":\"locked\"}"); return; }
  int i = server.arg("i").toInt();
  if (i < 0 || i > 6) { sendJson("{\"ok\":false}"); return; }
  clearGear(i);
  sendJson("{\"ok\":true}");
}

void hClear() {
  wifiTouch();
  if (!learnEnabled) { sendJson("{\"ok\":false,\"err\":\"locked\"}"); return; }
  clearLearnedCore();
  sendJson("{\"ok\":true}");
}

void hLock() {
  wifiTouch();
  learnEnabled = server.arg("on") == "1";
  saveLearnFlag();
  updateLearnedLabel();
  sendJson("{\"ok\":true}");
}

void hUnit() {
  wifiTouch();
  setUnit(server.arg("bar") == "1");
  sendJson("{\"ok\":true}");
}

void hMode() {
  wifiTouch();
  bool demo = server.arg("demo") == "1";
  if (demo == demoMode) { sendJson("{\"ok\":true}"); return; }
  sendJson("{\"ok\":true,\"restart\":true}");
  delay(150);
  setModeAndRestart(demo);
}

void hTpms() {
  wifiTouch();
  float lo = server.arg("lo").toFloat(), wa = server.arg("wa").toFloat(), hi = server.arg("hi").toFloat();
  if (!(lo > 0 && lo < wa && wa < hi && hi < 1000)) { sendJson("{\"ok\":false,\"err\":\"order\"}"); return; }
  LOW_KPA = lo; WARN_KPA = wa; HIGH_KPA = hi;
  saveThresholds();
  for (int i = 0; i < 4; i++) tires[i].dirty = true;
  sendJson("{\"ok\":true}");
}

void hPair() {
  wifiTouch();
  String a = server.arg("a");
  if (a == "start") {
    if (!startPairing()) { sendJson("{\"ok\":false,\"err\":\"demo\"}"); return; }
  } else if (a == "skip") {
    if (pairingSlot >= 0) { pairingSlot = pairingSlot + 1; pairUiDirty = true; }
  } else if (a == "cancel") {
    if (pairingSlot >= 0) cancelPairing();
  }
  sendJson("{\"ok\":true}");
}

void hWifi() {
  wifiTouch();
  sendJson("{\"ok\":true}");
  if (server.arg("on") != "1") wifiStopReq = true;     // ปิดหลังตอบเสร็จ (ทำในลูปหลัก)
}

void wifiRoutes() {
  server.on("/", HTTP_GET, hRoot);
  server.on("/api/state", HTTP_GET, hState);
  server.on("/api/learn", HTTP_POST, hLearn);
  server.on("/api/learncancel", HTTP_POST, hLearnCancel);
  server.on("/api/gears", HTTP_GET, hGears);
  server.on("/api/runs", HTTP_GET, hRuns);
  server.on("/api/rundel", HTTP_POST, hRunDel);
  server.on("/api/gearclear", HTTP_POST, hGearClear);
  server.on("/api/clear", HTTP_POST, hClear);
  server.on("/api/lock", HTTP_POST, hLock);
  server.on("/api/unit", HTTP_POST, hUnit);
  server.on("/api/mode", HTTP_POST, hMode);
  server.on("/api/tpms", HTTP_POST, hTpms);
  server.on("/api/pair", HTTP_POST, hPair);
  server.on("/api/wifi", HTTP_POST, hWifi);
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204); });
  server.onNotFound([]() { server.send(404, "text/plain", "not found"); });
}

void wifiLoop() {
  if (!wifiOn) return;
  server.handleClient();
  if (wifiStopReq || millis() - wifiLastAct > WIFI_IDLE_MS) wifiStop();
}

/* ========================================================= */
/* ================= BLE (TPMS) ============================ */
/* ========================================================= */

int findSlot(const char* mac) {
  for (int i = 0; i < 4; i++) {
    if (SENSOR_MAC[i][0] && strcasecmp(SENSOR_MAC[i], mac) == 0) return i;
    if (tires[i].mac[0] && strcasecmp(tires[i].mac, mac) == 0) return i;
  }
  return -1;
}

void handleDevice(BLEAdvertisedDevice& d) {
  if (!d.haveManufacturerData()) return;
  auto md = d.getManufacturerData();
  if (md.length() != 18) return;           // ฟอร์แมต TPMS ทั่วไป = 18 ไบต์
  const uint8_t* p = (const uint8_t*)md.c_str();

  // [0-1] company, [2-7] MAC, [8-11] แรงดัน, [12-15] อุณหภูมิ (x0.01 C), [16] แบตเตอรี่, [17] alarm
  uint32_t pr = p[8] | (p[9] << 8) | (p[10] << 16) | ((uint32_t)p[11] << 24);
  int32_t tp = (int32_t)(p[12] | (p[13] << 8) | (p[14] << 16) | ((uint32_t)p[15] << 24));
  float kpa = pr / PRESSURE_DIV;
  float tc = tp / 100.0f;

  auto macStr = d.getAddress().toString();
  const char* mac = macStr.c_str();
#if DEBUG_MODE == 1
  Serial.printf("TPMS %s  %.1f kPa  %.1f C  batt %d  alarm %d  raw:", mac, kpa, tc, p[16], p[17]);
  for (size_t k = 0; k < md.length(); k++) Serial.printf(" %02X", p[k]);
  Serial.println();
#endif

  if (kpa < 0 || kpa > 1000 || tc < -40 || tc > 150) return;

  int ps = pairingSlot;
  if (ps >= 0) {                   // โหมดจับคู่: รับเฉพาะตัวที่สัญญาณแรง (ถือใกล้บอร์ด)
    if (ps < 4 && d.getRSSI() > PAIR_RSSI) {
      for (int k = 0; k < ps; k++) {
        if (strcasecmp(tires[k].mac, mac) == 0) return;
      }
      strncpy(tires[ps].mac, mac, sizeof(tires[ps].mac) - 1);
      needSave = true;
      pairingSlot = ps + 1;
      pairUiDirty = true;
    }
    return;
  }

  int s = findSlot(mac);
  if (s < 0) return;
  tires[s].kpa = kpa;
  tires[s].tempC = tc;
  tires[s].batt = p[16];
  tires[s].alarm = p[17] != 0;
  tires[s].last = millis();
  tires[s].dirty = true;
}

// มี onResult หลายแบบเพื่อให้คอมไพล์ผ่านได้ทั้ง arduino-esp32 core 2.x และ 3.x
class TpmsCB : public BLEAdvertisedDeviceCallbacks {
 public:
  void onResult(BLEAdvertisedDevice d) { handleDevice(d); }
  void onResult(BLEAdvertisedDevice* d) { handleDevice(*d); }
  void onResult(const BLEAdvertisedDevice* d) { handleDevice(*const_cast<BLEAdvertisedDevice*>(d)); }
};

void scanTask(void*) {
  BLEScan* s = BLEDevice::getScan();
  for (;;) {
    s->start(5, false);
    s->clearResults();
    delay(100);
  }
}

void bleStart() {
  BLEDevice::init("");
  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new TpmsCB(), true);  // true = รับแพ็กเก็ตซ้ำ
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);
  xTaskCreate(scanTask, "scan", 4096, nullptr, 1, nullptr);
}

/* ========================================================= */
/* ================= DEMO DATA (TPMS) ====================== */
/* ========================================================= */

void demoUpdate() {
  static uint32_t lastMs = 0;
  if (millis() - lastMs < 1500) return;
  lastMs = millis();
  // kPa: หน้าซ้าย/หลังซ้ายปกติ (เขียว), หน้าขวา ~30.5 psi (เหลือง), หลังขวา ~27.6 psi (แดง)
  float base[4] = { 235, 210, 232, 190 };
  for (int i = 0; i < 4; i++) {
    tires[i].kpa = base[i] + random(-30, 30) / 10.0f;
    tires[i].tempC = 30 + i + random(-10, 10) / 10.0f;
    tires[i].batt = 90 - i * 25;
    tires[i].alarm = false;
    tires[i].last = millis();
    tires[i].dirty = true;
  }
}

/* ========================================================= */
/* ================= SETUP ================================= */
/* ========================================================= */

void setup() {
#if DEBUG_MODE == 1
  Serial.begin(115200);
  delay(500);
#endif
  for (int i = 0; i < NBTN; i++) pinMode(btnPin[i], INPUT_PULLUP);

  uiInit();                  // เริ่มจอ + LVGL + สร้างหน้าจอ (ถ้า USE_TFT 0 จะข้าม)
  loadSettings();
  uiUnitLabel();
  loadLearned();
  updateLearnedLabel();
  wifiRoutes();
  for (int i = 0; i < 4; i++) refreshPanel(i);

#if USE_LED
  strip.begin();
  strip.setBrightness(LED_BRIGHT_MAX);
  strip.show();
#endif

  analogReadResolution(12);
  analogSetAttenuation(ADC_ATTEN);
#if !LDR_ANALOG
  pinMode(LDR_PIN, INPUT);
#endif

  delay(300);
  for (int i = 0; i < NH; i++) {
    hraw[i] = readAvg(i);
    hf[i] = hraw[i];
  }
  lightF = readLight();
  ledBright = brightFromLight(lightF);

  if (!demoMode) bleStart();   // โหมด REAL: เริ่มสแกนเซนเซอร์ TPMS ทาง Bluetooth

#if DEBUG_MODE == 1
  Serial.println("=== GEAR INDICATOR + TPMS ===");
  Serial.printf("เรียนรู้เกียร์ไว้แล้ว %d/7 (%s) | โหมด TPMS = %s\n", __builtin_popcount(learnedMask),
                learnEnabled ? "เปิดเรียนรู้" : "ล็อก", demoMode ? "DEMO" : "REAL");
  printHelp();
#endif
}

/* ========================================================= */
/* ================= LOOP ================================== */
/* ========================================================= */

void loop() {
#if DEBUG_MODE == 1
  handleSerial();
#endif
  wifiLoop();                // หน้าเว็บ (ถ้าเปิด WiFi อยู่)

  /* ===== ปุ่ม MENU / NEXT / OK ===== */
  int btn = pollButtons();
  if (btn >= 0) onButton(btn);

  /* ===== จับคู่เซนเซอร์ ===== */
  if (needSave) {
    needSave = false;
    savePairs();
  }
  if (pairingSlot >= 4) {          // จับคู่ครบ/ข้ามครบแล้ว
    pairingSlot = -1;
    savePairs();
    uiPairHide();
    for (int i = 0; i < 4; i++) tires[i].dirty = true;
  } else if (pairingSlot >= 0 && pairUiDirty) {
    pairUiDirty = false;
    uiPairShow(pairingSlot);
    for (int i = 0; i < 4; i++) refreshPanel(i);
  }

  if (demoMode) demoUpdate();

  /* ===== TPMS: วาดใหม่เฉพาะแผงที่ข้อมูลเปลี่ยน + รีเฟรชสีเมื่อข้อมูลเก่าเกิน ===== */
  static uint32_t lastStaleCheck = 0;
  bool staleCheck = millis() - lastStaleCheck > 10000;
  if (staleCheck) lastStaleCheck = millis();
  if (pairingSlot < 0) {
    for (int i = 0; i < 4; i++) {
      if (tires[i].dirty || staleCheck) refreshPanel(i);
    }
  }

  /* ===== HALL + GEAR ===== */
  readHall();
  capSample();               // กำลังเก็บค่าเรียนรู้ (จากหน้าเว็บ) ก็สะสมค่าตรงนี้
  Match m = matchGear();

  int cand = (currentGear == 999) ? 0 : currentGear;
  if (!m.valid) cand = 0;
  else if (m.d1 <= UNKNOWN_DIST) cand = m.gear;

  // กฎทางผ่าน N: คอลัมน์ซ้าย {1,2} กลาง {3,4} ขวา {5,R} เปลี่ยนเกียร์ข้ามคอลัมน์ต้องผ่านช่อง N เสมอ
  // (ภายในคอลัมน์เดียวกัน 1-2, 3-4, 5-R เปลี่ยนตรงได้) ถ้าค่าเปลี่ยนข้ามคอลัมน์โดยไม่เคยผ่าน N
  // แปลว่าเป็นแค่คันเกียร์คลอน/ผ่านค่ากลางทาง ไม่ใช่เปลี่ยนเกียร์จริง
  static int pathGear = 999, rawPrev = 999;
  static bool passedN = false;
  static uint32_t rawSince = 0;
  if (cand != rawPrev) { rawPrev = cand; rawSince = millis(); }
  if (currentGear != pathGear) { pathGear = currentGear; passedN = false; }
  if (m.valid && m.gear == 0 && m.d1 <= PATH_N_DIST) passedN = true;
  if (currentGear != 999 && currentGear != 0 && cand != 0 && cand != currentGear &&
      gearColumn(currentGear) != gearColumn(cand) && !passedN &&
      millis() - rawSince < PATH_TIMEOUT_MS) cand = currentGear;

  // ฮิสเตอรีซิส: ถ้าเกียร์ปัจจุบันยังเป็นอันดับสองและห่างจากอันดับหนึ่งไม่ถึงเกณฑ์ ให้คงเกียร์เดิม
  if (m.valid && currentGear != 999 && cand != currentGear &&
      m.gear2 == currentGear && (m.d2 - m.d1) < GEAR_HYST) cand = currentGear;

  if (cand != lastCand) {
    lastCand = cand;
    candSince = millis();
  }

  uint32_t needMs = (cand == 5 || cand == -1) ? GEAR_WEAK_MS : GEAR_STABLE_MS;
  if (cand != currentGear && millis() - candSince >= needMs) {
#if DEBUG_MODE == 1
    if (currentGear != 999) {
      Serial.printf("### GEAR: %s -> %s | H0=%d H1=%d H2=%d | ห่าง %.0f",
                    gearName(currentGear), gearName(cand), hraw[0], hraw[1], hraw[2], m.d1);
      if (m.d2 < 1e8f) Serial.printf(" (รอง %s ห่าง %.0f)", gearName(m.gear2), m.d2);
      Serial.println();
    }
#endif
    currentGear = cand;
    showGear(currentGear);
  }

  /* ===== LIGHT + HALL VALUES ===== */
  updateBrightness();
  updateHallValues();

  /* ===== DEBUG STREAM ===== */
#if DEBUG_MODE == 1
  static unsigned long lastDebug = 0;
  if (streamOn && millis() - lastDebug >= DEBUG_INTERVAL) {
    lastDebug = millis();
    Serial.printf("RAW  H0=%4d H1=%4d H2=%4d", hraw[0], hraw[1], hraw[2]);
    if (m.valid) Serial.printf(" | ใกล้สุด=%s (ห่าง %.0f)", gearName(m.gear), m.d1);
    Serial.printf(" | แสดง=%s\n", gearName(currentGear));
  }
#endif

  uiTick();
  delay(5);
}

/*
  ----- ใช้รูปรถ / รูปพื้นหลัง (ไฟล์ .c ที่แถมมา วางไว้โฟลเดอร์เดียวกับไฟล์ .ino นี้) -----
  รูปรถ      : ไฟล์ car_img.c (260x285, โปร่งใส)  แล้วตั้ง USE_CAR_IMAGE 1
  พื้นหลัง   : ไฟล์ bg_img.c  (480x320)           แล้วตั้ง USE_BACKGROUND 1  (ตั้งทั้งสองพร้อมกันได้)

  ถ้าจะแปลงรูปเอง (https://lvgl.io/tools/imageconverter):
    - เปลี่ยนชื่อไฟล์รูปให้เป็นตัวอังกฤษล้วน ไม่มีช่องว่าง ขีดกลาง หรือวงเล็บ ก่อนแปลง
      (ชื่อไฟล์จะกลายเป็นชื่อตัวแปรใน .c ถ้ามี "(1)" หรือ "-" จะคอมไพล์ไม่ผ่าน)
      เช่นรูปรถชื่อ car_img.png  รูปพื้นหลังชื่อ bg_img.png
    - รูปพื้นหลัง: Color format = RGB565 ขนาด 480x320
    - รูปรถที่มีพื้นโปร่งใส: Color format = RGB565A8 (ถ้าใช้ RGB565 ธรรมดา พื้นโปร่งใสจะกลายเป็นสีดำ)
*/
