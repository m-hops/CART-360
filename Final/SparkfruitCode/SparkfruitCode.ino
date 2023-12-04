#include <Wire.h>
#include <SPI.h>
#include <SFE_LSM9DS0.h>

#define LSM9DS0_XM 0x1D
#define LSM9DS0_G 0x6B
#define PRINT_CALCULATED
#define PRINT_SPEED 500

const byte INT1XM = 9;

LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

void DOFConfig() {

  dof.setAccelScale(dof.A_SCALE_4G);
}

void DOFLoop() {

  printAccel();
}

void printAccel() {

    dof.readAccel();
    
    Serial.print("A: ");
  #ifdef PRINT_CALCULATED
    Serial.print(dof.calcAccel(dof.ax), 2);
    Serial.print(", ");
    Serial.print(dof.calcAccel(dof.ay), 2);
    Serial.print(", ");
    Serial.println(dof.calcAccel(dof.az), 2);
  #elif defined PRINT_RAW 
    Serial.print(dof.ax);
    Serial.print(", ");
    Serial.print(dof.ay);
    Serial.print(", ");
    Serial.println(dof.az);
  #endif

}

void setup() {
  
  Serial.begin(115200);
  uint16_t status = dof.begin();

  pinMode(INT1XM, INPUT);

  DOFConfig();
}

void loop() {
  DOFLoop();

}
