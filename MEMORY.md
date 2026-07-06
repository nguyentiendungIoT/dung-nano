# Long-Term Agent Memory

File nay giup agent nho boi canh du an giua cac phien lam viec. Day khong
phai bo nho chay tren Arduino va khong duoc dung de thay doi hanh vi firmware.

## Muc Tieu Du An

Day la du an PlatformIO cho Arduino Nano ATmega328. Firmware trong `src/main.cpp`
dieu khien relay B+, 4 servo va man hinh OLED SH1106 qua lenh Serial 9600 baud.

## Nguyen Tac Khi Agent Lam Viec

- Khong tu y sua firmware khi nguoi dung chi yeu cau tai lieu hoac tri nho agent.
- Truoc khi sua code, luon doc `src/main.cpp`, `platformio.ini`, va file lien quan.
- Khong tao module C++ moi neu chua co yeu cau ro rang.
- Khong chinh `.pio/`; day la thu muc build output cua PlatformIO.
- Neu can build, dung:

```powershell
C:\Users\tiend\.platformio\penv\Scripts\pio.exe run -e nanoatmega328
```

## Boi Canh Firmware Hien Tai

- Board: `nanoatmega328`.
- Framework: Arduino.
- Serial baudrate trong firmware: `9600`.
- Relay B+ dung chan D7.
- Servo FontCam dung D2.
- Servo RearCam dung D8.
- Servo SDcard dung D5.
- Servo OLED dung D3.
- OLED dung SH1106 128x64 qua I2C, full-buffer U8g2.

## Lenh Serial Da Biet

- `1`: bat relay B+.
- `2`: tat relay B+.
- `3`: FontCam ve 0 do.
- `4`: FontCam toi 45 do.
- `5`: RearCam ve 0 do.
- `6`: RearCam toi 45 do.
- `7`: chay chu ky servo SDcard.
- `8`: kich servo OLED.
- `44`: xoa OLED.
- Lenh 5 chu so: hien thi 3 cum so len OLED.

## Ghi Chu Cho Phien Sau

Neu nguoi dung noi "bo nho dai han cho agent", hieu la tao/cap nhat file `.md`
nhu `MEMORY.md` hoac `.agents/*.md`, khong phai EEPROM firmware.
