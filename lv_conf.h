/*
  lv_conf.h แบบย่อสำหรับ LVGL 9.1+ บน ESP32
  วางไว้ในโฟลเดอร์ Arduino/libraries (ข้างๆ โฟลเดอร์ lvgl ไม่ใช่ข้างใน)
  ค่าที่ไม่ได้ระบุที่นี่ LVGL จะใช้ค่าเริ่มต้นของมันเอง
*/

#if 1   /* เปลี่ยนเป็น 1 เพื่อเปิดใช้ไฟล์นี้ */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16

/* ให้ LVGL จองหน่วยความจำจาก heap ตอนรัน (malloc) แทนการสำรอง 64 KB ไว้ใน DRAM แบบ static
   ไม่งั้นลิงก์ไม่ผ่านเมื่อใช้ร่วมกับ Bluetooth (error: region dram0_0_seg overflowed) */
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB

#define LV_USE_LOG 0

/* ฟอนต์ที่ sketch ใช้ */
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_24 1

#endif /* LV_CONF_H */

#endif
