// Copyright(c) 2023 Takao Akaki (Oreginal m5stack-avatar-mic)
// Copyright(c) 2024 washishi

#include <M5Unified.h>
#include <Avatar.h>
#include "fft.hpp"
#include <cinttypes>

#ifdef ARDUINO_M5STACK_CORES3
#include <gob_unifiedButton.hpp>
#endif

#ifdef ECHO_BASE
#include <M5EchoBase.h>  // M5Unifyedでなんかマイクがうまく動かないので暫定で利用
M5EchoBase echobase(I2S_NUM_0);
#endif

#define USE_MIC
#define USE_FASTLED
// LEDをレベルメータとして使用する;
#ifdef USE_FASTLED
#include "FastLED.h"

#define USE_NEKOMIMI
// Nekomimi LED (9 9 straight) を使用する;

#ifdef USE_NEKOMIMI
  #define NUM_LEDS 18
  CRGB leds[NUM_LEDS];
  CRGB led_table[NUM_LEDS/2] = {CRGB::Purple,CRGB::MediumPurple,CRGB::Blue,CRGB::Green,CRGB::LimeGreen,CRGB::Yellow,CRGB::DarkOrange,CRGB::OrangeRed,CRGB::Red};
 #endif

void turn_off_led() {
  // Now turn the LED off, then pause
  for(int i=0;i<NUM_LEDS;i++) leds[i] = CRGB::Black;
  FastLED.show();
}

void fill_led_buff(CRGB color) {
  // Now turn the LED off, then pause
  for(int i=0;i<NUM_LEDS;i++) leds[i] =  color;
}

void clear_led_buff() {
  // Now turn the LED off, then pause
  for(int i=0;i<NUM_LEDS;i++) leds[i] =  CRGB::Black;
}

void level_led(int level1, int level2) {
  if(level1>NUM_LEDS/2) level1 = NUM_LEDS/2;
  if(level2>NUM_LEDS/2) level2 = NUM_LEDS/2;
  clear_led_buff();
  // LEDの順番
  //
  // M5GO Bottom2
  //    下 ------------------ 上
  // LED4 LED3 LED2 LED1 LED0
  // LED5 LED6 LED7 LED8 LED9
  //
  // Nekomimi
  // LED0 LED1  LED2  LED3  LED4  LED5  LED6  LED7　LED8
  // LED9 LED10 LED11 LED12 LED13 LED14 LED15 LED16 LED17
  
  // 右LED 
  for(int i=0;i<level1;i++){
#ifndef USE_NEKOMIMI
    leds[NUM_LEDS/2-1-i] = led_table[i];
#else
    leds[i] = led_table[i];
    leds[8-i] = led_table[i];
#endif
  }
  // 左LED
  for(int i=0;i<level2;i++){
    leds[i+NUM_LEDS/2] = led_table[i];
    leds[8-i+NUM_LEDS/2] = led_table[i];
  }
  FastLED.show();
}
#endif

#ifdef USE_MIC
  // ---------- Mic sampling ----------

  #define READ_LEN    (2 * 256)
  #define LIPSYNC_LEVEL_MAX 10.0f

  int16_t *adcBuffer = NULL;
  static fft_t fft;
  static constexpr size_t WAVE_SIZE = 256 * 2;

  static constexpr const size_t record_samplerate = 16000; // M5StickCPlus2だと48KHzじゃないと動かなかったが、M5Unified0.1.12で修正されたのとM5AtomS2+PDMUnitで不具合が出たので戻した。。
  static int16_t *rec_data;
  
  // setupの最初の方の機種判別で書き換えている場合があります。そちらもチェックしてください。（マイクの性能が異なるため）
  uint8_t lipsync_shift_level = 11; // リップシンクのデータをどのくらい小さくするか設定。口の開き方が変わります。
  float lipsync_max =LIPSYNC_LEVEL_MAX;  // リップシンクの単位ここを増減すると口の開き方が変わります。

#endif




using namespace m5avatar;

Avatar avatar;
ColorPalette *cps[6];
uint8_t palette_index = 0;

uint32_t last_rotation_msec = 0;
uint32_t last_lipsync_max_msec = 0;
uint8_t display_rotation = 1; // ディスプレイの向き(0〜3)
bool rotation_flg = false;

void lipsync() {

  size_t bytesread;
  uint64_t level1= 0;
  uint64_t level2= 0;
#ifndef SDL_h_
  switch (M5.getBoard()) {
  case m5::board_t::board_M5StackCoreS3:
  case m5::board_t::board_M5StackCoreS3SE:
    if ( M5.Mic.record(rec_data, WAVE_SIZE, record_samplerate,true)) {
      fft.exec(rec_data,1);
      for (size_t bx=5;bx<=60;++bx) {
        int32_t f = fft.get(bx);
        level1 += abs(f);
      }
      fft.exec(rec_data,2);
      for (size_t bx=5;bx<=60;++bx) {
        int32_t f = fft.get(bx);
        level2 += abs(f);
      }
    }
    break;
  default:
#ifdef ECHO_BASE
    size_t bytes_read = 0;
    if (i2s_read(I2S_NUM_0, rec_data, WAVE_SIZE, &bytes_read, echobase.getDuration(WAVE_SIZE)) == ESP_OK){
#else
    if ( M5.Mic.record(rec_data, WAVE_SIZE, record_samplerate)) {
#endif
      fft.exec(rec_data);
      for (size_t bx=5;bx<=60;++bx) {
        int32_t f = fft.get(bx);
        level1 += abs(f);
      }
      level2=level1;
    }
    break;
  }
  uint32_t temp_level1 = level1 >> lipsync_shift_level;
  uint32_t temp_level2 = level2 >> lipsync_shift_level;
  //M5_LOGI("level1:%" PRId64 "\n", level1) ;         // lipsync_maxを調整するときはこの行をコメントアウトしてください。
  //M5_LOGI("level2:%" PRId64 "\n", level2) ;         // lipsync_maxを調整するときはこの行をコメントアウトしてください。
  //M5_LOGI("temp_level1:%d\n", temp_level1) ;        // lipsync_maxを調整するときはこの行をコメントアウトしてください。
  //M5_LOGI("temp_level2:%d\n", temp_level2) ;        // lipsync_maxを調整するときはこの行をコメントアウトしてください。
  float ratio1 = (float)(temp_level1 / lipsync_max);
  float ratio2 = (float)(temp_level2 / lipsync_max);
  float ratio = (float)((ratio1+ratio2)/2);
  //M5_LOGI("ratio:%f\n", ratio);
  if (ratio <= 0.01f) {
    ratio = 0.0f;
    ratio1 = 0.0f;
    ratio2 = 0.0f;
    if ((lgfx::v1::millis() - last_lipsync_max_msec) > 500) {
      // 0.5秒以上無音の場合リップシンク上限をリセット
      last_lipsync_max_msec = lgfx::v1::millis();
      lipsync_max = LIPSYNC_LEVEL_MAX;
    }
  } else {
    if (ratio > 1.3f) {
      if (ratio > 1.5f) {
        // リップシンク上限を大幅に超えた場合、上限を上げていく。
        lipsync_max += 10.0f;
      }
      ratio = 1.3f;
      ratio1 = 1.3f;
      ratio2 = 1.3f;
    }
    last_lipsync_max_msec = lgfx::v1::millis(); // 無音でない場合は更新
  }

  if ((lgfx::v1::millis() - last_rotation_msec) > 350) {
    int direction = random(-2, 2);
    avatar.setRotation(direction * 10 * ratio);
    last_rotation_msec = lgfx::v1::millis();
  }
#else
  float ratio = 0.0f;
#endif
  avatar.setMouthOpenRatio(ratio);

#ifdef USE_FASTLED
   int led_level1 = (int)(ratio1*(NUM_LEDS/2));
   int led_level2 = (int)(ratio2*(NUM_LEDS/2));
   level_led(led_level1, led_level2);
#endif 
}
#ifdef ARDUINO_M5STACK_CORES3
goblib::UnifiedButton unifiedButton;
#endif

uint8_t rotation_position = 2; // 変更可能なディスプレイ向きの数 2 or 4

void setup()
{
  auto cfg = M5.config();
#if defined( PDM_PORTA )||( PDM_I2S )||( ECHO_BASE )
  cfg.internal_mic = false;
#else
  cfg.internal_mic = true;
#endif  
  M5.begin(cfg);
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);
  M5.Log.setEnableColor(m5::log_target_serial, false);
  M5_LOGI("Avatar Start");
  M5.Log.printf("M5.Log avatar Start\n");
  float scale = 0.0f;
  int8_t position_top = 0;
  int8_t position_left = 0;
//uint8_t display_rotation = 1; // ディスプレイの向き(0〜3)
  uint8_t first_cps = 0;
  auto mic_cfg = M5.Mic.config();
    switch (M5.getBoard()) {
    case m5::board_t::board_M5AtomS3:
    case m5::board_t::board_M5AtomS3R:
      first_cps = 4;
      scale = 0.55f;
      position_top =  -60;
      position_left = -95;
      display_rotation = 0;
      rotation_position = 4;
      // M5AtomS3は外部マイク(PDMUnit/ECHO BASE)なので設定を行う。
      mic_cfg.sample_rate = 16000;
      //mic_cfg.dma_buf_len = 256;
      //mic_cfg.dma_buf_count = 3;
#if defined( PDM_PORTA ) 
      mic_cfg.pin_ws = 1;
      mic_cfg.pin_data_in = 2;
#endif
#if defined( PDM_GPIO5_6 )
      mic_cfg.pin_ws = 5;
      mic_cfg.pin_data_in = 6;
#endif
      M5.Mic.config(mic_cfg);
#endif
#if defined( PDM_GPIO5_6 ) //Atom Mic
      mic_cfg.pin_ws = 5;
      mic_cfg.pin_data_in = 6;
      M5.Mic.config(mic_cfg);
#endif
#if defined( ECHO_BASE )
      echobase.init(16000 /*Sample Rate*/, 38 /*I2C SDA*/, 39 /*I2C SCL*/, 7 /*I2S DIN*/, 6 /*I2S WS*/, 5 /*I2S DOUT*/, 8 /*I2S BCK*/, Wire);
      i2s_set_clk(I2S_NUM_0,16000,I2S_BITS_PER_CHAN_16BIT,I2S_CHANNEL_MONO);  // EchoBaseのライブラリのチャンネル形式が異なるため変更
      echobase.setMicGain(ES8311_MIC_GAIN_6DB);  // Set microphone gain to 6dB.
      Wire.end(); // I2Cのピン(GPIO38)をFastLEDで利用するためI2Cを停止
#endif
      break;
case m5::board_t::board_M5StickC:
      first_cps = 1;
      scale = 0.6f;
      position_top = -80;
      position_left = -80;
      display_rotation = 3;
      break;

    case m5::board_t::board_M5StickCPlus:
      first_cps = 1;
      scale = 0.85f;
      position_top = -55;
      position_left = -35;
      display_rotation = 3;
      break;

    case m5::board_t::board_M5StickCPlus2:
      first_cps = 2;
      scale = 0.85f;
      position_top = -55;
      position_left = -35;
      display_rotation = 3;
      break;
   
     case m5::board_t::board_M5StackCore2:
      scale = 1.0f;
      position_top = 0;
      position_left = 0;
      display_rotation = 1;
      first_cps = 3;
      // 外部マイク設定
#if defined( PDM_PORTA )
      mic_cfg.pin_ws = 33;
      mic_cfg.pin_data_in = 32;
      M5.Mic.config(mic_cfg);
#endif
      break;

    case m5::board_t::board_M5StackCoreS3:
    case m5::board_t::board_M5StackCoreS3SE:
      scale = 1.0f;
      position_top = 0;
      position_left = 0;
      display_rotation = 1;
      first_cps = 3;
      break;

    case m5::board_t::board_M5Stack:
      scale = 1.0f;
      position_top = 0;
      position_left = 0;
      display_rotation = 1;
      first_cps = 3;
      // 外部マイク設定
#if defined( PDM_PORTA )
      M5.In_I2C.release();
      mic_cfg.pin_ws = 22;
      mic_cfg.pin_data_in = 21;
      M5.Mic.config(mic_cfg);
#endif
#if defined( PDM_I2S )
      mic_cfg.pin_ws = 0;
      mic_cfg.pin_data_in = 34;
      M5.Mic.config(mic_cfg);
#endif
      break;

    case m5::board_t::board_M5Dial:
      first_cps = 1;
      scale = 0.8f;
      position_top =  0;
      position_left = -40;
      display_rotation = 2;
      // M5ADial(StampS3)は外部マイク(PDMUnit)なので設定を行う。(Port.A)
      mic_cfg.pin_ws = 15;
      mic_cfg.pin_data_in = 13;
      M5.Mic.config(mic_cfg);
      break;

    defalut:
      M5.Log.println("Invalid board.");
      break;
  }
#ifndef SDL_h_
  rec_data = (typeof(rec_data))heap_caps_malloc(WAVE_SIZE * sizeof(int16_t), MALLOC_CAP_8BIT);
  memset(rec_data, 0 , WAVE_SIZE * sizeof(int16_t));
  M5.Speaker.end();
#endif
#ifndef ECHO_BASE
  M5.Mic.begin();
#endif
  M5.Display.setRotation(display_rotation);
  avatar.setScale(scale);
  avatar.setPosition(position_top, position_left);
  avatar.init(1); // start drawing
  cps[0] = new ColorPalette();
  cps[0]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[0]->set(COLOR_BACKGROUND, TFT_YELLOW);
  cps[1] = new ColorPalette();
  cps[1]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[1]->set(COLOR_BACKGROUND, TFT_ORANGE);
  cps[2] = new ColorPalette();
  cps[2]->set(COLOR_PRIMARY, (uint16_t)0x00ff00);
  cps[2]->set(COLOR_BACKGROUND, (uint16_t)0x303303);
  cps[3] = new ColorPalette();
  cps[3]->set(COLOR_PRIMARY, TFT_WHITE);
  cps[3]->set(COLOR_BACKGROUND, TFT_BLACK);
  cps[4] = new ColorPalette();
  cps[4]->set(COLOR_PRIMARY, TFT_BLACK);
  cps[4]->set(COLOR_BACKGROUND, TFT_WHITE);
  cps[5] = new ColorPalette();
  cps[5]->set(COLOR_PRIMARY, (uint16_t)0x303303);
  cps[5]->set(COLOR_BACKGROUND, (uint16_t)0x00ff00);
  avatar.setColorPalette(*cps[first_cps]);
  //avatar.addTask(lipsync, "lipsync");
  last_rotation_msec = lgfx::v1::millis();

#ifdef USE_FASTLED
  digitalWrite(LED_GPIO,LOW); // 1つ目のLEDが正常に動かないことがあるため追加
  FastLED.addLeds<SK6812, LED_GPIO, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(10);
  level_led(NUM_LEDS/2, NUM_LEDS/2);
  FastLED.show();
  delay(500);
#endif
  M5_LOGI("setup end");
}

uint32_t count = 0;

void loop()
{
  M5.update();
#ifdef ARDUINO_M5STACK_CORES3
  unifiedButton.update();
#endif
  if (M5.getBoard() == m5::board_t::board_M5StackCoreS3 && display_rotation == 3){
    // CoreS3は画面の反転で仮想ボタンの位置も変わってしまうため反転時はボタンCとして処理
    if (M5.BtnC.wasHold() && rotation_flg == false) {
//    M5.Display.setRotation(3);
      display_rotation+=2;
      if (display_rotation > 3){
        display_rotation %= 4;
      }
      M5.Display.setRotation(display_rotation);
      rotation_flg = true;
    } else if (M5.BtnC.wasClicked()) {
      M5_LOGI("Push BtnA");
      palette_index++;
      if (palette_index > 5) {
        palette_index = 0;
      }
      avatar.setColorPalette(*cps[palette_index]);
    }
  } else {
    if (M5.BtnA.wasHold() && rotation_flg == false) {
//    M5.Display.setRotation(3);
      display_rotation++;
      if (rotation_position == 2){
        display_rotation++;
      }
      if (display_rotation > 3){
        display_rotation %= 4;
      }
      M5.Display.setRotation(display_rotation);
      rotation_flg = true;
    } else if (M5.BtnA.wasClicked()) {
      M5_LOGI("Push BtnA");
      palette_index++;
      if (palette_index > 5) {
        palette_index = 0;
      }
      avatar.setColorPalette(*cps[palette_index]);
    }
  }
  if (M5.BtnA.isReleased() && M5.BtnC.isReleased()){
    rotation_flg = false;
  }
  if (M5.BtnPWR.wasClicked()) {
#ifdef ARDUINO
    esp_restart();
#endif
  } 
//  if ((millis() - last_rotation_msec) > 100) {
    //float angle = 10 * sin(count);
    //avatar.setRotation(angle);
    //last_rotation_msec = millis();
    //count++;
  //}

  // avatar's face updates in another thread
  // so no need to loop-by-loop rendering
  lipsync();
  lgfx::v1::delay(1);
}
