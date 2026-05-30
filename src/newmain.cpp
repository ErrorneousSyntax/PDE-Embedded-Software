#include <AccelStepper.h> 
#include <TMCStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>


#define STEP_PIN    18
#define DIR_PIN     19
#define EN_PIN      5
#define RX_PIN      16
#define TX_PIN      17

// STEPPER DRIVER CONFIG
#define MICROSTEPS      8
#define STEPS_PER_REV   200
#define MAX_SPEED   6400
#define ACCELERATION 400

#define R_SENSE     0.11f

TMC2208Stepper driver(&Serial2, R_SENSE);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

MPU6050 mpu;

float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float gyroAngleX = 0, gyroAngleY = 0;
float roll, pitch, yaw;
float elapsedTime;
unsigned long currentTime, previousTime;



//--------------------------------- IMU ANGLE DETECTION ---------------------------------
struct Angles {
    // Two orientations ensures orthogonality 
    float roll; //In one orientation
    float pitch; //The other direction
};

Angles getAngle() {
    int16_t ax, ay, az, gx, gy, gz;

    mpu.getAcceleration(&ax, &ay, &az);
    float AccX = ax / 16384.0;
    float AccZ = az / 16384.0;
    float AccY = ay / 16384.0;

    float pitchAccel = atan2(AccX, AccZ) * 180.0 / PI;
    float rollAccel  = atan2(AccY, AccZ) * 180.0 / PI;

    previousTime = currentTime;
    currentTime  = millis();
    elapsedTime  = (currentTime - previousTime) / 1000.0;

    mpu.getRotation(&gx, &gy, &gz);
    float pitchGyro = (gy / 131.0) ;
    float rollGyro  = (gx / 131.0) ;

    // Same filter as your MicroPython code
    pitch = 0.95 * (pitch + pitchGyro * elapsedTime) + 0.05 * pitchAccel;
    roll  = 0.95 * (roll  + rollGyro  * elapsedTime) + 0.05 * rollAccel;

    return {round(roll), round(pitch)};
}

void moveStepper(){

}



void setup() {
    Serial.begin(115200);
    delay(1000);

    //IMU
    Wire.begin(21, 22);
    delay(100);
    mpu.initialize();
    Serial.println(mpu.testConnection() ? "MPU6050 connected" : "MPU6050 failed");
    currentTime = millis();
    digitalWrite(EN_PIN, HIGH);   // disable driver while configuring 

    // Driver
    Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    driver.begin();
    driver.toff(4);               // lower toff = faster switching, better at high speeds
    driver.rms_current(600);      // ~900mA is safe for most NEMA17s at 9V continuous
    driver.microsteps(MICROSTEPS);
    driver.pwm_autoscale(true);
    driver.pwm_autograd(true);    // auto-tunes PWM gradient for better high-speed performance
    driver.en_spreadCycle(true);  // SpreadCycle gives more torque at high speeds vs StealthChop
    stepper.setMaxSpeed(MAX_SPEED);
    stepper.setAcceleration(ACCELERATION);
    stepper.move(-10000);      // changing this value to calibrate
    digitalWrite(EN_PIN, LOW);    // enable driver after config

    //LEDs and other shit
    pinMode(25, OUTPUT);
}

void isValidAngle(int(roll),int(pitch)){
    if (abs(roll)<10 && abs(pitch)<10){
        digitalWrite(25,HIGH);
    }
    else{
        digitalWrite(25,LOW);
    }
}


void loop() {
    // TESTING IMU
    // Angles angles = getAngle();
    // Serial.print("Roll: ");
    // Serial.println(angles.roll);
    // Serial.print("Pitch: ");
    // Serial.println(angles.pitch);
    // delay(50);

    // TESTING STEPPER
    // stepper.run();
    Angles angles = getAngle();
    isValidAngle(angles.roll,angles.pitch);
}