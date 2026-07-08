# Long-Term Agent Memory

File nay giup agent nho boi canh du an giua cac phien lam viec. Day khong
phai bo nho chay tren Arduino va khong duoc dung de thay doi hanh vi firmware.

## Muc Tieu Du An

Day la du an PlatformIO cho Arduino Nano ATmega328. Firmware trong `src/main.cpp`
dieu khien relay, 3 servo, man hinh OLED SH1106 I2C, va doc INA3221 qua Serial
9600 baud.

## Nguyen Tac Khi Agent Lam Viec

- Khong tu y sua firmware khi nguoi dung chi yeu cau tai lieu hoac tri nho agent.
- Truoc khi sua code, luon doc `src/main.cpp`, `platformio.ini`, va file lien quan.
- Khong tao module C++ moi neu chua co yeu cau ro rang.
- Khong chinh `.pio/`; day la thu muc build output cua PlatformIO.
- Khi thay doi pin, lenh Serial, env build, I2C/OLED/INA3221, hoac ket qua
  phan cung quan trong, phai cap nhat file nay cung luc.
- Tren may hien tai, dung PlatformIO:

```powershell
C:\Users\Dung.NT213690\.platformio\penv\Scripts\pio.exe run -e nanoatmega328new
```

## Boi Canh Firmware Hien Tai

- Default env trong `platformio.ini`: `nanoatmega328new`.
- Framework: Arduino.
- Serial baudrate trong firmware: `9600`.
- Relay B+ dung D9.
- Relay bo sung dung D10, D11, D12.
- Servo FontCam dung D2, home 0 do, work 25 do.
- Servo RearCam dung D8, home 0 do, work 25 do.
- Servo OLED dung D3, min pulse 544 us, max pulse 2400 us, target 120 do.
- Khong con servo SDcard trong source hien tai.
- I2C dung SDA=A4, SCL=A5.
- OLED dung SH1106 128x64 qua I2C, U8g2 full-buffer `U8G2_SH1106_128X64_NONAME_F_HW_I2C`,
  xoay `U8G2_R2`, clock 400 kHz de cap nhat man hinh muot hon.
- INA3221 tu probe dia chi `0x40..0x43`, mac dinh `0x40`, config `0x7127`,
  va duoc cau hinh mot lan khi boot.
- `SERIAL_DEBUG_ENABLE` dang tat (`0`); log debug tong quat khong in ra Serial.
- Theo yeu cau moi nhat, `INA3221_VALUE_LOG_ENABLE=1` de in dien ap/dong dien
  3 kenh moi 1 giay, va `OLED_DOT_LOG_ENABLE=1` de in trang thai cham OLED moi
  lan blink.

## Lenh Serial Hien Tai

- `3`: FontCam ve 0 do.
- `4`: FontCam toi 25 do.
- `5`: RearCam ve 0 do.
- `6`: RearCam toi 25 do.
- `8`: kich chu ky servo OLED.
- `9`: bat relay B+ D9.
- `10`: tat relay B+ D9.
- `11`: bat relay D10.
- `12`: tat relay D10.
- `13`: bat relay D11.
- `14`: tat relay D11.
- `15`: bat relay D12.
- `16`: tat relay D12.
- `44`: xoa OLED.
- Lenh 5 chu so: hien thi 3 cum so len OLED.

## Ghi Chu I2C Gan Nhat

- Debug phan cung truoc do xac dinh loi la bus I2C bi keo LOW khi gan module/day,
  khong phai chi do sai dia chi INA3221 `0x40`.
- Nguoi dung da bao sau khi thao OLED va chi de INA3221 thi he thong hoat dong ngon.
- Da bo log debug runtime on ao va khoi phuc OLED ve full-buffer 400 kHz.
- Theo yeu cau sau do, da bat lai log co chon loc cho gia tri INA3221 va cham OLED.
- Ban full-buffer kem log INA/OLED moi nhat build thanh cong voi RAM
  `1844/2048 bytes` va Flash `18916/30720 bytes`.
- Upload ban co log INA/OLED nay chua thanh cong vi COM3 ton tai la CH340 nhung
  dang bi process khac giu: `PermissionError(13, 'Access is denied')`.

## Ghi Chu Cho Phien Sau

Neu nguoi dung noi "bo nho dai han cho agent", hieu la tao/cap nhat file `.md`
nhu `MEMORY.md` hoac `.agents/*.md`, khong phai EEPROM firmware.
