# Project Context

## Tong Quan

Project la PlatformIO Arduino Nano ATmega328 tai:

```text
C:\Users\tiend\Documents\PlatformIO\Projects\Dung_uno_nano_3D
```

Firmware chinh nam o `src/main.cpp`. Du an dung Arduino C++ de dieu khien relay,
servo va OLED theo lenh Serial.

## Cau Truc Quan Trong

- `platformio.ini`: cau hinh PlatformIO, board `nanoatmega328`.
- `src/main.cpp`: firmware chinh.
- `include/`: header dung chung neu sau nay can tach code.
- `lib/`: thu vien local cua du an.
- `test/`: test PlatformIO neu duoc them sau nay.
- `.pio/`: build output, khong sua tay.

## Phan Cung Va Giao Thuc

- Serial: 9600 baud.
- Relay B+: D7.
- FontCam servo: D2.
- RearCam servo: D8.
- SDcard servo: D5.
- OLED servo: D3.
- OLED: SH1106 128x64 I2C, U8g2 full-buffer.

## Lenh Build

Neu `pio` khong co tren PATH, dung:

```powershell
C:\Users\tiend\.platformio\penv\Scripts\pio.exe run -e nanoatmega328
```
