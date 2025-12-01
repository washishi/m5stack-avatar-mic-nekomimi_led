
# m5stack-avatar-mic-nekomimi_led
[Nekomimi LED](https://github.com/washishi/nekomimi_led) のサンプルプログラムです  
音に合わせてAvatarが口パクしたり、傾いたりすると共にNekomimi LEDが音量に応じて光ります  
またボタンAを押すと画面の色が変わります、長押しすると表示方向が変化します(繰り返し行うと元に戻ります) 
なおｽﾀｯｸﾁｬﾝに入れてもサーボの制御はしていないためサーボは動きません  

※本プログラムは Takao Akaki (mongonta0716) さんの [m5stack-avatar-mic](https://github.com/mongonta0716/m5stack-avatar-mic) を改変したものです  
　またそれを改変した 
robo8080さんの [m5stack-avatar-mic-led](https://github.com/robo8080/m5stack-avatar-mic-led) も参考にさせてもらっています

# 環境
VSCode + PlatformIO  

# 使用ライブラリ等 
詳細はplatformio.iniを見てください  
・espressif32@6.5.0  
・M5Stack-Avatar@0.9.2
・M5Unified@0.2.2
・FastLED@3.6.0
・gob_unifiedButton@0.1.5 (CoreS3,S3SEのみ)

# 対応デバイス

- M5S5tack  BASIC + M5GO BOTTOM または M5GO か FIRE
  - env:M5Stack_BASIC-M5GO_BOTTOM を選択
  - マイクはM5GO BOTTOM のアナログマイクを利用します(GPIO34をAD変換)
  - Nekomimi LEDはPortBに接続(O端子を利用、G26)

- M5S5tack BASIC + PDMマイクユニット
  - env:M5Stack_BASIC-PDM_PortA を選択
  - PDMマイクユニット を PortAに接続
  - Nekomimi LED は GND,+5V,G26 に何とかして接続してください

- M5S5tack BASIC + CORE2_Ext._board等
  - env:M5Stack_BASIC-PDM_I2S を選択
  - CORE2_Expansion_boardをCORE2より借用して接続しそのPDMマイクを利用  
    またはそのマイクの互換品をなんとかして接続(GND,+3.3v,G0:CLK,G34:DAT)
  - Nekomimi LED は GND,+5V,G26 に何とかして接続してください
    
- CORE2 + M5GO BOTTOM2 または CORE2 FOR AWS
  - env:M5Stack_CORE2-M5GO_BOTTOM2 を選択
  - マイクはM5GO BOTTOM2 のPDMマイクを利用します(G0:CLK,G34:DAT)
  - Nekomimi LEDはPortBに接続(O端子を利用、G26)

- CORE2 (PortAにNekomimi LEDを取り付け)
  - env:M5Stack_CORE2-LED_PortA を選択
  - マイクはCORE2_Expansion_boardのPDMマイクを利用(GND,+3.3v,G0:CLK,G34:DAT)
  - Nekomimi LEDはPortAに接続(DATA端子を利用、G32)

- CORE2 + PDMマイクユニット(PortA)
  - env:M5Stack_CORE2-PDM_PortA を選択
  - PDMマイクユニット を PortAに接続
  - Nekomimi LED は GND,+5V,G26 に何とかして接続してください

- CoreS3 または CoreS3 SE + DIN BASE もしくは M5GO3 Bottom
  - env:M5Stack_CoreS3 を選択
  - マイクは内蔵のマイクを利用します(DUAL MIC対応)
  - Nekomimi LEDはPortBに接続(O端子を利用、G9)

- CoreS3 または CoreS3 SE  (PortAにNekomimi LEDを取り付け)
  - env:M5Stack_CoreS3-LED_PortA を選択
  - マイクは内蔵のマイクを利用します(DUAL MIC対応)
  - Nekomimi LEDはPortAに接続(DATA端子を利用、G2)

- M5StickC または M5StickC Plus
  - env:M5StickC を選択
  - Nekomimi LED は GROVE PORTに接続(G32)

- M5StickC Plus2
  - env:M5StickC_PLUS2 を選択
  - Nekomimi LED は GROVE PORTに接続(G32)

- AtomS3 または AtomS3R + PDMマイクユニット
  - env:M5AtomS3-PDM を選択
  - PDMマイクユニット を GROVE PORTに接続
  - Nekomimi LED は GND,+5V,G8 に何とかして接続してください

- AtomS3 または AtomS3R + [Atom_MIC](https://github.com/washishi/atom_mic)
  - env:M5AtomS3-AtomMIC を選択
  - Atom_MIC を接続
  - Nekomimi LED は Atom_MIC の GROVEコネクタに接続してください

- M5Dial + PDMマイクユニット(PortA)
  - env:M5Dial-PDM_PortA を選択
  - PDMマイクユニット を PortAに接続
  - Nekomimi LED は PortBに接続(O端子を利用、G2)  

# LICENSE
MIT  
  ・[m5stack-avatar-mic-nekomimi](https://github.com/washishi/m5stack-avatar-mic-nekomimi_led/blob/main/LICENSE)  
  ・[m5stack-avatar-mic (original)](https://github.com/mongonta0716/m5stack-avatar-mic/blob/main/LICENSE)
# Author
[washishi](https://github.com/washishi)
