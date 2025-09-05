#pragma once
#include <Arduino.h>

// ====== הגדרות דגימה כלליות ======
static constexpr uint32_t SAMPLE_HZ = 50;
static constexpr uint32_t SAMPLE_MS = 1000 / SAMPLE_HZ;

// ====== הגדרות כתיבת קובץ (SD_MMC) ======
static constexpr char  LOG_PATH[] = "/run.csv";  // נתיב קובץ ברירת מחדל
// שים לב: אין צורך להגדיר SD_CS_PIN ב־SD_MMC

// ====== חיישן מהירות (Hall Effect) ======
static constexpr int   SPEED_PIN       = 4;
static constexpr float WHEEL_DIAM_M    = 0.116f;
static constexpr float WHEEL_CIRCUM_M  = 3.1415926f * WHEEL_DIAM_M;
static constexpr int   MAGNETS_PER_REV = 1;
static constexpr float DIST_PER_PULSE_M= WHEEL_CIRCUM_M / MAGNETS_PER_REV;
static constexpr float DIST_PER_REV_M  = WHEEL_CIRCUM_M;

// ====== חיישני רגליים (TOF) ======
#define FOOTLIFT_CONN_UART 1
#if FOOTLIFT_CONN_UART
static constexpr uint8_t FOOT_LEFT_RX   = 15;// לבן
static constexpr uint8_t FOOT_LEFT_TX   = 16;// ירוק
static constexpr uint8_t FOOT_RIGHT_RX  = 17;
static constexpr uint8_t FOOT_RIGHT_TX  = 18;
#else
static constexpr uint8_t TOF_SDA        = 21;
static constexpr uint8_t TOF_SCL        = 22;
static constexpr uint8_t FOOT_LEFT_XSHUT  = 32;
static constexpr uint8_t FOOT_RIGHT_XSHUT = 33;
#endif

#define ENABLE_UART_TOF 1

// ====== חיישני לחץ (HX711) ======
#define ENABLE_HX711 0
static constexpr uint8_t HANDL_DOUT = 32;
static constexpr uint8_t HANDL_SCK  = 33;
static constexpr uint8_t HANDR_DOUT = 25;
static constexpr uint8_t HANDR_SCK  = 26;
static constexpr float   HANDL_KG_PER_UNIT = 1.0f;
static constexpr float   HANDR_KG_PER_UNIT = 1.0f;

// ====== ניתוח ומידע נוסף ======
#define ONLINE_FOOTLIFT_COUNT 0
#define ENABLE_LOGGER 1
#define LOG_FOOTLIFT_MM 1     // שמירת ערכי המרחק מהרגליים
#define LOG_SPEED_DATA 1      // שמירת מהירות
#define LOG_PRESSURE_DATA 0   // לא לשמור לחץ יד

// ====== Debug ======
static constexpr bool DEBUG_SERIAL = true;
