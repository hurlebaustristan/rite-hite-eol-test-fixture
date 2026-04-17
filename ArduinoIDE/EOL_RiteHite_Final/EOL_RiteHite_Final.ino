#include <Arduino.h>
#include <Wire.h>
#include <cstring>
#include <cstdio>

// ---------------------- Pins (from your notes) ----------------------
static constexpr int PIN_I2C_SDA = 15;
static constexpr int PIN_I2C_SCL = 16;

static constexpr int PIN_IOBUS_RST_N = 9;   // active-low reset shared on Q3/Q4
static constexpr int PIN_Q3_INT_N    = 17;  // PI4 Q3 INT#
static constexpr int PIN_Q4_INT_N    = 18;  // PI4 Q4 INT#
static constexpr int PIN_ADC_ALRT    = 8;   // ADS1015 ALERT/RDY (optional)

static constexpr int PIN_CELL_TX = 10;      // ESP32 -> cell module RX (your note)
static constexpr int PIN_CELL_RX = 11;      // ESP32 <- cell module TX (your note)

// ---------------------- I2C addresses ----------------------
// ADS1015 with ADDR tied to GND => 0x48 (7-bit)
static constexpr uint8_t I2C_ADDR_ADS = 0x48;

// PI4IOE5V6416 base 0x20 + ADDR bit => 0x20 or 0x21
// We'll auto-detect both at runtime.
static constexpr uint8_t PI4_ADDR0 = 0x20;
static constexpr uint8_t PI4_ADDR1 = 0x21;

// ---------------------- Build options ----------------------
static constexpr bool ENABLE_CELL_UART_BRIDGE = false; // set true if you want Serial <-> Serial1 bridge
static constexpr bool ENABLE_RELAY_OUTPUTS_BY_DEFAULT = false; // safety: must be enabled via command
static constexpr bool PROTO_ENABLE = true;
static constexpr bool PROTO_ONLY_MODE = true;
static constexpr uint32_t PROTO_BAUD = 115200;
static constexpr int PROTO_RX_PIN = 11;
static constexpr int PROTO_TX_PIN = 10;

// ---------------------- Fixture protocol tokens ----------------------
static constexpr char TOKEN_PING[] = "PING";
static constexpr char TOKEN_PONG[] = "PONG";
static constexpr char TOKEN_READY[] = "READY";
static constexpr char TOKEN_OK[] = "OK";
static constexpr char TOKEN_FAULTD[] = "FAULTD";
static constexpr char TOKEN_MOVINGON[] = "MOVINGON";
static constexpr char TOKEN_ANALOG[] = "ANALOG";
static constexpr char TOKEN_STOP[] = "STOP";
static constexpr char TOKEN_OK_ANALOG[] = "OK_ANALOG";
static constexpr char TOKEN_OK_STOP[] = "OK_STOP";
static constexpr char TOKEN_DO1_OFF[] = "DO1_OFF";
static constexpr char TOKEN_DO1_ON[] = "DO1_ON";
static constexpr char TOKEN_DO2_OFF[] = "DO2_OFF";
static constexpr char TOKEN_DO2_ON[] = "DO2_ON";
static constexpr char TOKEN_K1_OFF[] = "K1_OFF";
static constexpr char TOKEN_K1_ON[] = "K1_ON";
static constexpr char TOKEN_K2_OFF[] = "K2_OFF";
static constexpr char TOKEN_K2_ON[] = "K2_ON";
static constexpr char TOKEN_OK_DO1_OFF[] = "OK_DO1_OFF";
static constexpr char TOKEN_OK_DO1_ON[] = "OK_DO1_ON";
static constexpr char TOKEN_OK_DO2_OFF[] = "OK_DO2_OFF";
static constexpr char TOKEN_OK_DO2_ON[] = "OK_DO2_ON";
static constexpr char TOKEN_OK_K1_OFF[] = "OK_K1_OFF";
static constexpr char TOKEN_OK_K1_ON[] = "OK_K1_ON";
static constexpr char TOKEN_OK_K2_OFF[] = "OK_K2_OFF";
static constexpr char TOKEN_OK_K2_ON[] = "OK_K2_ON";
static constexpr char TOKEN_DO_READ_OK[] = "DO_READ_OK";
static constexpr char TOKEN_RELAYS_READ_OK[] = "RELAYS_READ_OK";
static constexpr char TOKEN_DI_BEGIN[] = "DI_BEGIN";
static constexpr char TOKEN_OK_DI_BEGIN[] = "OK_DI_BEGIN";
static constexpr char TOKEN_DI_READ_OK[] = "DI_READ_OK";
static constexpr char TOKEN_AI_BEGIN[] = "AI_BEGIN";
static constexpr char TOKEN_OK_AI_BEGIN[] = "OK_AI_BEGIN";
static constexpr char TOKEN_AI_READ_OK[] = "AI_READ_OK";
static constexpr char TOKEN_BTN_BEGIN[] = "BTN_BEGIN";
static constexpr char TOKEN_OK_BTN_BEGIN[] = "OK_BTN_BEGIN";
static constexpr char TOKEN_BTN_USR0_PRESS[] = "BTN_USR0_PRESS";
static constexpr char TOKEN_OK_BTN_USR0_PRESS[] = "OK_BTN_USR0_PRESS";
static constexpr char TOKEN_NG_BTN_USR0_PRESS[] = "BTN_USR0_PRESS_HIGH";
static constexpr char TOKEN_BTN_USR0_RELEASE[] = "BTN_USR0_RELEASE";
static constexpr char TOKEN_OK_BTN_USR0_RELEASE[] = "OK_BTN_USR0_RELEASE";
static constexpr char TOKEN_NG_BTN_USR0_RELEASE[] = "BTN_USR0_RELEASE_LOW";
static constexpr char TOKEN_BTN_PNL0_PRESS[] = "BTN_PNL0_PRESS";
static constexpr char TOKEN_OK_BTN_PNL0_PRESS[] = "OK_BTN_PNL0_PRESS";
static constexpr char TOKEN_NG_BTN_PNL0_PRESS[] = "BTN_PNL0_PRESS_HIGH";
static constexpr char TOKEN_BTN_PNL0_RELEASE[] = "BTN_PNL0_RELEASE";
static constexpr char TOKEN_OK_BTN_PNL0_RELEASE[] = "OK_BTN_PNL0_RELEASE";
static constexpr char TOKEN_NG_BTN_PNL0_RELEASE[] = "BTN_PNL0_RELEASE_LOW";
static constexpr char TOKEN_BTN_PNL1_PRESS[] = "BTN_PNL1_PRESS";
static constexpr char TOKEN_OK_BTN_PNL1_PRESS[] = "OK_BTN_PNL1_PRESS";
static constexpr char TOKEN_NG_BTN_PNL1_PRESS[] = "BTN_PNL1_PRESS_HIGH";
static constexpr char TOKEN_BTN_PNL1_RELEASE[] = "BTN_PNL1_RELEASE";
static constexpr char TOKEN_OK_BTN_PNL1_RELEASE[] = "OK_BTN_PNL1_RELEASE";
static constexpr char TOKEN_NG_BTN_PNL1_RELEASE[] = "BTN_PNL1_RELEASE_LOW";
static constexpr char TOKEN_BTN_PNL2_PRESS[] = "BTN_PNL2_PRESS";
static constexpr char TOKEN_OK_BTN_PNL2_PRESS[] = "OK_BTN_PNL2_PRESS";
static constexpr char TOKEN_NG_BTN_PNL2_PRESS[] = "BTN_PNL2_PRESS_HIGH";
static constexpr char TOKEN_BTN_PNL2_RELEASE[] = "BTN_PNL2_RELEASE";
static constexpr char TOKEN_OK_BTN_PNL2_RELEASE[] = "OK_BTN_PNL2_RELEASE";
static constexpr char TOKEN_NG_BTN_PNL2_RELEASE[] = "BTN_PNL2_RELEASE_LOW";
static constexpr char TOKEN_BTN_PNL3_PRESS[] = "BTN_PNL3_PRESS";
static constexpr char TOKEN_OK_BTN_PNL3_PRESS[] = "OK_BTN_PNL3_PRESS";
static constexpr char TOKEN_NG_BTN_PNL3_PRESS[] = "BTN_PNL3_PRESS_HIGH";
static constexpr char TOKEN_BTN_PNL3_RELEASE[] = "BTN_PNL3_RELEASE";
static constexpr char TOKEN_OK_BTN_PNL3_RELEASE[] = "OK_BTN_PNL3_RELEASE";
static constexpr char TOKEN_NG_BTN_PNL3_RELEASE[] = "BTN_PNL3_RELEASE_LOW";
static constexpr char TOKEN_BTN_READ_OK[] = "BTN_READ_OK";
static constexpr char TOKEN_LED_BEGIN[] = "LED_BEGIN";
static constexpr char TOKEN_OK_LED_BEGIN[] = "OK_LED_BEGIN";
static constexpr char TOKEN_LED_ALL_ON[] = "LED_ALL_ON";
static constexpr char TOKEN_OK_LED_ALL_ON[] = "OK_LED_ALL_ON";
static constexpr char TOKEN_LED_ROTATE_250[] = "LED_ROTATE_250";
static constexpr char TOKEN_OK_LED_ROTATE_250[] = "OK_LED_ROTATE_250";
static constexpr char TOKEN_LED_STOP_ALL_ON[] = "LED_STOP_ALL_ON";
static constexpr char TOKEN_OK_LED_STOP_ALL_ON[] = "OK_LED_STOP_ALL_ON";
static constexpr char TOKEN_LED_VISUAL_OK[] = "LED_VISUAL_OK";

// ---------------------- Analog conversion constants ----------------------
static constexpr uint32_t ANALOG_STREAM_PERIOD_MS = 200;
static constexpr uint8_t ANALOG_CHANNEL_COUNT = 4;
static constexpr float PORT_VOLTAGE_SCALE[ANALOG_CHANNEL_COUNT] = {1.0f, 1.0f, 5.1f, 5.1f};
static constexpr bool PORT_IS_CURRENT_LOOP[ANALOG_CHANNEL_COUNT] = {true, true, false, false};
static constexpr float CURRENT_LOOP_SHUNT_OHMS[ANALOG_CHANNEL_COUNT] = {150.0f, 150.0f, 0.0f, 0.0f};
static constexpr float PORT23_DIVIDER_OHMS = 10200.0f;
static constexpr float PORT_ZERO_CLAMP_V = 0.015f;

// ---------------------- Helpers: I2C ----------------------
static bool i2cWrite8(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  return (Wire.endTransmission(true) == 0);
}

static bool i2cRead8(uint8_t addr, uint8_t reg, uint8_t &valOut) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  uint8_t n = Wire.requestFrom((int)addr, 1, (int)true);
  if (n != 1) return false;
  valOut = Wire.read();
  return true;
}

static bool i2cWrite16(uint8_t addr, uint8_t reg, uint16_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write((uint8_t)((val >> 8) & 0xFF)); // MSB first
  Wire.write((uint8_t)(val & 0xFF));        // LSB
  return (Wire.endTransmission(true) == 0);
}

static bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t &valOut) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  uint8_t n = Wire.requestFrom((int)addr, 2, (int)true);
  if (n != 2) return false;
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  valOut = ((uint16_t)msb << 8) | lsb;
  return true;
}

static bool i2cPing(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission(true) == 0);
}

static void i2cScan() {
  Serial.println("\nI2C scan:");
  int found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    if (i2cPing(a)) {
      Serial.printf("  - Found device at 0x%02X\n", a);
      found++;
    }
  }
  if (!found) Serial.println("  (none found)");
}

// ---------------------- PI4IOE5V6416 minimal driver ----------------------
class PI4IOE5V6416 {
public:
  explicit PI4IOE5V6416(uint8_t addr) : _addr(addr) {}

  bool begin() {
    return i2cPing(_addr);
  }

  bool setConfig(uint8_t cfg0, uint8_t cfg1) {
    // 06h/07h: 1=input, 0=output
    return i2cWrite8(_addr, 0x06, cfg0) && i2cWrite8(_addr, 0x07, cfg1);
  }

  bool writeOutputs(uint8_t out0, uint8_t out1) {
    // 02h/03h output port regs
    return i2cWrite8(_addr, 0x02, out0) && i2cWrite8(_addr, 0x03, out1);
  }

  bool readInputs(uint8_t &in0, uint8_t &in1) {
    // 00h/01h input port regs
    return i2cRead8(_addr, 0x00, in0) && i2cRead8(_addr, 0x01, in1);
  }

  uint8_t addr() const { return _addr; }

private:
  uint8_t _addr;
};

// ---------------------- ADS1015 minimal single-shot read ----------------------
static uint16_t adsBuildConfigSingleEnded(uint8_t channel, uint8_t pgaBits, uint8_t drBits) {
  // Config fields (ADS1015):
  // OS=1 (start single conversion), MUX=100..111 (AIN0..AIN3 vs GND),
  // PGA as selected, MODE=1 (single-shot), DR as selected, COMP disabled (11b)
  uint16_t mux = 0x4 + (channel & 0x03); // 100b..111b
  uint16_t cfg =
      (1u << 15) |                 // OS
      ((mux & 0x7) << 12) |         // MUX
      ((pgaBits & 0x7) << 9) |      // PGA
      (1u << 8) |                  // MODE = single-shot
      ((drBits & 0x7) << 5) |       // DR
      (0u << 4) |                  // COMP_MODE (don't care if disabled)
      (0u << 3) |                  // COMP_POL  (don't care if disabled)
      (0u << 2) |                  // COMP_LAT  (don't care if disabled)
      (0x3u);                      // COMP_QUE = 11b disable comparator
  return cfg;
}

static bool adsReadSingleEndedVolts(uint8_t channel, float &voltsOut) {
  // Defaults:
  // PGA=001b => ±4.096 V (2 mV/LSB for ADS1015 12-bit) (see datasheet table)
  // DR=100b => 1600 SPS (default)
  const uint8_t pgaBits = 0x01; // ±4.096 V
  const uint8_t drBits  = 0x04; // 1600 SPS

  const uint16_t cfg = adsBuildConfigSingleEnded(channel, pgaBits, drBits);

  // Pointer 0x01 = Config register, 0x00 = Conversion register
  if (!i2cWrite16(I2C_ADDR_ADS, 0x01, cfg)) return false;

  // Poll OS bit until it reads back 1 (conversion complete)
  uint32_t t0 = millis();
  while (millis() - t0 < 50) {
    uint16_t cfgRead = 0;
    if (i2cRead16(I2C_ADDR_ADS, 0x01, cfgRead)) {
      if (cfgRead & (1u << 15)) break;
    }
    delay(1);
  }

  uint16_t raw16 = 0;
  if (!i2cRead16(I2C_ADDR_ADS, 0x00, raw16)) return false;

  // ADS1015 conversion is 12-bit left-justified in 16-bit reg: bits [15:4]
  int16_t rawSigned = (int16_t)raw16;
  rawSigned >>= 4;

  // For PGA ±4.096 V, LSB = 2 mV (0.002 V)
  voltsOut = (float)rawSigned * 0.002f;
  return true;
}

// ---------------------- Analog measurement model ----------------------
struct AnalogPortReading {
  float adcVolts;
  float portVolts;
  float portCurrentmA;
};

// Arduino auto-generates prototypes for .ino functions. Declare these
// explicitly after AnalogPortReading so the generated prototype does not
// reference the type before it exists.
static bool readAnalogPort(uint8_t port, AnalogPortReading& out);
static void printAnalogLine(Stream& out, const char* labelPrefix);

static bool readAnalogPort(uint8_t port, AnalogPortReading& out) {
  if (port >= ANALOG_CHANNEL_COUNT) {
    return false;
  }

  float adcVolts = 0.0f;
  if (!adsReadSingleEndedVolts(port, adcVolts)) {
    return false;
  }

  float portVolts = adcVolts * PORT_VOLTAGE_SCALE[port];
  float portCurrentmA = 0.0f;

  if (PORT_IS_CURRENT_LOOP[port]) {
    const float shuntOhms = CURRENT_LOOP_SHUNT_OHMS[port];
    if (shuntOhms <= 0.0f) {
      return false;
    }
    portCurrentmA = (portVolts / shuntOhms) * 1000.0f;
  } else {
    // Informational branch current through divider path.
    portCurrentmA = (portVolts / PORT23_DIVIDER_OHMS) * 1000.0f;
  }

  if ((portVolts > -PORT_ZERO_CLAMP_V) && (portVolts < PORT_ZERO_CLAMP_V)) {
    portVolts = 0.0f;
    portCurrentmA = 0.0f;
  }

  out.adcVolts = adcVolts;
  out.portVolts = portVolts;
  out.portCurrentmA = portCurrentmA;
  return true;
}

static void printAnalogLine(Stream& out, const char* labelPrefix) {
  const uint32_t now = millis();
  out.printf("ANALOG t=%lu", (unsigned long)now);

  for (uint8_t port = 0; port < ANALOG_CHANNEL_COUNT; port++) {
    AnalogPortReading reading = {};
    if (readAnalogPort(port, reading)) {
      out.printf(" %s%u=%.3fV/%.3fmA",
                 labelPrefix,
                 (unsigned)port,
                 (double)reading.portVolts,
                 (double)reading.portCurrentmA);
    } else {
      out.printf(" %s%u=ERR", labelPrefix, (unsigned)port);
    }
  }

  out.print("\r\n");
}

// ---------------------- Global device handles ----------------------
static PI4IOE5V6416 *g_q3 = nullptr;
static PI4IOE5V6416 *g_q4 = nullptr;

static uint8_t g_q3_out0 = 0xFF;  // Q3 P0 outputs (GEN_0..7)
static uint8_t g_q4_out1 = 0x0F;  // Q4 P1 image: bits [7:4] outputs, bits [3:0] kept high
static bool g_allowRelayOutputs = ENABLE_RELAY_OUTPUTS_BY_DEFAULT;

// ---------------------- Fixture protocol engine ----------------------
// Q4 P1 mapping from schematic:
//   P1_4 = ISO_PORTD_1 = Relay K1
//   P1_5 = ISO_PORTD_2 = Relay K2
//   P1_6 = ISO_PORTD_3 = Digital Output 1
//   P1_7 = ISO_PORTD_4 = Digital Output 2
static constexpr uint8_t Q4_BIT_K1 = 4;
static constexpr uint8_t Q4_BIT_K2 = 5;
static constexpr uint8_t Q4_BIT_D1 = 6;
static constexpr uint8_t Q4_BIT_D2 = 7;
// Relay driver stage on ISO_PORTD_1/2 is active-high: HIGH energizes the coil.
static constexpr bool Q4_ACTIVE_LOW = false;
static constexpr size_t PROTO_RX_BUF_SIZE = 128;
static constexpr uint32_t LED_ROTATE_PERIOD_MS = 250;
static constexpr uint32_t DI_TEST_WAIT_HIGH_MS = 1500;
static constexpr uint32_t DI_TEST_WAIT_LOW_MS = 1500;

static char g_protoRxBuf[PROTO_RX_BUF_SIZE] = {0};
static size_t g_protoRxLen = 0;
static bool g_ledRotateActive = false;
static uint32_t g_ledRotateLastMs = 0;
static uint8_t g_ledRotateIndex = 0;

enum DiWatchPhase : uint8_t {
  DI_WATCH_IDLE = 0,
  DI_WATCH_WAIT_HIGH,
  DI_WATCH_WAIT_LOW
};

static bool g_diWatchActive = false;
static uint8_t g_diWatchChannel = 0;
static DiWatchPhase g_diWatchPhase = DI_WATCH_IDLE;
static uint32_t g_diWatchPhaseStartMs = 0;

static bool g_protoAnalogStreamActive = false;
static uint32_t g_protoAnalogLastSampleMs = 0;
static bool g_usbAnalogStreamActive = false;
static uint32_t g_usbAnalogLastSampleMs = 0;

static void sendTokenExact(const char* token) {
  if (PROTO_ENABLE && token) {
    Serial1.print(token);
  }
}

static void protoDropPrefix(size_t count) {
  if (count >= g_protoRxLen) {
    g_protoRxLen = 0;
    g_protoRxBuf[0] = '\0';
    return;
  }

  memmove(g_protoRxBuf, g_protoRxBuf + count, g_protoRxLen - count);
  g_protoRxLen -= count;
  g_protoRxBuf[g_protoRxLen] = '\0';
}

static void protoAppendByte(uint8_t byteIn) {
  char c = '\0';
  if (byteIn >= 32 && byteIn <= 126) {
    c = static_cast<char>(byteIn);
  } else if (byteIn == '\r' || byteIn == '\n' || byteIn == '\t') {
    c = ' ';
  }

  if (c == '\0') {
    return;
  }

  if (g_protoRxLen >= (PROTO_RX_BUF_SIZE - 1)) {
    // Keep the most recent half of the buffer when noise fills it.
    protoDropPrefix(g_protoRxLen - ((PROTO_RX_BUF_SIZE - 1) / 2));
  }

  g_protoRxBuf[g_protoRxLen++] = c;
  g_protoRxBuf[g_protoRxLen] = '\0';
}

static bool setQ4OutputLogical(uint8_t bit, bool isOn) {
  if (bit < 4 || bit > 7) {
    return false;
  }

  const uint8_t mask = (uint8_t)(1u << bit);
  if (Q4_ACTIVE_LOW) {
    if (isOn) {
      g_q4_out1 &= (uint8_t)~mask;
    } else {
      g_q4_out1 |= mask;
    }
  } else {
    if (isOn) {
      g_q4_out1 |= mask;
    } else {
      g_q4_out1 &= (uint8_t)~mask;
    }
  }

  // P1[3:0] are inputs and must stay high in output image.
  g_q4_out1 |= 0x0F;
  return true;
}

static void setAllQ4OutputsOffImage() {
  g_q4_out1 = 0x0F;
  setQ4OutputLogical(Q4_BIT_K1, false);
  setQ4OutputLogical(Q4_BIT_K2, false);
  setQ4OutputLogical(Q4_BIT_D1, false);
  setQ4OutputLogical(Q4_BIT_D2, false);
}

static bool applyAllQ4OutputsOff() {
  if (!g_q4) {
    return false;
  }

  setAllQ4OutputsOffImage();
  return g_q4->writeOutputs(0xFF, g_q4_out1);
}

static bool applyQ4Outputs() {
  if (!g_q4) {
    Serial.println("PROTO: Q4 expander missing");
    return false;
  }

  if (!g_q4->writeOutputs(0xFF, g_q4_out1)) {
    Serial.println("PROTO: Q4 write failed");
    return false;
  }

  return true;
}

static bool applyQ3Outputs() {
  if (!g_q3) {
    Serial.println("PROTO: Q3 expander missing");
    return false;
  }

  if (!g_q3->writeOutputs(g_q3_out0, 0xFF)) {
    Serial.println("PROTO: Q3 write failed");
    return false;
  }

  return true;
}

static bool handleSetOutput(uint8_t bit, bool isOn, const char* ackToken) {
  if (!setQ4OutputLogical(bit, isOn)) {
    Serial.println("PROTO: Invalid Q4 bit");
    return false;
  }

  if (!applyQ4Outputs()) {
    return false;
  }

  sendTokenExact(ackToken);
  return true;
}

static bool handlePing() { sendTokenExact(TOKEN_PONG); return true; }
static bool handleReady() { sendTokenExact(TOKEN_OK); return true; }
static bool handleFaultDone() { sendTokenExact(TOKEN_MOVINGON); return true; }
static bool handleAnalogStart() {
  g_protoAnalogStreamActive = true;
  g_protoAnalogLastSampleMs = millis() - ANALOG_STREAM_PERIOD_MS;
  sendTokenExact(TOKEN_OK_ANALOG);
  return true;
}
static bool handleAnalogStop() {
  g_protoAnalogStreamActive = false;
  sendTokenExact(TOKEN_OK_STOP);
  return true;
}
static bool handleDo1Off() { return handleSetOutput(Q4_BIT_D1, false, TOKEN_OK_DO1_OFF); }
static bool handleDo1On() { return handleSetOutput(Q4_BIT_D1, true, TOKEN_OK_DO1_ON); }
static bool handleDo2Off() { return handleSetOutput(Q4_BIT_D2, false, TOKEN_OK_DO2_OFF); }
static bool handleDo2On() { return handleSetOutput(Q4_BIT_D2, true, TOKEN_OK_DO2_ON); }
static bool handleK1Off() { return handleSetOutput(Q4_BIT_K1, false, TOKEN_OK_K1_OFF); }
static bool handleK1On() { return handleSetOutput(Q4_BIT_K1, true, TOKEN_OK_K1_ON); }
static bool handleK2Off() { return handleSetOutput(Q4_BIT_K2, false, TOKEN_OK_K2_OFF); }
static bool handleK2On() { return handleSetOutput(Q4_BIT_K2, true, TOKEN_OK_K2_ON); }
static bool handleDoReadOk() { return true; }
static bool handleRelaysReadOk() { return true; }
static bool handleDiBegin() { sendTokenExact(TOKEN_OK_DI_BEGIN); return true; }
static bool handleAiBegin() { sendTokenExact(TOKEN_OK_AI_BEGIN); return true; }
static bool handleDiReadOk() { return true; }
static bool handleAiReadOk() { return true; }
static bool handleBtnBegin() { sendTokenExact(TOKEN_OK_BTN_BEGIN); return true; }
static bool handleBtnReadOk() { return true; }
static bool handleLedVisualOk() { return true; }

static bool readQ3ButtonState(uint8_t p1Bit, bool& highOut) {
  if (!g_q3) {
    Serial.println("PROTO: Q3 button read failed, expander missing");
    return false;
  }

  if (p1Bit > 7) {
    return false;
  }

  uint8_t q3_in0 = 0;
  uint8_t q3_in1 = 0;
  if (!g_q3->readInputs(q3_in0, q3_in1)) {
    Serial.println("PROTO: Q3 button read failed, I2C error");
    return false;
  }

  highOut = ((q3_in1 >> p1Bit) & 0x01u) != 0u;
  return true;
}

static bool handleButtonStateCommand(const char* buttonName, uint8_t p1Bit, bool expectHigh,
                                     const char* ackToken, const char* negativeAckToken) {
  bool measuredHigh = false;
  if (!readQ3ButtonState(p1Bit, measuredHigh)) {
    return false;
  }

  if (measuredHigh != expectHigh) {
    Serial.printf("PROTO: %s mismatch exp=%s got=%s\n",
                  buttonName,
                  expectHigh ? "HIGH" : "LOW",
                  measuredHigh ? "HIGH" : "LOW");
    sendTokenExact(negativeAckToken);
    return true;
  }

  sendTokenExact(ackToken);
  return true;
}

static bool handleBtnUsr0Press() { return handleButtonStateCommand("USR0", 5, false, TOKEN_OK_BTN_USR0_PRESS, TOKEN_NG_BTN_USR0_PRESS); }
static bool handleBtnUsr0Release() { return handleButtonStateCommand("USR0", 5, true, TOKEN_OK_BTN_USR0_RELEASE, TOKEN_NG_BTN_USR0_RELEASE); }
static bool handleBtnPnl0Press() { return handleButtonStateCommand("PNL0", 0, false, TOKEN_OK_BTN_PNL0_PRESS, TOKEN_NG_BTN_PNL0_PRESS); }
static bool handleBtnPnl0Release() { return handleButtonStateCommand("PNL0", 0, true, TOKEN_OK_BTN_PNL0_RELEASE, TOKEN_NG_BTN_PNL0_RELEASE); }
static bool handleBtnPnl1Press() { return handleButtonStateCommand("PNL1", 1, false, TOKEN_OK_BTN_PNL1_PRESS, TOKEN_NG_BTN_PNL1_PRESS); }
static bool handleBtnPnl1Release() { return handleButtonStateCommand("PNL1", 1, true, TOKEN_OK_BTN_PNL1_RELEASE, TOKEN_NG_BTN_PNL1_RELEASE); }
static bool handleBtnPnl2Press() { return handleButtonStateCommand("PNL2", 2, false, TOKEN_OK_BTN_PNL2_PRESS, TOKEN_NG_BTN_PNL2_PRESS); }
static bool handleBtnPnl2Release() { return handleButtonStateCommand("PNL2", 2, true, TOKEN_OK_BTN_PNL2_RELEASE, TOKEN_NG_BTN_PNL2_RELEASE); }
static bool handleBtnPnl3Press() { return handleButtonStateCommand("PNL3", 3, false, TOKEN_OK_BTN_PNL3_PRESS, TOKEN_NG_BTN_PNL3_PRESS); }
static bool handleBtnPnl3Release() { return handleButtonStateCommand("PNL3", 3, true, TOKEN_OK_BTN_PNL3_RELEASE, TOKEN_NG_BTN_PNL3_RELEASE); }

static void updateLedRotation() {
  if (!g_ledRotateActive) {
    return;
  }

  const uint32_t now = millis();
  if ((now - g_ledRotateLastMs) < LED_ROTATE_PERIOD_MS) {
    return;
  }

  g_ledRotateLastMs = now;
  g_ledRotateIndex = (uint8_t)((g_ledRotateIndex + 1u) & 0x07u);
  g_q3_out0 = (uint8_t)(0xFFu & (uint8_t)~(1u << g_ledRotateIndex));
  if (!applyQ3Outputs()) {
    g_ledRotateActive = false;
  }
}

static bool handleLedBegin() {
  sendTokenExact(TOKEN_OK_LED_BEGIN);
  return true;
}

static bool handleLedAllOn() {
  g_ledRotateActive = false;
  g_q3_out0 = 0xFF;
  if (!applyQ3Outputs()) {
    return false;
  }
  sendTokenExact(TOKEN_OK_LED_ALL_ON);
  return true;
}

static bool handleLedRotate250() {
  g_ledRotateActive = true;
  g_ledRotateIndex = 0;
  g_ledRotateLastMs = millis();
  g_q3_out0 = (uint8_t)(0xFFu & (uint8_t)~(1u << g_ledRotateIndex));
  if (!applyQ3Outputs()) {
    g_ledRotateActive = false;
    return false;
  }
  sendTokenExact(TOKEN_OK_LED_ROTATE_250);
  return true;
}

static bool handleLedStopAllOn() {
  g_ledRotateActive = false;
  g_q3_out0 = 0xFF;
  if (!applyQ3Outputs()) {
    return false;
  }
  sendTokenExact(TOKEN_OK_LED_STOP_ALL_ON);
  return true;
}

static bool readDiState(uint8_t oneBasedIndex, bool& highOut) {
  if (!g_q4) {
    Serial.println("PROTO: DI read failed, Q4 missing");
    return false;
  }

  uint8_t q4_in0 = 0;
  uint8_t q4_in1 = 0;
  if (!g_q4->readInputs(q4_in0, q4_in1)) {
    Serial.println("PROTO: DI read failed, I2C error");
    return false;
  }

  if (oneBasedIndex >= 1 && oneBasedIndex <= 8) {
    highOut = ((q4_in0 >> (oneBasedIndex - 1)) & 0x01u) != 0u;
    return true;
  }

  if (oneBasedIndex >= 9 && oneBasedIndex <= 12) {
    highOut = ((q4_in1 >> (oneBasedIndex - 9)) & 0x01u) != 0u;
    return true;
  }

  return false;
}

static void sendDiTestReply(uint8_t oneBasedIndex, bool pass) {
  char reply[16];
  snprintf(reply, sizeof(reply), "DI%u%s", (unsigned)oneBasedIndex, pass ? "OK" : "FAIL");
  sendTokenExact(reply);
}

static void finishDiTransitionWatch(bool pass, const char* reason) {
  if (!g_diWatchActive) {
    return;
  }

  if (!pass && reason) {
    Serial.printf("PROTO: DI%uTEST failed (%s)\n", (unsigned)g_diWatchChannel, reason);
  }

  sendDiTestReply(g_diWatchChannel, pass);

  g_diWatchActive = false;
  g_diWatchChannel = 0;
  g_diWatchPhase = DI_WATCH_IDLE;
  g_diWatchPhaseStartMs = 0;
}

static bool startDiTransitionWatch(uint8_t oneBasedIndex) {
  if (oneBasedIndex < 1 || oneBasedIndex > 12) {
    return false;
  }

  if (g_diWatchActive) {
    return false;
  }

  g_diWatchActive = true;
  g_diWatchChannel = oneBasedIndex;
  g_diWatchPhase = DI_WATCH_WAIT_HIGH;
  g_diWatchPhaseStartMs = millis();
  return true;
}

static void updateDiTransitionWatch() {
  if (!g_diWatchActive) {
    return;
  }

  bool highNow = false;
  if (!readDiState(g_diWatchChannel, highNow)) {
    finishDiTransitionWatch(false, "read failed");
    return;
  }

  const uint32_t now = millis();

  if (g_diWatchPhase == DI_WATCH_WAIT_HIGH) {
    if (highNow) {
      g_diWatchPhase = DI_WATCH_WAIT_LOW;
      g_diWatchPhaseStartMs = now;
      return;
    }

    if ((now - g_diWatchPhaseStartMs) >= DI_TEST_WAIT_HIGH_MS) {
      finishDiTransitionWatch(false, "high timeout");
    }
    return;
  }

  if (g_diWatchPhase == DI_WATCH_WAIT_LOW) {
    if (!highNow) {
      finishDiTransitionWatch(true, nullptr);
      return;
    }

    if ((now - g_diWatchPhaseStartMs) >= DI_TEST_WAIT_LOW_MS) {
      finishDiTransitionWatch(false, "low timeout");
    }
  }
}

static void updateProtoAnalogStreaming() {
  if (!g_protoAnalogStreamActive) {
    return;
  }

  const uint32_t now = millis();
  if ((now - g_protoAnalogLastSampleMs) < ANALOG_STREAM_PERIOD_MS) {
    return;
  }

  g_protoAnalogLastSampleMs = now;
  printAnalogLine(Serial1, "A");
}

static bool handleDiTransitionTestCommand(uint8_t oneBasedIndex) {
  if (!startDiTransitionWatch(oneBasedIndex)) {
    /* Reject overlapping/invalid requests with a deterministic fail reply. */
    sendDiTestReply(oneBasedIndex, false);
  }
  return true;
}

static bool handleDiStateCommand(uint8_t oneBasedIndex, bool expectHigh) {
  bool measuredHigh = false;
  if (!readDiState(oneBasedIndex, measuredHigh)) {
    return false;
  }

  if (measuredHigh != expectHigh) {
    Serial.printf("PROTO: DI%u mismatch exp=%s got=%s\n",
                  (unsigned)oneBasedIndex,
                  expectHigh ? "HIGH" : "LOW",
                  measuredHigh ? "HIGH" : "LOW");
    return false;
  }

  char ackToken[24];
  snprintf(ackToken, sizeof(ackToken), "OK_DI%u_%s", (unsigned)oneBasedIndex, expectHigh ? "HIGH" : "LOW");
  sendTokenExact(ackToken);
  return true;
}

static constexpr uint8_t AI_PHASE_LOW  = 0;
static constexpr uint8_t AI_PHASE_MID  = 1;
static constexpr uint8_t AI_PHASE_HIGH = 2;

static const char* aiPhaseName(uint8_t phase) {
  switch (phase) {
    case AI_PHASE_LOW:  return "LOW";
    case AI_PHASE_MID:  return "MID";
    case AI_PHASE_HIGH: return "HIGH";
    default:            return "LOW";
  }
}

static bool handleAiSampleCommand(uint8_t oneBasedIndex, uint8_t phase) {
  if (oneBasedIndex < 1 || oneBasedIndex > 4) {
    return false;
  }

  // Determinism guard: stop Serial1 analog streaming while answering point requests.
  g_protoAnalogStreamActive = false;

  AnalogPortReading reading = {};
  if (!readAnalogPort((uint8_t)(oneBasedIndex - 1), reading)) {
    Serial.printf("PROTO: AI%u sample failed\n", (unsigned)oneBasedIndex);
    return false;
  }

  char reply[40];
  snprintf(reply, sizeof(reply), "OK_AI%u_%s:%.3f", (unsigned)oneBasedIndex, aiPhaseName(phase), (double)reading.adcVolts);
  sendTokenExact(reply);
  return true;
}

using ProtoHandler = bool (*)();
struct ProtoCommand {
  const char* rxToken;
  ProtoHandler handler;
};

static const ProtoCommand kProtoCommands[] = {
    {TOKEN_LED_STOP_ALL_ON, handleLedStopAllOn},
    {TOKEN_BTN_USR0_RELEASE, handleBtnUsr0Release},
    {TOKEN_BTN_PNL0_RELEASE, handleBtnPnl0Release},
    {TOKEN_BTN_PNL1_RELEASE, handleBtnPnl1Release},
    {TOKEN_BTN_PNL2_RELEASE, handleBtnPnl2Release},
    {TOKEN_BTN_PNL3_RELEASE, handleBtnPnl3Release},
    {TOKEN_BTN_USR0_PRESS, handleBtnUsr0Press},
    {TOKEN_BTN_PNL0_PRESS, handleBtnPnl0Press},
    {TOKEN_BTN_PNL1_PRESS, handleBtnPnl1Press},
    {TOKEN_BTN_PNL2_PRESS, handleBtnPnl2Press},
    {TOKEN_BTN_PNL3_PRESS, handleBtnPnl3Press},
    {TOKEN_LED_ROTATE_250, handleLedRotate250},
    {TOKEN_LED_VISUAL_OK, handleLedVisualOk},
    {TOKEN_LED_ALL_ON, handleLedAllOn},
    {TOKEN_LED_BEGIN, handleLedBegin},
    {TOKEN_BTN_READ_OK, handleBtnReadOk},
    {TOKEN_BTN_BEGIN, handleBtnBegin},
    {TOKEN_AI_READ_OK, handleAiReadOk},
    {TOKEN_DI_READ_OK, handleDiReadOk},
    {TOKEN_AI_BEGIN, handleAiBegin},
    {TOKEN_DI_BEGIN, handleDiBegin},
    {TOKEN_RELAYS_READ_OK, handleRelaysReadOk},
    {TOKEN_DO_READ_OK, handleDoReadOk},
    {TOKEN_DO1_OFF, handleDo1Off},
    {TOKEN_DO1_ON, handleDo1On},
    {TOKEN_DO2_OFF, handleDo2Off},
    {TOKEN_DO2_ON, handleDo2On},
    {TOKEN_K1_OFF, handleK1Off},
    {TOKEN_K1_ON, handleK1On},
    {TOKEN_K2_OFF, handleK2Off},
    {TOKEN_K2_ON, handleK2On},
    {TOKEN_ANALOG, handleAnalogStart},
    {TOKEN_STOP, handleAnalogStop},
    {TOKEN_FAULTD, handleFaultDone},
    {TOKEN_READY, handleReady},
    {TOKEN_PING, handlePing},
};

static bool processDynamicInputsCommands() {
  bool matched = false;
  size_t bestPos = 0;
  size_t bestLen = 0;
  bool bestIsDiTest = false;
  bool bestIsAnalog = false;
  bool bestHigh = false;
  uint8_t bestAiPhase = AI_PHASE_LOW;
  uint8_t bestIndex = 0;

  char token[20];
  for (uint8_t i = 1; i <= 12; i++) {
    snprintf(token, sizeof(token), "DI%uTEST", (unsigned)i);
    if (const char* found = strstr(g_protoRxBuf, token)) {
      const size_t pos = (size_t)(found - g_protoRxBuf);
      const size_t len = strlen(token);
      if (!matched || pos < bestPos || (pos == bestPos && len > bestLen)) {
        matched = true;
        bestPos = pos;
        bestLen = len;
        bestIsDiTest = true;
        bestIsAnalog = false;
        bestHigh = false;
        bestAiPhase = AI_PHASE_LOW;
        bestIndex = i;
      }
    }

    snprintf(token, sizeof(token), "DI%u_HIGH", (unsigned)i);
    if (const char* found = strstr(g_protoRxBuf, token)) {
      const size_t pos = (size_t)(found - g_protoRxBuf);
      const size_t len = strlen(token);
      if (!matched || pos < bestPos || (pos == bestPos && len > bestLen)) {
        matched = true;
        bestPos = pos;
        bestLen = len;
        bestIsDiTest = false;
        bestIsAnalog = false;
        bestHigh = true;
        bestAiPhase = AI_PHASE_LOW;
        bestIndex = i;
      }
    }

    snprintf(token, sizeof(token), "DI%u_LOW", (unsigned)i);
    if (const char* found = strstr(g_protoRxBuf, token)) {
      const size_t pos = (size_t)(found - g_protoRxBuf);
      const size_t len = strlen(token);
      if (!matched || pos < bestPos || (pos == bestPos && len > bestLen)) {
        matched = true;
        bestPos = pos;
        bestLen = len;
        bestIsDiTest = false;
        bestIsAnalog = false;
        bestHigh = false;
        bestAiPhase = AI_PHASE_LOW;
        bestIndex = i;
      }
    }
  }

  for (uint8_t i = 1; i <= 4; i++) {
    snprintf(token, sizeof(token), "AI%u_HIGH_REQ", (unsigned)i);
    if (const char* found = strstr(g_protoRxBuf, token)) {
      const size_t pos = (size_t)(found - g_protoRxBuf);
      const size_t len = strlen(token);
      if (!matched || pos < bestPos || (pos == bestPos && len > bestLen)) {
        matched = true;
        bestPos = pos;
        bestLen = len;
        bestIsDiTest = false;
        bestIsAnalog = true;
        bestHigh = true;
        bestAiPhase = AI_PHASE_HIGH;
        bestIndex = i;
      }
    }

    snprintf(token, sizeof(token), "AI%u_MID_REQ", (unsigned)i);
    if (const char* found = strstr(g_protoRxBuf, token)) {
      const size_t pos = (size_t)(found - g_protoRxBuf);
      const size_t len = strlen(token);
      if (!matched || pos < bestPos || (pos == bestPos && len > bestLen)) {
        matched = true;
        bestPos = pos;
        bestLen = len;
        bestIsDiTest = false;
        bestIsAnalog = true;
        bestHigh = false;
        bestAiPhase = AI_PHASE_MID;
        bestIndex = i;
      }
    }

    snprintf(token, sizeof(token), "AI%u_LOW_REQ", (unsigned)i);
    if (const char* found = strstr(g_protoRxBuf, token)) {
      const size_t pos = (size_t)(found - g_protoRxBuf);
      const size_t len = strlen(token);
      if (!matched || pos < bestPos || (pos == bestPos && len > bestLen)) {
        matched = true;
        bestPos = pos;
        bestLen = len;
        bestIsDiTest = false;
        bestIsAnalog = true;
        bestHigh = false;
        bestAiPhase = AI_PHASE_LOW;
        bestIndex = i;
      }
    }
  }

  if (!matched) {
    return false;
  }

  bool ok = false;
  if (bestIsDiTest) {
    ok = handleDiTransitionTestCommand(bestIndex);
  } else if (bestIsAnalog) {
    ok = handleAiSampleCommand(bestIndex, bestAiPhase);
  } else {
    ok = handleDiStateCommand(bestIndex, bestHigh);
  }

  if (!ok) {
    if (bestIsDiTest) {
      Serial.printf("PROTO: DI%uTEST failed\n", (unsigned)bestIndex);
    } else if (bestIsAnalog) {
      Serial.printf("PROTO: AI%u_%s_REQ failed\n", (unsigned)bestIndex, aiPhaseName(bestAiPhase));
    } else {
      Serial.printf("PROTO: DI%u_%s failed\n", (unsigned)bestIndex, bestHigh ? "HIGH" : "LOW");
    }
  }

  protoDropPrefix(bestPos + bestLen);
  return true;
}

static bool processProtocolBufferOnce() {
  if (processDynamicInputsCommands()) {
    return true;
  }

  const ProtoCommand* bestCmd = nullptr;
  size_t bestPos = 0;
  size_t bestLen = 0;

  for (size_t i = 0; i < (sizeof(kProtoCommands) / sizeof(kProtoCommands[0])); i++) {
    const char* found = strstr(g_protoRxBuf, kProtoCommands[i].rxToken);
    if (!found) {
      continue;
    }

    const size_t pos = static_cast<size_t>(found - g_protoRxBuf);
    const size_t len = strlen(kProtoCommands[i].rxToken);

    if (!bestCmd || pos < bestPos || (pos == bestPos && len > bestLen)) {
      bestCmd = &kProtoCommands[i];
      bestPos = pos;
      bestLen = len;
    }
  }

  if (bestCmd) {
    const bool handled = bestCmd->handler();
    if (!handled) {
      Serial.printf("PROTO: command failed: %s\n", bestCmd->rxToken);
    }

    // Consume through the matched token so one token maps to one action.
    protoDropPrefix(bestPos + bestLen);
    return true;
  }

  if (g_protoRxLen > 0 && g_protoRxBuf[0] == ' ') {
    protoDropPrefix(1);
  } else if (g_protoRxLen > (PROTO_RX_BUF_SIZE - 16)) {
    protoDropPrefix(g_protoRxLen - ((PROTO_RX_BUF_SIZE - 1) / 2));
  }

  return false;
}

static void processFixtureProtocol() {
  if (!PROTO_ENABLE) {
    return;
  }

  while (Serial1.available()) {
    protoAppendByte((uint8_t)Serial1.read());
  }

  while (processProtocolBufferOnce()) {
  }
}

// ---------------------- Reset expanders ----------------------
static void resetI2CBusDevices() {
  pinMode(PIN_IOBUS_RST_N, OUTPUT);
  digitalWrite(PIN_IOBUS_RST_N, HIGH);
  delay(2);
  digitalWrite(PIN_IOBUS_RST_N, LOW);
  delay(2);
  digitalWrite(PIN_IOBUS_RST_N, HIGH);
  delay(5);
}

// ---------------------- Serial command menu ----------------------
static void printHelp() {
  Serial.println("\nCommands:");
  Serial.println("  ANALOG            - start analog stream on USB");
  Serial.println("  STOP              - stop analog stream on USB");
  Serial.println("  ADC               - one-shot analog sample line");
  Serial.println("  SCAN              - I2C scan");
  Serial.println("  HELP              - show this menu");
  Serial.println("  help              - show this menu");
  Serial.println("  scan              - I2C scan");
  Serial.println("  status            - show detected addresses + last output bytes");
  Serial.println("  relays on|off      - enable/disable toggling relay/digital outputs on Q4");
  Serial.println("  q3 <hex>           - write Q3 P0 output byte (GEN_7..GEN_0)");
  Serial.println("  q4 <hex>           - write Q4 P1 output byte for bits7..4 (K2,K1,D2,D1) + keep bits3..0 high");
  Serial.println("  read              - read and print all PI4 inputs once");
  Serial.println("  adc               - read AIN0..AIN3 once");
  Serial.println("  rst               - pulse GPIO9 reset (active-low) and re-init");
}

static void showStatus() {
  Serial.printf("\nQ3 addr: %s\n", g_q3 ? "OK" : "NOT FOUND");
  if (g_q3) Serial.printf("  - 0x%02X, out0=0x%02X\n", g_q3->addr(), g_q3_out0);

  Serial.printf("Q4 addr: %s\n", g_q4 ? "OK" : "NOT FOUND");
  if (g_q4) Serial.printf("  - 0x%02X, out1=0x%02X, relaysEnabled=%s\n",
                          g_q4->addr(), g_q4_out1, g_allowRelayOutputs ? "YES" : "NO");
}

static void readAllInputsOnce() {
  if (!g_q3 || !g_q4) return;

  uint8_t q3_in0=0, q3_in1=0;
  uint8_t q4_in0=0, q4_in1=0;

  if (g_q3->readInputs(q3_in0, q3_in1)) {
    Serial.printf("Q3 inputs: P0=0x%02X (unused), P1=0x%02X\n", q3_in0, q3_in1);
  } else {
    Serial.println("Q3 readInputs failed");
  }

  if (g_q4->readInputs(q4_in0, q4_in1)) {
    Serial.printf("Q4 inputs: P0=0x%02X, P1=0x%02X (bits3..0 are LD26..23)\n", q4_in0, q4_in1);
  } else {
    Serial.println("Q4 readInputs failed");
  }
}

static void readAdcOnce() {
  Serial.println("ADS1015 AIN0..AIN3:");
  for (uint8_t ch = 0; ch < 4; ch++) {
    float v = 0.0f;
    if (adsReadSingleEndedVolts(ch, v)) {
      Serial.printf("  AIN%u = %.3f V\n", ch, (double)v);
    } else {
      Serial.printf("  AIN%u = (read failed)\n", ch);
    }
  }
}

// Parse a hex byte like "A5" or "0xA5"
static bool parseHexByte(const String &s, uint8_t &out) {
  char *endp = nullptr;
  long v = strtol(s.c_str(), &endp, 0);
  if (endp == s.c_str()) return false;
  if (v < 0 || v > 255) return false;
  out = (uint8_t)v;
  return true;
}

static void handleCommandLine(String line) {
  line.trim();
  if (!line.length()) return;

  // split by space
  int sp = line.indexOf(' ');
  String cmd = (sp < 0) ? line : line.substring(0, sp);
  String arg = (sp < 0) ? ""   : line.substring(sp + 1);
  cmd.toLowerCase();
  arg.trim();

  if (cmd == "analog") {
    g_usbAnalogStreamActive = true;
    g_usbAnalogLastSampleMs = millis() - ANALOG_STREAM_PERIOD_MS;
    Serial.println(TOKEN_OK_ANALOG);
    return;
  }

  if (cmd == "stop") {
    g_usbAnalogStreamActive = false;
    Serial.println(TOKEN_OK_STOP);
    return;
  }

  if (cmd == "help") { printHelp(); return; }
  if (cmd == "scan") { i2cScan(); return; }
  if (cmd == "status") { showStatus(); return; }
  if (cmd == "read") { readAllInputsOnce(); return; }
  if (cmd == "adc") { printAnalogLine(Serial, "P"); return; }

  if (cmd == "relays") {
    arg.toLowerCase();
    if (arg == "on") { g_allowRelayOutputs = true; Serial.println("Relay/digital outputs ENABLED"); }
    else if (arg == "off") { g_allowRelayOutputs = false; Serial.println("Relay/digital outputs DISABLED"); }
    else Serial.println("Usage: relays on|off");
    return;
  }

  if (cmd == "q3") {
    if (!g_q3) { Serial.println("Q3 not found"); return; }
    uint8_t b = 0;
    if (!parseHexByte(arg, b)) { Serial.println("Usage: q3 <hexByte>"); return; }
    g_q3_out0 = b;
    if (!g_q3->writeOutputs(g_q3_out0, 0xFF)) Serial.println("Q3 writeOutputs failed");
    return;
  }

  if (cmd == "q4") {
    if (!g_q4) { Serial.println("Q4 not found"); return; }
    if (!g_allowRelayOutputs) { Serial.println("Relays are disabled. Run: relays on"); return; }
    uint8_t b = 0;
    if (!parseHexByte(arg, b)) { Serial.println("Usage: q4 <hexByte>"); return; }

    // Only allow writing bits7..4. Keep bits3..0 high (inputs).
    g_q4_out1 = (b & 0xF0) | 0x0F;
    if (!g_q4->writeOutputs(0xFF, g_q4_out1)) Serial.println("Q4 writeOutputs failed");
    return;
  }

  if (cmd == "rst") {
    Serial.println("Pulsing reset and reinitializing...");
    resetI2CBusDevices();
    // fall through to re-init in setup-like flow:
    // easiest is reboot
    ESP.restart();
    return;
  }

  Serial.println("Unknown command. Type: help");
}

static void printUsbProtoHelp() {
  Serial.println("\nCommands:");
  Serial.println("  ANALOG  - start continuous analog stream");
  Serial.println("  STOP    - stop analog stream");
  Serial.println("  ADC     - one-shot analog sample");
  Serial.println("  SCAN    - I2C scan");
  Serial.println("  HELP    - show this menu");
}

static void handleUsbProtoLine(String line) {
  line.trim();
  if (!line.length()) {
    return;
  }

  String cmd = line;
  cmd.toUpperCase();

  if (cmd == TOKEN_ANALOG) {
    g_usbAnalogStreamActive = true;
    g_usbAnalogLastSampleMs = millis() - ANALOG_STREAM_PERIOD_MS;
    Serial.println(TOKEN_OK_ANALOG);
    return;
  }

  if (cmd == TOKEN_STOP) {
    g_usbAnalogStreamActive = false;
    Serial.println(TOKEN_OK_STOP);
    return;
  }

  if (cmd == "ADC") {
    printAnalogLine(Serial, "P");
    return;
  }

  if (cmd == "SCAN") {
    i2cScan();
    return;
  }

  if (cmd == "HELP") {
    printUsbProtoHelp();
    return;
  }

  Serial.println("Unknown command. Use ANALOG, STOP, ADC, SCAN, HELP.");
}

static void updateUsbAnalogStreaming() {
  if (!g_usbAnalogStreamActive) {
    return;
  }

  const uint32_t now = millis();
  if ((now - g_usbAnalogLastSampleMs) < ANALOG_STREAM_PERIOD_MS) {
    return;
  }

  g_usbAnalogLastSampleMs = now;
  printAnalogLine(Serial, "P");
}

// ---------------------- Setup / loop ----------------------
static String g_lineBuf;
static String g_usbProtoLineBuf;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\nESP32-S3 I2C Expander + ADS1015 Bring-up");

  pinMode(PIN_Q3_INT_N, INPUT_PULLUP);
  pinMode(PIN_Q4_INT_N, INPUT_PULLUP);
  pinMode(PIN_ADC_ALRT, INPUT_PULLUP);

  resetI2CBusDevices();

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);

  i2cScan();

  // Detect PI4 expanders at 0x20/0x21
  bool have0 = i2cPing(PI4_ADDR0);
  bool have1 = i2cPing(PI4_ADDR1);

  if (have0) Serial.printf("PI4 found at 0x%02X\n", PI4_ADDR0);
  if (have1) Serial.printf("PI4 found at 0x%02X\n", PI4_ADDR1);

  // Assign: prefer Q3=0x20, Q4=0x21 if both exist. (If your hardware swaps them, just swap here.)
  if (have0) g_q3 = new PI4IOE5V6416(PI4_ADDR0);
  if (have1) g_q4 = new PI4IOE5V6416(PI4_ADDR1);

  if (!g_q3 || !g_q4) {
    Serial.println("WARNING: Did not find both PI4 expanders at 0x20/0x21. Check ADDR strapping / solder / pullups.");
  }

  // Configure Q3: P0 outputs (GEN_0..7), P1 inputs
  if (g_q3 && g_q3->begin()) {
    g_q3->setConfig(0x00, 0xFF);
    g_q3_out0 = 0xFF;
    g_q3->writeOutputs(g_q3_out0, 0xFF);
    Serial.println("Q3 configured: P0=outputs, P1=inputs");
  }

  // Configure Q4: P0 inputs, P1[3:0] inputs, P1[7:4] outputs (K1,K2,D1,D2)
  if (g_q4 && g_q4->begin()) {
    // Relay schematics show the driver nets should idle deasserted/off at boot.
    // Preload the latch to the OFF image before switching P1[7:4] into outputs.
    applyAllQ4OutputsOff();
    g_q4->setConfig(0xFF, 0x0F);
    applyAllQ4OutputsOff();
    Serial.println("Q4 configured: P0=inputs, P1[7:4]=outputs, P1[3:0]=inputs, all outputs OFF");
  }

  // ADS1015 presence check
  if (i2cPing(I2C_ADDR_ADS)) Serial.printf("ADS1015 found at 0x%02X\n", I2C_ADDR_ADS);
  else Serial.printf("WARNING: ADS1015 not found at 0x%02X\n", I2C_ADDR_ADS);

  if (PROTO_ENABLE) {
    Serial1.begin(PROTO_BAUD, SERIAL_8N1, PROTO_RX_PIN, PROTO_TX_PIN);
    Serial.printf("Fixture protocol enabled on Serial1 (RX=%d TX=%d @ %lu)\n",
                  PROTO_RX_PIN, PROTO_TX_PIN, (unsigned long)PROTO_BAUD);
  } else if (ENABLE_CELL_UART_BRIDGE) {
    Serial1.begin(115200, SERIAL_8N1, PIN_CELL_RX, PIN_CELL_TX);
    Serial.println("Serial1 bridge enabled (CELL_RX=GPIO11, CELL_TX=GPIO10)");
  }

  if (!PROTO_ONLY_MODE) {
    printHelp();
  } else {
    printUsbProtoHelp();
  }
}

void loop() {
  if (PROTO_ENABLE) {
    processFixtureProtocol();
    updateDiTransitionWatch();
    updateLedRotation();
    updateProtoAnalogStreaming();
  }

  if (PROTO_ONLY_MODE) {
    while (Serial.available()) {
      char c = (char)Serial.read();
      if (c == '\r') continue;
      if (c == '\n') {
        handleUsbProtoLine(g_usbProtoLineBuf);
        g_usbProtoLineBuf = "";
      } else {
        g_usbProtoLineBuf += c;
        if (g_usbProtoLineBuf.length() > 120) {
          g_usbProtoLineBuf.remove(0, g_usbProtoLineBuf.length() - 120);
        }
      }
    }

    updateUsbAnalogStreaming();
    delay(1);
    return;
  }

  // Serial command input
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      handleCommandLine(g_lineBuf);
      g_lineBuf = "";
    } else {
      g_lineBuf += c;
    }
  }

  // Optional UART bridge
  if (ENABLE_CELL_UART_BRIDGE && !PROTO_ENABLE) {
    while (Serial1.available()) Serial.write((char)Serial1.read());
    while (Serial.available()) Serial1.write((char)Serial.read());
  }

  updateUsbAnalogStreaming();

  static uint32_t tLed = 0, tIn = 0, tAdc = 0;
  static uint8_t ledIdx = 0;

  // LED walk on Q3 GEN_0..7 every 200 ms
  if (g_q3 && (millis() - tLed) > 200) {
    tLed = millis();

    // walking 0 bit (active-low LED wiring would invert; adjust if needed)
    g_q3_out0 = (uint8_t)~(1u << (ledIdx & 7));
    g_q3->writeOutputs(g_q3_out0, 0xFF);
    ledIdx++;
  }

  // Input monitor every 50 ms
  if (g_q3 && g_q4 && (millis() - tIn) > 50) {
    tIn = millis();

    static uint8_t last_q3_p1 = 0xFF;
    static uint8_t last_q4_p0 = 0xFF;
    static uint8_t last_q4_p1 = 0xFF;

    uint8_t q3_in0=0, q3_in1=0, q4_in0=0, q4_in1=0;

    if (g_q3->readInputs(q3_in0, q3_in1)) {
      if (q3_in1 != last_q3_p1) {
        Serial.printf("Q3 P1 changed: 0x%02X -> 0x%02X\n", last_q3_p1, q3_in1);
        last_q3_p1 = q3_in1;
      }
    }

    if (g_q4->readInputs(q4_in0, q4_in1)) {
      if (q4_in0 != last_q4_p0) {
        Serial.printf("Q4 P0 changed: 0x%02X -> 0x%02X\n", last_q4_p0, q4_in0);
        last_q4_p0 = q4_in0;
      }
      uint8_t q4_inputs_p1 = q4_in1 & 0x0F;
      uint8_t last_inputs_p1 = last_q4_p1 & 0x0F;
      if (q4_inputs_p1 != last_inputs_p1) {
        Serial.printf("Q4 P1[3:0] changed: 0x%X -> 0x%X\n", last_inputs_p1, q4_inputs_p1);
        last_q4_p1 = q4_in1;
      }
    }
  }

  // ADC read every 1000 ms
  if ((millis() - tAdc) > 1000) {
    tAdc = millis();
    float v0=0, v1=0, v2=0, v3=0;
    if (adsReadSingleEndedVolts(0, v0) &&
        adsReadSingleEndedVolts(1, v1) &&
        adsReadSingleEndedVolts(2, v2) &&
        adsReadSingleEndedVolts(3, v3)) {
      Serial.printf("ADC: A0=%.3fV A1=%.3fV A2=%.3fV A3=%.3fV\n",
                    (double)v0, (double)v1, (double)v2, (double)v3);
    } else {
      Serial.println("ADC read failed (check address/power/pullups).");
    }
  }
}
