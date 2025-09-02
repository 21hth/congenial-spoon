#include <Wire.h>
#include "TCA9548A.h"
#include "MPU9250_asukiaaa.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

TCA9548A tca(0x70);

MPU9250_asukiaaa mpuNeck;
MPU9250_asukiaaa mpuBody;

const int NECK_CHANNEL = 0;
const int BODY_CHANNEL = 1;

float neckAngleOffset = 0.0;
float bodyAngleOffset = 0.0;

bool isBadPosture = false;
bool warningSent = false;
unsigned long badPostureStartTime = 0;
const unsigned long BAD_POSTURE_THRESHOLD_MS = 240UL * 1000UL;
const float ANGLE_THRESHOLD_DEGREES = 15;

bool isSetupDone = false;

void initSensor(int channel, MPU9250_asukiaaa &mpu, const char* label) {
  tca.openChannel(channel);
  Serial.print("초기화 중... ");
  Serial.print(label);
  
  mpu.beginAccel();
  
  Serial.println(" -> 완료!");
  tca.closeChannel(channel);
}

float readPitch(int channel, MPU9250_asukiaaa &mpu) {
  tca.openChannel(channel);
  mpu.accelUpdate();

  float ax = mpu.accelX();
  float ay = mpu.accelY();
  float az = mpu.accelZ();

  float pitch = atan2(-ax, s세_알리미'를 시작합니다.");

  tca.begin();
  Serial.println("TCA9548A 초기화 완료");

  initSensor(NECK_CHANNEL, mpuNeck, "MPU (Neck) on Channel 0");
  initSensor(BODY_CHANNEL, mpuBody, "MPU (Body) on Channel 1");
}

void loop() {
  if (SerialBT.connected()) {
    if (SerialBT.available()) {
      int receivedValue = SerialBT.read();
      if (receivedValue == '2') {
        Serial.println("5초 뒤 자세를 측정합니다.");
        delay(5000);
        setReferenceAngles();
        isSetupDone = true;
        Serial.println("기준 자세 설정 완료. 자세 모니터링을 시작합니다.");
        SerialBT.print('3');
        Serial.println("Sent '3' to smartphone.");
      }
    }
    
    float neckPitch = readPitch(NECK_CHANNEL, mpuNeck);
    
    float bodyPitch = readPitch(BODY_CHANNEL, mpuBody);

    float relativeAngleChange = abs((neckPitch - neckAngleOffset) - (bodyPitch - bodyAngleOffset));
    
    Serial.print("상대적 각도 변화: ");
    Serial.print(relativeAngleChange, 2);
    Serial.println("도");

    if (relativeAngleChange > ANGLE_THRESHOLD_DEGREES) {
      if (!isBadPosture) {
        isBadPosture = true;
        badPostureStartTime = millis();
        warningSent = false;
      }
      
      if (millis() - badPostureStartTime >= BAD_POSTURE_THRESHOLD_MS) {
        if (!warningSent) {
          SerialBT.print('5');
          Serial.println("경고: 나쁜 자세 감지! 자세를 바르게 해주세요.");
          warningSent = true;
        }
      }
    } else {
      isBadPosture = false;
      badPostureStartTime = 0;
      if (warningSent) {
        SerialBT.print('4');
        Serial.println("자세 정상화, 4 전송.");
      }
      warningSent = false;
    }

    delay(500);
  } else {
    isSetupDone = false;
    isBadPosture = false;
    warningSent = false;
    badPostureStartTime = 0;
    delay(1000);
  }

}
