#include <Arduino.h> // Nap API Arduino co ban nhu Serial, pinMode, digitalWrite va millis.
#include <Wire.h>    // Nap driver I2C dung chung cho OLED va INA3221.
#include <Servo.h>   // Nap thu vien dieu khien cac servo con lai trong he thong.
#include <U8g2lib.h> // Nap thu vien dieu khien man hinh OLED SH1106.

#define SERIAL_DEBUG_ENABLE 1 // Bat log debug qua Serial; doi thanh 0 neu muon tat log.

constexpr uint8_t PIN_RELAY_BPLUS = 9;   // Gan chan D9 cho relay B+.
constexpr uint8_t PIN_RELAY_D10 = 10;    // Gan chan D10 cho relay bo sung 1.
constexpr uint8_t PIN_RELAY_D11 = 11;    // Gan chan D11 cho relay bo sung 2.
constexpr uint8_t PIN_RELAY_D12 = 12;    // Gan chan D12 cho relay bo sung 3.
constexpr uint8_t PIN_SERVO_FONTCAM = 2; // Gan chan D2 cho servo camera truoc.
constexpr uint8_t PIN_SERVO_REARCAM = 8; // Gan chan D8 cho servo camera sau.
constexpr uint8_t PIN_SERVO_OLED = 3;    // Gan chan D3 cho servo OLED.

constexpr uint8_t RELAY_ON = HIGH; // Dinh nghia muc tin hieu bat relay.
constexpr uint8_t RELAY_OFF = LOW; // Dinh nghia muc tin hieu tat relay.

constexpr uint32_t SERIAL_BAUDRATE = 9600; // Dinh nghia toc do Serial 9600 baud.
constexpr uint32_t RX_TIMEOUT_MS = 35;     // Dinh nghia thoi gian cho de ket thuc lenh khong co xuong dong.
constexpr size_t RX_BUFFER_SIZE = 16;      // Dinh nghia kich thuoc bo dem nhan lenh Serial.

constexpr uint8_t SERVO_FONTCAM_HOME_ANGLE = 0;  // Dinh nghia goc home cua servo camera truoc.
constexpr uint8_t SERVO_FONTCAM_WORK_ANGLE = 15; // Dinh nghia goc lam viec cua servo camera truoc.

constexpr uint8_t SERVO_REARCAM_HOME_ANGLE = 0;  // Dinh nghia goc home cua servo camera sau.
constexpr uint8_t SERVO_REARCAM_WORK_ANGLE = 15; // Dinh nghia goc lam viec cua servo camera sau.
constexpr uint8_t SERVO_ANGLE_UNKNOWN = 255;     // Dinh nghia gia tri danh dau chua biet goc servo.

constexpr uint8_t SERVO_OLED_MIN_ANGLE = 0;         // Dinh nghia goc ban dau cua servo OLED.
constexpr uint8_t SERVO_OLED_TARGET_ANGLE = 120;    // Dinh nghia goc dich cua servo OLED.
constexpr uint16_t SERVO_OLED_MIN_PULSE_US = 544;   // Dinh nghia xung nho nhat cho servo OLED.
constexpr uint16_t SERVO_OLED_MAX_PULSE_US = 2400;  // Dinh nghia xung lon nhat cho servo OLED.
constexpr uint32_t SERVO_OLED_BOOST_MS = 160;       // Dinh nghia thoi gian day xung tang toc cho servo OLED.
constexpr uint32_t SERVO_OLED_TARGET_HOLD_MS = 300; // Dinh nghia thoi gian giu servo OLED o goc dich.

constexpr uint32_t OLED_I2C_CLOCK_HZ = 400000UL; // Dinh nghia toc do bus I2C cua OLED.
constexpr uint8_t OLED_DISPLAY_CLOCK = 0xF0;     // Dinh nghia clock noi bo cua OLED.
constexpr uint8_t OLED_CONTRAST = 255;           // Dinh nghia do tuong phan OLED toi da.

constexpr uint32_t OLED_STATUS_DOT_INTERVAL_MS = 500; // Dinh nghia chu ky nhap nhay cham trang thai.
constexpr uint8_t OLED_STATUS_DOT_X = 122;            // Dinh nghia toa do X cua cham trang thai.
constexpr uint8_t OLED_STATUS_DOT_Y = 58;             // Dinh nghia toa do Y cua cham trang thai.
constexpr uint8_t OLED_STATUS_DOT_RADIUS = 5;         // Dinh nghia ban kinh cham trang thai.

constexpr uint8_t INA3221_I2C_ADDRESS = 0x40;            // Dia chi mac dinh khi chan A0 noi GND.
constexpr uint8_t INA3221_REGISTER_CONFIG = 0x00;        // Thanh ghi cau hinh INA3221.
constexpr uint8_t INA3221_REGISTER_CH1_SHUNT = 0x01;     // Thanh ghi shunt voltage kenh 1.
constexpr uint8_t INA3221_REGISTER_CH1_BUS = 0x02;       // Thanh ghi bus voltage kenh 1.
constexpr uint16_t INA3221_CONFIG_DEFAULT = 0x7127;      // Bat 3 kenh, do lien tuc shunt + bus.
constexpr uint32_t INA3221_SAMPLE_INTERVAL_MS = 200;     // Chu ky doc INA3221 la 0,2 giay.
constexpr float INA3221_BUS_LSB_V = 0.008f;              // Bus voltage LSB sau khi bo 3 bit reserve.
constexpr float INA3221_SHUNT_LSB_V = 0.000040f;         // Shunt voltage LSB sau khi bo 3 bit reserve.
constexpr float INA3221_SHUNT_RESISTOR_OHMS = 0.1f;      // Gia tri shunt R100 thuong gap tren module.

Servo servoFontCam; // Tao doi tuong servo cho camera truoc.
Servo servoRearCam; // Tao doi tuong servo cho camera sau.
Servo servoOled;    // Tao doi tuong servo cho co cau OLED.

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R2, U8X8_PIN_NONE); // Tao driver OLED SH1106 I2C xoay 180 do.

enum class OledServoState : uint8_t // Khai bao cac trang thai cua chu ky servo OLED.
{                                   // Bat dau danh sach trang thai servo OLED.
  IDLE,                             // Trang thai servo OLED dang ranh.
  BOOSTING,                         // Trang thai servo OLED dang nhan xung tang toc.
  WAITING_RETURN                    // Trang thai servo OLED dang cho quay ve vi tri ban dau.
}; // Ket thuc enum trang thai servo OLED.

enum class OledContentMode : uint8_t // Khai bao cac che do noi dung hien thi OLED.
{                                    // Bat dau danh sach che do hien thi OLED.
  CLEAR,                             // Che do man hinh trong.
  DIGITS                             // Che do hien thi cac chu so nhan tu Serial.
}; // Ket thuc enum che do hien thi OLED.

OledServoState oledServoState = OledServoState::IDLE;     // Luu trang thai hien tai cua servo OLED.
OledContentMode oledContentMode = OledContentMode::CLEAR; // Luu che do noi dung hien tai cua OLED.

uint8_t rearCamAngle = SERVO_ANGLE_UNKNOWN; // Luu goc gan nhat cua servo camera sau.
uint8_t oledDigitP1 = 0;                    // Luu chu so nhom 1 de ve len OLED.
uint8_t oledDigitP2 = 0;                    // Luu so nhom 2 de ve len OLED.
uint8_t oledDigitP3 = 0;                    // Luu so nhom 3 de ve len OLED.

uint32_t oledServoStartMs = 0;          // Luu moc thoi gian bat dau trang thai servo OLED.
uint32_t oledStatusDotLastToggleMs = 0; // Luu moc thoi gian lan doi cham trang thai gan nhat.
uint32_t ina3221LastSampleMs = 0;       // Luu moc thoi gian doc INA3221 gan nhat.

bool oledStatusDotVisible = true; // Luu trang thai hien/an cua cham trang thai OLED.
bool ina3221Online = false;       // Luu trang thai phat hien INA3221 tren bus I2C.

char rxBuffer[RX_BUFFER_SIZE] = {0}; // Tao bo dem chua lenh Serial dang nhan.
size_t rxLen = 0;                    // Luu so ky tu hien co trong bo dem nhan.
uint32_t lastRxByteMs = 0;           // Luu thoi diem nhan byte Serial gan nhat.

#if SERIAL_DEBUG_ENABLE                                                       // Bien dich cac ham log that khi debug dang bat.
void logLine(const __FlashStringHelper *tag, const __FlashStringHelper *text) // In mot dong log tu chuoi flash.
{                                                                             // Bat dau ham log chuoi flash.
  Serial.print(tag);                                                          // In tien to log.
  Serial.println(text);                                                       // In noi dung log va xuong dong.
} // Ket thuc ham log chuoi flash.

void logLine(const __FlashStringHelper *tag, const char *text) // In mot dong log tu chuoi RAM.
{                                                              // Bat dau ham log chuoi RAM.
  Serial.print(tag);                                           // In tien to log.
  Serial.println(text);                                        // In noi dung log va xuong dong.
} // Ket thuc ham log chuoi RAM.

void logRxByte(uint8_t b)          // In byte Serial vua nhan de debug.
{                                  // Bat dau ham log byte Serial.
  Serial.print(F("[RX-BYTE] 0x")); // In nhan hex cua byte nhan.
  if (b < 0x10)                    // Kiem tra byte co can them so 0 dang truoc khong.
  {                                // Bat dau nhanh them so 0.
    Serial.print('0');             // In so 0 de giu dinh dang hai ky tu hex.
  } // Ket thuc nhanh them so 0.
  Serial.print(b, HEX);                 // In gia tri byte o he hex.
  if (b >= 32 && b <= 126)              // Kiem tra byte co phai ky tu ASCII in duoc khong.
  {                                     // Bat dau nhanh in ky tu ASCII.
    Serial.print(F(" ('"));             // In dau mo phan ky tu.
    Serial.write(static_cast<char>(b)); // In ky tu tuong ung cua byte.
    Serial.println(F("')"));            // In dau dong phan ky tu va xuong dong.
  } // Ket thuc nhanh in ky tu ASCII.
  else                // Xu ly byte khong in duoc.
  {                   // Bat dau nhanh xuong dong don gian.
    Serial.println(); // Xuong dong sau gia tri hex.
  } // Ket thuc nhanh xuong dong don gian.
} // Ket thuc ham log byte Serial.
#else  // Bien dich cac ham log rong khi debug dang tat.
void logLine(const __FlashStringHelper *, const __FlashStringHelper *) {} // Bo qua log chuoi flash khi debug tat.
void logLine(const __FlashStringHelper *, const char *) {}                // Bo qua log chuoi RAM khi debug tat.
void logRxByte(uint8_t) {}                                                // Bo qua log byte Serial khi debug tat.
#endif // Ket thuc lua chon bien dich log debug.

void configureRelayOutput(uint8_t pin)      // Cau hinh mot chan relay ve trang thai tat an toan.
{                                           // Bat dau ham cau hinh relay.
  pinMode(pin, OUTPUT);                     // Dat chan relay la output.
  digitalWrite(pin, RELAY_OFF);             // Tat relay ngay khi khoi dong.
} // Ket thuc ham cau hinh relay.

void relaySet(uint8_t pin, const __FlashStringHelper *name, bool on) // Bat hoac tat relay theo chan chi dinh.
{                                                                    // Bat dau ham dieu khien relay.
  digitalWrite(pin, on ? RELAY_ON : RELAY_OFF);                      // Xuat muc tin hieu relay theo yeu cau.
#if SERIAL_DEBUG_ENABLE                                              // Chi log khi debug dang bat.
  Serial.print(F("[ACT] "));                                         // In tien to log hanh dong.
  Serial.print(name);                                                // In ten relay.
  Serial.println(on ? F(" ON") : F(" OFF"));                         // In trang thai relay.
#endif                                                               // Ket thuc khoi log co dieu kien.
} // Ket thuc ham dieu khien relay.

void setRearCamAngle(uint8_t targetAngle)                                                                                                     // Dat servo camera sau den goc yeu cau.
{                                                                                                                                             // Bat dau ham dat goc camera sau.
  if (rearCamAngle == targetAngle)                                                                                                            // Kiem tra servo da o dung goc yeu cau chua.
  {                                                                                                                                           // Bat dau nhanh bo qua neu khong can di chuyen.
    logLine(F("[ACT] "), targetAngle == SERVO_REARCAM_HOME_ANGLE ? F("ServoRearCam already at 0 deg") : F("ServoRearCam already at 45 deg")); // Ghi log da o san goc yeu cau.
    return;                                                                                                                                   // Thoat ham vi khong can ghi lai servo.
  } // Ket thuc nhanh bo qua neu khong can di chuyen.

  servoRearCam.write(targetAngle);                                                                                          // Gui goc moi den servo camera sau.
  rearCamAngle = targetAngle;                                                                                               // Cap nhat bien nho goc hien tai.
  logLine(F("[ACT] "), targetAngle == SERVO_REARCAM_HOME_ANGLE ? F("ServoRearCam -> 0 deg") : F("ServoRearCam -> 45 deg")); // Ghi log goc moi.
} // Ket thuc ham dat goc camera sau.

void configureOledForCamera()                                                                   // Khoi tao va cau hinh man hinh OLED.
{                                                                                               // Bat dau ham cau hinh OLED.
  oled.setBusClock(OLED_I2C_CLOCK_HZ);                                                          // Dat toc do I2C cho OLED.
  oled.begin();                                                                                 // Khoi dong driver OLED.
  oled.setContrast(OLED_CONTRAST);                                                              // Dat do tuong phan OLED.
  oled.sendF("ca", 0x0D5, OLED_DISPLAY_CLOCK);                                                  // Gui lenh dat display clock cho OLED.
  logLine(F("[OLED-CONFIG] "), F("full-buffer, i2c=400kHz, display-clock=0xF0, contrast=255")); // Ghi log cau hinh OLED.
} // Ket thuc ham cau hinh OLED.

bool ina3221WriteRegister(uint8_t reg, uint16_t value) // Ghi mot thanh ghi 16-bit cua INA3221.
{                                                       // Bat dau ham ghi thanh ghi INA3221.
  Wire.beginTransmission(INA3221_I2C_ADDRESS);          // Bat dau giao tiep voi INA3221.
  Wire.write(reg);                                      // Chon thanh ghi dich.
  Wire.write(static_cast<uint8_t>(value >> 8));         // Ghi byte cao truoc theo chuan SMBus word.
  Wire.write(static_cast<uint8_t>(value & 0xFF));       // Ghi byte thap.
  return Wire.endTransmission() == 0;                   // Bao thanh cong khi thiet bi ACK.
} // Ket thuc ham ghi thanh ghi INA3221.

bool ina3221ReadRegister(uint8_t reg, int16_t &value) // Doc mot thanh ghi 16-bit cua INA3221.
{                                                     // Bat dau ham doc thanh ghi INA3221.
  Wire.beginTransmission(INA3221_I2C_ADDRESS);        // Bat dau giao tiep voi INA3221.
  Wire.write(reg);                                    // Dat con tro thanh ghi can doc.
  if (Wire.endTransmission(false) != 0)               // Gui repeated-start de doc ngay sau khi chon thanh ghi.
  {                                                   // Bat dau nhanh xu ly loi I2C.
    return false;                                     // Bao doc that bai.
  } // Ket thuc nhanh xu ly loi I2C.

  const uint8_t bytesRead = Wire.requestFrom(INA3221_I2C_ADDRESS, static_cast<uint8_t>(2)); // Doc 2 byte du lieu.
  if (bytesRead != 2)                                                                       // Kiem tra du so byte can doc.
  {                                                                                         // Bat dau nhanh xu ly thieu byte.
    return false;                                                                           // Bao doc that bai.
  } // Ket thuc nhanh xu ly thieu byte.

  const uint16_t raw = static_cast<uint16_t>(Wire.read()) << 8 | static_cast<uint16_t>(Wire.read()); // Ghep byte cao/thap.
  value = static_cast<int16_t>(raw);                                                                // Giu nguyen dang co dau 16-bit.
  return true;                                                                                      // Bao doc thanh cong.
} // Ket thuc ham doc thanh ghi INA3221.

bool configureIna3221() // Khoi tao INA3221 o che do do lien tuc ca 3 kenh.
{                       // Bat dau ham cau hinh INA3221.
  if (!ina3221WriteRegister(INA3221_REGISTER_CONFIG, INA3221_CONFIG_DEFAULT)) // Kiem tra thiet bi co ACK khong.
  {                                                                          // Bat dau nhanh xu ly khong thay INA3221.
    logLine(F("[INA3221] "), F("not found at 0x40 or config write failed")); // Ghi log loi ket noi/cau hinh.
    return false;                                                            // Bao cau hinh that bai.
  } // Ket thuc nhanh xu ly khong thay INA3221.

  logLine(F("[INA3221] "), F("ready at 0x40, continuous 3-channel mode")); // Ghi log INA3221 san sang.
  return true;                                                            // Bao cau hinh thanh cong.
} // Ket thuc ham cau hinh INA3221.

float ina3221RawToVoltage(int16_t raw, float lsbVoltage) // Quy doi gia tri thanh ghi INA3221 sang volt.
{                                                        // Bat dau ham quy doi gia tri INA3221.
  return static_cast<float>(raw / 8) * lsbVoltage;       // Bo 3 bit reserve, giu dau va nhan voi LSB.
} // Ket thuc ham quy doi gia tri INA3221.

bool readIna3221Channel(uint8_t channelIndex, float &busVoltageV, float &shuntVoltageV, float &currentA)
{                                                                                    // Bat dau ham doc mot kenh INA3221.
  const uint8_t shuntRegister = INA3221_REGISTER_CH1_SHUNT + channelIndex * 2;       // Tinh thanh ghi shunt cua kenh.
  const uint8_t busRegister = INA3221_REGISTER_CH1_BUS + channelIndex * 2;           // Tinh thanh ghi bus cua kenh.
  int16_t rawShunt = 0;                                                              // Luu gia tri shunt tho.
  int16_t rawBus = 0;                                                                // Luu gia tri bus tho.

  if (!ina3221ReadRegister(shuntRegister, rawShunt) || !ina3221ReadRegister(busRegister, rawBus)) // Doc shunt va bus.
  {                                                                                                // Bat dau nhanh xu ly loi doc.
    return false;                                                                                  // Bao doc kenh that bai.
  } // Ket thuc nhanh xu ly loi doc.

  shuntVoltageV = ina3221RawToVoltage(rawShunt, INA3221_SHUNT_LSB_V); // Quy doi dien ap tren shunt.
  busVoltageV = ina3221RawToVoltage(rawBus, INA3221_BUS_LSB_V);       // Quy doi dien ap bus.
  currentA = shuntVoltageV / INA3221_SHUNT_RESISTOR_OHMS;             // Tinh dong dua tren dien tro shunt.
  return true;                                                        // Bao doc kenh thanh cong.
} // Ket thuc ham doc mot kenh INA3221.

void printIna3221Readings() // Doc va in 3 kenh INA3221 ra Serial Monitor.
{                           // Bat dau ham in gia tri INA3221.
  Serial.print(F("[INA3221] ")); // In tien to log INA3221.

  for (uint8_t channelIndex = 0; channelIndex < 3; ++channelIndex) // Duyet 3 kenh INA3221.
  {                                                               // Bat dau than vong lap doc kenh.
    float busVoltageV = 0.0f;                                     // Luu dien ap bus.
    float shuntVoltageV = 0.0f;                                   // Luu dien ap tren shunt.
    float currentA = 0.0f;                                        // Luu dong dien tinh toan.

    if (!readIna3221Channel(channelIndex, busVoltageV, shuntVoltageV, currentA)) // Kiem tra doc kenh co thanh cong khong.
    {                                                                           // Bat dau nhanh xu ly loi doc.
      Serial.print(F("read failed CH"));                                        // In thong bao loi doc.
      Serial.println(channelIndex + 1);                                         // In so kenh loi va xuong dong.
      ina3221Online = false;                                                    // Cho phep thu cau hinh lai lan sau.
      return;                                                                   // Thoat vi du lieu dong khong day du.
    } // Ket thuc nhanh xu ly loi doc.

    Serial.print(F("CH"));              // In nhan kenh.
    Serial.print(channelIndex + 1);     // In so kenh.
    Serial.print(F(" bus="));           // In nhan dien ap bus.
    Serial.print(busVoltageV, 3);       // In dien ap bus voi 3 chu so thap phan.
    Serial.print(F("V shunt="));        // In nhan dien ap shunt.
    Serial.print(shuntVoltageV * 1000.0f, 2); // In dien ap shunt theo mV.
    Serial.print(F("mV current="));     // In nhan dong dien.
    Serial.print(currentA, 3);          // In dong dien voi 3 chu so thap phan.
    Serial.print(F("A"));               // In don vi dong dien.

    if (channelIndex < 2)               // Kiem tra co can chen dau phan cach khong.
    {                                   // Bat dau nhanh chen dau phan cach.
      Serial.print(F(" | "));           // In dau phan cach giua cac kenh.
    } // Ket thuc nhanh chen dau phan cach.
  } // Ket thuc than vong lap doc kenh.

  Serial.println(); // Ket thuc dong log INA3221.
} // Ket thuc ham in gia tri INA3221.

void updateIna3221Readings()     // Doc INA3221 theo chu ky khong chan loop.
{                                // Bat dau ham cap nhat INA3221.
  const uint32_t now = millis(); // Lay thoi gian hien tai.

  if (now - ina3221LastSampleMs < INA3221_SAMPLE_INTERVAL_MS) // Kiem tra da den chu ky doc 0,2s chua.
  {                                                           // Bat dau nhanh chua den chu ky.
    return;                                                   // Thoat de loop tiep tuc xu ly viec khac.
  } // Ket thuc nhanh chua den chu ky.

  ina3221LastSampleMs = now; // Cap nhat moc thoi gian doc gan nhat.

  if (!ina3221Online)             // Kiem tra INA3221 da san sang chua.
  {                               // Bat dau nhanh thu khoi tao INA3221.
    ina3221Online = configureIna3221(); // Thu cau hinh lai khi chua thay thiet bi.
    if (!ina3221Online)                 // Kiem tra cau hinh lai co thanh cong khong.
    {                                   // Bat dau nhanh chua san sang.
      return;                           // Thoat, lan sau se thu lai.
    } // Ket thuc nhanh chua san sang.
  } // Ket thuc nhanh thu khoi tao INA3221.

  printIna3221Readings(); // Doc va in 3 kenh ra Serial Monitor.
} // Ket thuc ham cap nhat INA3221.

void drawOledStatusDot()                                                                        // Ve cham trang thai neu dang duoc phep hien.
{                                                                                               // Bat dau ham ve cham trang thai.
  if (oledStatusDotVisible)                                                                     // Kiem tra cham trang thai co dang hien khong.
  {                                                                                             // Bat dau nhanh ve cham trang thai.
    oled.drawDisc(OLED_STATUS_DOT_X, OLED_STATUS_DOT_Y, OLED_STATUS_DOT_RADIUS, U8G2_DRAW_ALL); // Ve hinh tron trang thai len buffer OLED.
  } // Ket thuc nhanh ve cham trang thai.
} // Ket thuc ham ve cham trang thai.

void drawOledDigits(uint8_t p1, uint8_t p2, uint8_t p3) // Ve ba nhom chu so len OLED.
{                                                       // Bat dau ham ve chu so.
  char p1Text[2];                                       // Tao bo dem chuoi cho nhom 1 mot chu so.
  char p2Text[3];                                       // Tao bo dem chuoi cho nhom 2 toi da hai chu so.
  char p3Text[3];                                       // Tao bo dem chuoi cho nhom 3 toi da hai chu so.

  snprintf(p1Text, sizeof(p1Text), "%u", p1); // Chuyen nhom 1 thanh chuoi.
  snprintf(p2Text, sizeof(p2Text), "%u", p2); // Chuyen nhom 2 thanh chuoi.
  snprintf(p3Text, sizeof(p3Text), "%u", p3); // Chuyen nhom 3 thanh chuoi.

  oled.setFontPosTop(); // Dat toa do font theo dinh tren de can vi tri de hon.

  oled.setFont(u8g2_font_logisoso26_tn);                   // Chon font nho cho hang tren.
  const int topY = 0;                                      // Dat toa do Y cua hang tren.
  const int p2Width = oled.getStrWidth(p2Text);            // Tinh chieu rong nhom 2.
  const int p2X = (128 - p2Width < 0) ? 0 : 128 - p2Width; // Can nhom 2 sat ben phai man hinh.
  oled.drawStr(0, topY, p1Text);                           // Ve nhom 1 o goc trai tren.
  oled.drawStr(p2X, topY, p2Text);                         // Ve nhom 2 o goc phai tren.

  oled.setFont(u8g2_font_logisoso38_tn);        // Chon font lon cho hang duoi.
  const int p3Width = oled.getStrWidth(p3Text); // Tinh chieu rong nhom 3.
  const int p3XRaw = (128 - p3Width) / 2;       // Tinh toa do X de can giua nhom 3.
  const int p3X = (p3XRaw < 0) ? 0 : p3XRaw;    // Gioi han X khong am neu chuoi qua rong.
  const int bottomY = 26;                       // Dat toa do Y cua hang duoi.
  oled.drawStr(p3X, bottomY, p3Text);           // Ve nhom 3 o hang duoi.
} // Ket thuc ham ve chu so.

void renderOledContent(bool logRender) // Ve lai noi dung OLED theo che do hien tai.
{                                      // Bat dau ham render OLED.
  oled.clearBuffer();                  // Xoa buffer ve cua OLED.
  oled.setDrawColor(1);                // Dat mau ve la trang/bat pixel.

  if (oledContentMode == OledContentMode::DIGITS)          // Kiem tra co can ve chu so khong.
  {                                                        // Bat dau nhanh ve chu so.
    drawOledDigits(oledDigitP1, oledDigitP2, oledDigitP3); // Ve cac chu so dang luu.
  } // Ket thuc nhanh ve chu so.

  drawOledStatusDot(); // Ve cham trang thai len buffer.
  oled.sendBuffer();   // Gui buffer da ve ra man hinh OLED.

#if SERIAL_DEBUG_ENABLE                             // Chi bien dich log render khi debug dang bat.
  if (logRender)                                    // Kiem tra lan render nay co can ghi log khong.
  {                                                 // Bat dau nhanh ghi log render.
    Serial.print(F("[OLED-RENDER] mode="));         // In tien to log render.
    if (oledContentMode == OledContentMode::DIGITS) // Kiem tra mode dang la chu so.
    {                                               // Bat dau nhanh log mode chu so.
      Serial.print(F("digits p1="));                // In nhan nhom 1.
      Serial.print(oledDigitP1);                    // In gia tri nhom 1.
      Serial.print(F(" p2="));                      // In nhan nhom 2.
      Serial.print(oledDigitP2);                    // In gia tri nhom 2.
      Serial.print(F(" p3="));                      // In nhan nhom 3.
      Serial.print(oledDigitP3);                    // In gia tri nhom 3.
    } // Ket thuc nhanh log mode chu so.
    else                        // Xu ly mode man hinh trong.
    {                           // Bat dau nhanh log mode trong.
      Serial.print(F("clear")); // In ten mode trong.
    } // Ket thuc nhanh log mode trong.
    Serial.print(F(" dot="));                                  // In nhan trang thai cham.
    Serial.println(oledStatusDotVisible ? F("on") : F("off")); // In trang thai cham va xuong dong.
  } // Ket thuc nhanh ghi log render.
#endif // Ket thuc khoi log render co dieu kien.
} // Ket thuc ham render OLED.

void oledClearScreen()                      // Chuyen OLED ve trang thai man hinh trong.
{                                           // Bat dau ham xoa OLED.
  oledContentMode = OledContentMode::CLEAR; // Dat mode hien thi la trong.
  renderOledContent(true);                  // Ve lai OLED va ghi log.
} // Ket thuc ham xoa OLED.

bool isAllDigits(const char *s, size_t len) // Kiem tra chuoi co toan chu so khong.
{                                           // Bat dau ham kiem tra chu so.
  for (size_t i = 0; i < len; ++i)          // Duyet tung ky tu trong chuoi.
  {                                         // Bat dau than vong lap.
    if (s[i] < '0' || s[i] > '9')           // Kiem tra ky tu hien tai co nam ngoai '0' den '9' khong.
    {                                       // Bat dau nhanh gap ky tu khong phai so.
      return false;                         // Bao chuoi khong phai toan chu so.
    } // Ket thuc nhanh gap ky tu khong phai so.
  } // Ket thuc than vong lap.
  return true; // Bao chuoi chi gom chu so.
} // Ket thuc ham kiem tra chu so.

void renderOledDigits(uint8_t p1, uint8_t p2, uint8_t p3) // Luu va hien thi bo ba gia tri len OLED.
{                                                         // Bat dau ham hien thi chu so.
  oledContentMode = OledContentMode::DIGITS;              // Dat mode hien thi la chu so.
  oledDigitP1 = p1;                                       // Luu gia tri nhom 1.
  oledDigitP2 = p2;                                       // Luu gia tri nhom 2.
  oledDigitP3 = p3;                                       // Luu gia tri nhom 3.

  renderOledContent(true); // Ve lai OLED va ghi log.
} // Ket thuc ham hien thi chu so.

void updateOledStatusDot()       // Cap nhat cham trang thai OLED theo chu ky nhap nhay.
{                                // Bat dau ham cap nhat cham trang thai.
  const uint32_t now = millis(); // Lay thoi gian hien tai cua Arduino.

  if (now - oledStatusDotLastToggleMs < OLED_STATUS_DOT_INTERVAL_MS) // Kiem tra da den luc doi trang thai cham chua.
  {                                                                  // Bat dau nhanh chua den luc doi.
    return;                                                          // Thoat ham de giu trang thai hien tai.
  } // Ket thuc nhanh chua den luc doi.

  oledStatusDotLastToggleMs = now;              // Cap nhat moc thoi gian doi trang thai.
  oledStatusDotVisible = !oledStatusDotVisible; // Dao trang thai hien/an cua cham.
  renderOledContent(false);                     // Ve lai OLED nhung khong log render chi tiet.

#if SERIAL_DEBUG_ENABLE                                      // Chi bien dich log blink khi debug dang bat.
  Serial.print(F("[OLED-BLINK] dot="));                      // In tien to log cham nhap nhay.
  Serial.println(oledStatusDotVisible ? F("on") : F("off")); // In trang thai cham moi.
#endif                                                       // Ket thuc khoi log blink co dieu kien.
} // Ket thuc ham cap nhat cham trang thai.

bool triggerOledServoCycle()                                                      // Kich hoat chu ky servo OLED.
{                                                                                 // Bat dau ham kich servo OLED.
  if (oledServoState != OledServoState::IDLE)                                     // Kiem tra servo OLED co dang ban khong.
  {                                                                               // Bat dau nhanh tu choi lenh khi dang chay.
    logLine(F("[WARN] "), F("ServoOled cycle already running, command ignored")); // Ghi log lenh bi bo qua.
    return false;                                                                 // Bao khong khoi dong duoc chu ky moi.
  } // Ket thuc nhanh tu choi lenh khi dang chay.

  servoOled.writeMicroseconds(SERVO_OLED_MAX_PULSE_US); // Gui xung lon nhat de day servo OLED tang toc.
  oledServoState = OledServoState::BOOSTING;            // Chuyen servo OLED sang trang thai tang toc.
  oledServoStartMs = millis();                          // Luu moc thoi gian bat dau chu ky.
#if SERIAL_DEBUG_ENABLE                                 // Chi bien dich log kich servo khi debug dang bat.
  Serial.print(F("[ACT] ServoOled max accel pulse "));  // In tien to log xung tang toc.
  Serial.print(SERVO_OLED_MAX_PULSE_US);                // In gia tri xung tang toc.
  Serial.println(F(" us"));                             // In don vi micro giay va xuong dong.
#endif                                                  // Ket thuc khoi log kich servo co dieu kien.
  return true;                                          // Bao da khoi dong chu ky servo OLED.
} // Ket thuc ham kich servo OLED.

void updateOledServoCycle()      // Cap nhat chu ky servo OLED khong chan loop.
{                                // Bat dau ham cap nhat servo OLED.
  const uint32_t now = millis(); // Lay thoi gian hien tai cua Arduino.

  if (oledServoState == OledServoState::BOOSTING)      // Kiem tra servo OLED dang o buoc tang toc khong.
  {                                                    // Bat dau nhanh xu ly buoc tang toc.
    if (now - oledServoStartMs >= SERVO_OLED_BOOST_MS) // Kiem tra da het thoi gian tang toc chua.
    {                                                  // Bat dau nhanh chuyen sang goc dich.
      servoOled.write(SERVO_OLED_TARGET_ANGLE);        // Dat servo OLED den goc dich.
      oledServoState = OledServoState::WAITING_RETURN; // Chuyen sang trang thai cho quay ve.
      oledServoStartMs = now;                          // Cap nhat moc thoi gian cho buoc giu goc dich.
#if SERIAL_DEBUG_ENABLE                                // Chi bien dich log goc dich khi debug dang bat.
      Serial.print(F("[ACT] ServoOled target "));      // In tien to log goc dich.
      Serial.print(SERVO_OLED_TARGET_ANGLE);           // In goc dich cua servo OLED.
      Serial.println(F(" deg"));                       // In don vi do va xuong dong.
#endif                                                 // Ket thuc khoi log goc dich co dieu kien.
    } // Ket thuc nhanh chuyen sang goc dich.
  } // Ket thuc nhanh xu ly buoc tang toc.
  else if (oledServoState == OledServoState::WAITING_RETURN)  // Kiem tra servo OLED dang giu truoc khi quay ve khong.
  {                                                           // Bat dau nhanh xu ly buoc quay ve.
    if (now - oledServoStartMs >= SERVO_OLED_TARGET_HOLD_MS)  // Kiem tra da het thoi gian giu goc dich chua.
    {                                                         // Bat dau nhanh dua servo ve home.
      servoOled.writeMicroseconds(SERVO_OLED_MIN_PULSE_US);   // Gui xung nho nhat de dua servo OLED ve vi tri ban dau.
      oledServoState = OledServoState::IDLE;                  // Chuyen servo OLED ve trang thai ranh.
      logLine(F("[ACT] "), F("ServoOled returned to 0 deg")); // Ghi log da quay ve home.
    } // Ket thuc nhanh dua servo ve home.
  } // Ket thuc nhanh xu ly buoc quay ve.
} // Ket thuc ham cap nhat servo OLED.

uint16_t parseCommandNumber(const char *cmd, size_t len) // Chuyen lenh toan chu so thanh so nguyen.
{                                                        // Bat dau ham chuyen lenh thanh so.
  uint16_t value = 0;                                    // Luu gia tri lenh dang tinh.
  for (size_t i = 0; i < len; ++i)                       // Duyet tung chu so trong lenh.
  {                                                      // Bat dau than vong lap.
    value = static_cast<uint16_t>(value * 10 + (cmd[i] - '0')); // Tich luy gia tri thap phan.
  } // Ket thuc than vong lap.
  return value; // Tra ve gia tri lenh.
} // Ket thuc ham chuyen lenh thanh so.

void processCommand(const char *cmd) // Xu ly lenh Serial da tach hoan chinh.
{                                    // Bat dau ham xu ly lenh.
  const size_t len = strlen(cmd);    // Tinh do dai lenh.

#if SERIAL_DEBUG_ENABLE                // Chi bien dich log lenh khi debug dang bat.
  Serial.print(F("[PROCESS] cmd=\"")); // In tien to log lenh.
  Serial.print(cmd);                   // In noi dung lenh.
  Serial.println(F("\""));             // Dong ngoac kep va xuong dong.
#endif                                 // Ket thuc khoi log lenh co dieu kien.

  if (len == 2 && strcmp(cmd, "44") == 0)                         // Kiem tra lenh 44 de xoa OLED.
  {                                                               // Bat dau nhanh xu ly lenh xoa OLED.
    logLine(F("[OLED-COM] "), F("command=44 accepted -> clear")); // Ghi log chap nhan lenh xoa OLED.
    oledClearScreen();                                            // Xoa noi dung OLED.
    return;                                                       // Ket thuc xu ly lenh.
  } // Ket thuc nhanh xu ly lenh xoa OLED.

  if (len == 5)                                                                 // Kiem tra lenh 5 ky tu de hien thi chu so.
  {                                                                             // Bat dau nhanh xu ly lenh 5 ky tu.
    if (!isAllDigits(cmd, len))                                                 // Kiem tra lenh 5 ky tu co toan chu so khong.
    {                                                                           // Bat dau nhanh tu choi lenh khong phai so.
      logLine(F("[OLED-COM] "), F("5-char command rejected (not all digits)")); // Ghi log tu choi lenh OLED.
      return;                                                                   // Ket thuc xu ly lenh.
    } // Ket thuc nhanh tu choi lenh khong phai so.

    const uint8_t p1 = static_cast<uint8_t>(cmd[0] - '0');                         // Tach chu so dau thanh nhom 1.
    const uint8_t p2 = static_cast<uint8_t>((cmd[1] - '0') * 10 + (cmd[2] - '0')); // Tach hai chu so tiep theo thanh nhom 2.
    const uint8_t p3 = static_cast<uint8_t>((cmd[3] - '0') * 10 + (cmd[4] - '0')); // Tach hai chu so cuoi thanh nhom 3.

    logLine(F("[OLED-COM] "), F("5-digit command accepted")); // Ghi log chap nhan lenh hien thi.
    renderOledDigits(p1, p2, p3);                             // Hien thi ba nhom gia tri len OLED.
    return;                                                   // Ket thuc xu ly lenh.
  } // Ket thuc nhanh xu ly lenh 5 ky tu.

  if (isAllDigits(cmd, len))                                        // Kiem tra lenh co phai dang so khong.
  {                                                                 // Bat dau nhanh xu ly lenh so.
    const uint16_t commandNumber = parseCommandNumber(cmd, len);    // Chuyen lenh sang so nguyen.
    switch (commandNumber)                                          // Chon hanh dong theo ma lenh.
    {                                                               // Bat dau switch lenh so.
    case 3:                                                         // Lenh 3 dua FontCam ve home.
      servoFontCam.write(SERVO_FONTCAM_HOME_ANGLE);                 // Dat servo FontCam ve goc home.
      logLine(F("[ACT] "), F("ServoFontCam -> 0 deg"));             // Ghi log FontCam ve home.
      return;                                                       // Ket thuc xu ly lenh.
    case 4:                                                         // Lenh 4 dua FontCam den goc lam viec.
      servoFontCam.write(SERVO_FONTCAM_WORK_ANGLE);                 // Dat servo FontCam den goc lam viec.
      logLine(F("[ACT] "), F("ServoFontCam -> 45 deg"));            // Ghi log FontCam den goc lam viec.
      return;                                                       // Ket thuc xu ly lenh.
    case 5:                                                         // Lenh 5 dua RearCam ve home.
      logLine(F("[CMD] "), F("5 -> ServoRearCam HOME 0 deg"));      // Ghi log y nghia lenh 5.
      setRearCamAngle(SERVO_REARCAM_HOME_ANGLE);                    // Dat RearCam ve home.
      return;                                                       // Ket thuc xu ly lenh.
    case 6:                                                         // Lenh 6 dua RearCam den goc lam viec.
      logLine(F("[CMD] "), F("6 -> ServoRearCam WORK 45 deg"));     // Ghi log y nghia lenh 6.
      setRearCamAngle(SERVO_REARCAM_WORK_ANGLE);                    // Dat RearCam den goc lam viec.
      return;                                                       // Ket thuc xu ly lenh.
    case 8:                                                         // Lenh 8 kich chu ky servo OLED.
      triggerOledServoCycle();                                      // Kich chu ky servo OLED.
      return;                                                       // Ket thuc xu ly lenh.
    case 9:                                                         // Lenh 9 bat relay D9/B+.
      relaySet(PIN_RELAY_BPLUS, F("RelayD9_Bplus"), true);          // Bat relay B+ tren D9.
      return;                                                       // Ket thuc xu ly lenh.
    case 10:                                                        // Lenh 10 tat relay D9/B+.
      relaySet(PIN_RELAY_BPLUS, F("RelayD9_Bplus"), false);         // Tat relay B+ tren D9.
      return;                                                       // Ket thuc xu ly lenh.
    case 11:                                                        // Lenh 11 bat relay D10.
      relaySet(PIN_RELAY_D10, F("RelayD10"), true);                 // Bat relay D10.
      return;                                                       // Ket thuc xu ly lenh.
    case 12:                                                        // Lenh 12 tat relay D10.
      relaySet(PIN_RELAY_D10, F("RelayD10"), false);                // Tat relay D10.
      return;                                                       // Ket thuc xu ly lenh.
    case 13:                                                        // Lenh 13 bat relay D11.
      relaySet(PIN_RELAY_D11, F("RelayD11"), true);                 // Bat relay D11.
      return;                                                       // Ket thuc xu ly lenh.
    case 14:                                                        // Lenh 14 tat relay D11.
      relaySet(PIN_RELAY_D11, F("RelayD11"), false);                // Tat relay D11.
      return;                                                       // Ket thuc xu ly lenh.
    case 15:                                                        // Lenh 15 bat relay D12.
      relaySet(PIN_RELAY_D12, F("RelayD12"), true);                 // Bat relay D12.
      return;                                                       // Ket thuc xu ly lenh.
    case 16:                                                        // Lenh 16 tat relay D12.
      relaySet(PIN_RELAY_D12, F("RelayD12"), false);                // Tat relay D12.
      return;                                                       // Ket thuc xu ly lenh.
    default:                                                        // Xu ly lenh so khong ho tro.
      logLine(F("[WARN] "), F("Unknown numeric command"));          // Ghi log lenh so khong biet.
      return;                                                       // Ket thuc xu ly lenh.
    } // Ket thuc switch lenh so.
  } // Ket thuc nhanh xu ly lenh so.

  logLine(F("[WARN] "), F("Unsupported command length or format")); // Ghi log lenh khong dung dinh dang.
} // Ket thuc ham xu ly lenh.

void finalizeRxCommand(const char *reason) // Ket thuc lenh Serial dang nhan va dua di xu ly.
{                                          // Bat dau ham ket thuc lenh nhan.
  if (rxLen == 0)                          // Kiem tra bo dem co rong khong.
  {                                        // Bat dau nhanh bo qua bo dem rong.
    return;                                // Thoat ham vi khong co lenh.
  } // Ket thuc nhanh bo qua bo dem rong.

  rxBuffer[rxLen] = '\0'; // Them ky tu ket thuc chuoi C vao bo dem.

#if SERIAL_DEBUG_ENABLE                // Chi bien dich log lenh nhan khi debug dang bat.
  Serial.print(F("[RX-CMD] source=")); // In tien to nguon ket thuc lenh.
  Serial.print(reason);                // In ly do ket thuc lenh.
  Serial.print(F(" cmd=\""));          // In nhan noi dung lenh.
  Serial.print(rxBuffer);              // In lenh da nhan.
  Serial.println(F("\""));             // Dong ngoac kep va xuong dong.
#endif                                 // Ket thuc khoi log lenh nhan co dieu kien.

  processCommand(rxBuffer); // Xu ly lenh da nhan.
  rxLen = 0;                // Xoa do dai bo dem de san sang nhan lenh moi.
} // Ket thuc ham ket thuc lenh nhan.

void readSerialCommands()                                    // Doc va tach lenh tu cong Serial.
{                                                            // Bat dau ham doc Serial.
  while (Serial.available() > 0)                             // Lap khi con byte dang cho trong Serial.
  {                                                          // Bat dau than vong lap doc Serial.
    const uint8_t raw = static_cast<uint8_t>(Serial.read()); // Doc mot byte tu Serial.
    logRxByte(raw);                                          // Ghi log byte vua doc.

    if (raw == '\r' || raw == '\n')   // Kiem tra byte co phai dau ket thuc dong khong.
    {                                 // Bat dau nhanh xu ly dau ket thuc dong.
      finalizeRxCommand("delimiter"); // Ket thuc lenh bang dau phan tach.
      continue;                       // Bo qua phan them byte vao bo dem.
    } // Ket thuc nhanh xu ly dau ket thuc dong.

    if (rxLen >= RX_BUFFER_SIZE - 1)                                            // Kiem tra bo dem co sap tran khong.
    {                                                                           // Bat dau nhanh xu ly tran bo dem.
      logLine(F("[WARN] "), F("RX buffer overflow, dropping current command")); // Ghi log tran bo dem.
      rxLen = 0;                                                                // Xoa lenh dang nhan de tranh ghi qua mang.
    } // Ket thuc nhanh xu ly tran bo dem.

    rxBuffer[rxLen++] = static_cast<char>(raw); // Them byte vua doc vao bo dem lenh.
    lastRxByteMs = millis();                    // Cap nhat thoi diem nhan byte gan nhat.
  } // Ket thuc than vong lap doc Serial.

  if (rxLen > 0)                             // Kiem tra co lenh dang nhan dang do khong.
  {                                          // Bat dau nhanh kiem tra timeout lenh.
    const uint32_t now = millis();           // Lay thoi gian hien tai cua Arduino.
    if (now - lastRxByteMs >= RX_TIMEOUT_MS) // Kiem tra da qua timeout tu byte cuoi chua.
    {                                        // Bat dau nhanh ket thuc lenh bang timeout.
      finalizeRxCommand("timeout");          // Ket thuc lenh vi het thoi gian cho.
    } // Ket thuc nhanh ket thuc lenh bang timeout.
  } // Ket thuc nhanh kiem tra timeout lenh.
} // Ket thuc ham doc Serial.

void setup()                     // Ham khoi tao chay mot lan khi Arduino boot.
{                                // Bat dau ham setup.
  Serial.begin(SERIAL_BAUDRATE); // Khoi dong Serial voi baudrate da dinh nghia.
  Wire.begin();                  // Khoi dong bus I2C dung chung cho OLED va INA3221.

  configureRelayOutput(PIN_RELAY_BPLUS); // Tat relay B+ D9 ngay khi khoi dong.
  configureRelayOutput(PIN_RELAY_D10);   // Tat relay D10 ngay khi khoi dong.
  configureRelayOutput(PIN_RELAY_D11);   // Tat relay D11 ngay khi khoi dong.
  configureRelayOutput(PIN_RELAY_D12);   // Tat relay D12 ngay khi khoi dong.

  servoOled.attach(PIN_SERVO_OLED, SERVO_OLED_MIN_PULSE_US, SERVO_OLED_MAX_PULSE_US); // Gan servo OLED vao chan D3 voi dai xung rieng.
  servoFontCam.attach(PIN_SERVO_FONTCAM);                                             // Gan servo camera truoc vao chan D2.
  servoRearCam.attach(PIN_SERVO_REARCAM);                                             // Gan servo camera sau vao chan D8.
  logLine(F("[SETUP] "), F("ServoRearCam attached on D8"));                           // Ghi log servo camera sau da gan.

  servoOled.write(SERVO_OLED_MIN_ANGLE);                // Dua servo OLED ve goc ban dau.
  servoFontCam.write(SERVO_FONTCAM_HOME_ANGLE);         // Dua servo camera truoc ve home.
  logLine(F("[SETUP] "), F("ServoFontCam home=0 deg")); // Ghi log servo camera truoc da ve home.
  setRearCamAngle(SERVO_REARCAM_HOME_ANGLE);            // Dua servo camera sau ve home.

  configureOledForCamera();             // Khoi tao cau hinh man hinh OLED.
  oledStatusDotVisible = true;          // Bat hien cham trang thai luc khoi dong.
  oledStatusDotLastToggleMs = millis(); // Luu moc thoi gian nhap nhay ban dau.
  oledClearScreen();                    // Xoa man hinh OLED sau khi khoi tao.
  ina3221Online = configureIna3221();   // Khoi tao INA3221 neu da gan tren bus I2C.
  ina3221LastSampleMs = millis();       // Bat dau chu ky doc INA3221 sau 0,2 giay.

  logLine(F("[ACT] "), F("Boot completed: Serial 9600, relays OFF, 3 servos attached")); // Ghi log boot hoan tat.
} // Ket thuc ham setup.

void loop()               // Ham lap chinh cua firmware.
{                         // Bat dau ham loop.
  readSerialCommands();   // Doc va xu ly lenh Serial neu co.
  updateOledServoCycle(); // Cap nhat chu ky servo OLED khong chan chuong trinh.
  updateOledStatusDot();  // Cap nhat cham trang thai OLED.
  updateIna3221Readings(); // Doc va in INA3221 moi 0,2 giay.
} // Ket thuc ham loop.
