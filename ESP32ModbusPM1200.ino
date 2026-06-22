// ESP32 Modbus communication with Schneider PowerLogic PM1200 Meter Demo
// By Rodney Tan (PhD)
// Ver 1.00 (Jun 2026)

#include <ModbusMaster.h>
// Hardware Serial 2 pins on ESP32
#define RX2_PIN 16
#define TX2_PIN 17
// RS-485 transceiver DE/RE flow control pin
#define RS485_CONTROL_PIN 4 
#define METER_SLAVE_ID 1 
// instantiate ModbusMaster object
ModbusMaster meter;


void preTransmission() {
  digitalWrite(RS485_CONTROL_PIN, HIGH);
}

void postTransmission() {
  digitalWrite(RS485_CONTROL_PIN, LOW);
}

// Function to convert two 16-bit registers into a single 32-bit float
float registersToFloat(uint16_t reg1, uint16_t reg2) {
  uint32_t combined = ((uint32_t)reg1 << 16) | reg2;
  float result;
  memcpy(&result, &combined, sizeof(float));
  return result;
}


void setup() {
  // Setup Serial Port
  Serial.begin(115200);
  // Schneider PM1200 default configuration is 9600 Baud, 8 Data Bits, Even Parity, 1 Stop Bit
  Serial2.begin(9600, SERIAL_8E1, RX2_PIN, TX2_PIN);
  // Initiate RS-485 transceiver DE/RE flow control
  pinMode(RS485_CONTROL_PIN, OUTPUT);
  digitalWrite(RS485_CONTROL_PIN, LOW);
  meter.preTransmission(preTransmission);
  meter.postTransmission(postTransmission);
  // Initiate ESP32 as Modbus master to communicate with slave
  meter.begin(METER_SLAVE_ID, Serial2);
  Serial.println("ESP32 Modbus Master Initiated");
}

void loop() {
  // 0-based offset for register 3915 in the manual and read 14 words length of register
  uint8_t response = meter.readHoldingRegisters(3914, 14); 
  if (response == meter.ku8MBSuccess) {
    // Get frequency register from buffer
    uint16_t reg3914 = meter.getResponseBuffer(0);
    uint16_t reg3915 = meter.getResponseBuffer(1);
    // Get voltage phase 1 register from buffer
    uint16_t reg3926 = meter.getResponseBuffer(12);
    uint16_t reg3927 = meter.getResponseBuffer(13);
    // Convert register values to Float
    float F = registersToFloat(reg3915, reg3914); // Least Significant Register First, Little endian order 
    float V1 = registersToFloat(reg3927, reg3926); // Least Significant Register First, Little endian order 
    
    Serial.print(F, 2);
    Serial.print(", ");
    Serial.println(V1, 1);
  } else {
    Serial.print("Modbus Read Error! Code: 0x");
    Serial.println(response, HEX);
  }
    
  delay(2000);
}
