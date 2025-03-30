#include <M5Unified.h>

// ======================================================================
// LED関連
// ======================================================================
#include <FastLED.h>
#define ATOM_LED_PIN 27 // ATOM Lite本体のLED用
#define ATOM_NUM_LEDS 1 // Atom LED

static CRGB atom_leds[ATOM_NUM_LEDS];

// Atom本体のLEDを光らせる用の関数
void setLed(CRGB color)
{
  // change RGB to GRB
  uint8_t t = color.r;
  color.r = color.g;
  color.g = t;
  atom_leds[0] = color;
  FastLED.show();
}
// ======================================================================

// ======================================================================
// AtomicMotion関連
// ======================================================================
#include <M5AtomicMotion.h>
M5AtomicMotion AtomicMotion;

// AtomMotionのサーボモーターのチャンネル
u_int ch_right = 0; // AtomMotion label S1
u_int ch_left  = 1;  // AtomMotion label S2
// u_int ch_left  = 2; // AtomMotion label S3
// u_int ch_right = 3; // AtomMotion label S4

// ======================================================================
// サーボモーター制御
// ======================================================================

// 操作モード判定用フラグ
bool CONTROL_MODE = false;

// サーボPWM設定：SG90を想定
const uint16_t CENTER_PWM = 1500;
const uint16_t CENTER_DEG = 90;
const uint16_t CHx_MAX_L = 300;
const uint16_t CHx_MAX_R = 290;

// ======================================================================
// ESPNow関連
// ======================================================================
#include <WiFi.h>    // ESPNOWを使う場合はWiFiも必要
#include <esp_now.h> // ESPNOW本体

// ESP-NOW受信時に呼ばれる関数
void OnDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  // Serial.print("Last Packet Recv from: ");  Serial.println(macStr);
  // Serial.print("Last Packet Recv Data: ");  Serial.println(*data);

  Serial.print(data[0]);
  Serial.print("\t");
  Serial.print(data[1]);
  Serial.print("\t");
  Serial.println(data[2]);

  if ((data[2] == 0))
  {
    // 停止モード
    CONTROL_MODE = false;
    setLed(CRGB::Orange);
  } else {
    // スティック操作モード
    CONTROL_MODE = true;
    setLed(CRGB::Green);
  }

  if(CONTROL_MODE){

    // Lサーボ前進
    if (data[0] == 1)
    {
      setLed(CRGB::Blue);
      AtomicMotion.setServoPulse(ch_left, CENTER_PWM + CHx_MAX_L);
    }
    // Lサーボ後退
    else if (data[0] == 2)
    {
      setLed(CRGB::Pink);
      AtomicMotion.setServoPulse(ch_left, CENTER_PWM - CHx_MAX_L);
    }
    // Lサーボ停止
    else
    {
      setLed(CRGB::Green);
      AtomicMotion.setServoPulse(ch_left, CENTER_PWM);
    }

    // Rサーボ前進
    if ((data[2] == 1 && data[0] == 1) || (data[2] == 2 && data[1] == 1))
    {
      AtomicMotion.setServoPulse(ch_right, CENTER_PWM - CHx_MAX_R);
    }
    // Rサーボ後退
    else if ((data[2] == 1 && data[0] == 2) || (data[2] == 2 && data[1] == 2))
    {
      AtomicMotion.setServoPulse(ch_right, CENTER_PWM + CHx_MAX_R);
    }
    // Rサーボ停止
    else
    {
      setLed(CRGB::Black);
      AtomicMotion.setServoPulse(ch_right, CENTER_PWM);
    }

  } else {
    // 停止
    AtomicMotion.setServoPulse(ch_left, CENTER_PWM);
    AtomicMotion.setServoPulse(ch_right, CENTER_PWM);
  }

}
// ======================================================================

// ======================================================================
// setup & loop関連
// ======================================================================
void setup()
{
  // 設定用の情報を抽出
  auto cfg = M5.config();
  // M5Stackをcfgの設定で初期化
  M5.begin(cfg);

  // ================================
  // AtomMotion用の設定
  // AtomLite PIN
  uint8_t sda = 25;
  uint8_t scl = 21;
  while (
      !AtomicMotion.begin(&Wire, M5_ATOMIC_MOTION_I2C_ADDR, sda, scl, 100000))
  {
    Serial.println("Atomic Motion begin failed");
    delay(1000);
  }

  // ================================
  // EspNowの設定
  // WiFi初期化
  WiFi.mode(WIFI_STA);
  // ESP-NOWの初期化(出来なければリセットして繰り返し)
  if (esp_now_init() != ESP_OK)
  {
    return;
  }
  // ESP-NOW受信時に呼ばれる関数の登録
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived));

  // LEDの設定
  FastLED.addLeds<WS2811, ATOM_LED_PIN, RGB>(atom_leds, ATOM_NUM_LEDS);
  FastLED.setBrightness(5);
  for (int i = 0; i < 3; i++)
  {
    setLed(CRGB::Red);
    delay(500);
    setLed(CRGB::Black);
    delay(500);
    setLed(CRGB::Orange);
  }
}

void loop()
{
  M5.update();

  // if (M5.BtnA.wasPressed())
  // {
  // }

  delay(1);
}
