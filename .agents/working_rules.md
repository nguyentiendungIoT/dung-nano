# Working Rules

## Quy Tac Chinh

- Giao tiep voi nguoi dung bang tieng Viet neu khong co yeu cau khac.
- Khong tu y sua firmware khi nguoi dung chi yeu cau tao tai lieu hoac file nho.
- Truoc moi thay doi code, doc file dang sua va giai thich ngan se sua gi.
- Giu thay doi nho, dung pham vi yeu cau.
- Khong xoa hoac rollback thay doi cua nguoi dung neu chua duoc yeu cau.

## Khi Sua Firmware

- Uu tien code don gian, de build tren Arduino Nano RAM 2 KB.
- Khong dung `delay()` neu logic can chay song song; uu tien `millis()`.
- Khong them thu vien moi neu chua can thiet.
- Sau khi sua firmware, build bang PlatformIO va bao ro ket qua.

## Khi Tao Tri Nho Agent

- Chi tao/cap nhat file Markdown nhu `MEMORY.md` hoac `.agents/*.md`.
- Khong tao `.cpp`, `.h`, EEPROM logic, script, hoac thay doi firmware neu nguoi
  dung chi noi "agent co tri nho dai han".
- Ghi ro quyet dinh, lenh build, pin map, giao thuc Serial va cac canh bao quan
  trong de phien sau doc nhanh.
