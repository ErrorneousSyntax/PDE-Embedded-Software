#include <AccelStepper.h> 
#include <TMCStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// -------------------------------- PIN DEFINITIONS --------------------------------

// Stepper 
#define STEP_PIN    18
#define DIR_PIN     19
#define EN_PIN      15
#define RX_PIN      16
#define TX_PIN      17
// STEPPER DRIVER CONFIG
#define MICROSTEPS      8
#define STEPS_PER_REV   200
#define MAX_SPEED     12000
#define ACCELERATION  20000
#define R_SENSE     0.11f

//SCREEN
#define TFT_CS    27
#define TFT_DC    26
#define TFT_RST   25
#define TFT_SCLK  22
#define TFT_MOSI  21

// BUYTTON PINS
#define UP_BTN  
#define OK_BTN  
#define DOWN_BTN    


// -------------------------------- OBJECT CREATION --------------------------------

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,TFT_DC,TFT_MOSI,TFT_SCLK,TFT_RST);
TMC2208Stepper driver(&Serial2, R_SENSE);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
MPU6050 mpu;


// -------------------------------- DEFINING VARIABLES --------------------------------

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



    //avoid measurement where linear acceleration is significant
    float accMag = sqrt(AccX*AccX + AccY*AccY + AccZ*AccZ);
    bool accelReliable = abs(accMag - 1.0) < 0.15;  // tune 0.1–0.25

    float alpha;
    if (accelReliable) {
        alpha = 0.9;   // gyro dominant, slow accel correction
    } else {
        alpha = 1.0;    // ignore accel, gyro only
    }
    // Complementary filter
    pitch = alpha * (pitch + pitchGyro * elapsedTime) + (1-alpha) * pitchAccel;
    roll  = alpha * (roll  + rollGyro  * elapsedTime) + (1-alpha) * rollAccel;

    return {round(roll), round(pitch)};
}

bool isValidAngle(int(roll),int(pitch)){
    if (abs(roll)<10 && abs(pitch)<10){
        digitalWrite(25,LOW);
        return true;
    }
    else{
        digitalWrite(25,HIGH);
        return false;
    }
}



// Gets the screen up and working
// void setup() {
//   SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
//   tft.initR(INITR_GREENTAB);
//   tft.setRotation(0);
//   tft.fillScreen(ST77XX_WHITE);
//   tft.setTextColor(ST77XX_BLACK);
//   tft.setTextSize(2);
//   tft.setCursor(15,40);
//   tft.println("HELLO");
//   tft.setCursor(15,80);
//   tft.println("PIPETTE");
// }



void moveStepper(int steps) {
    stepper.move(steps);          // relative move from current position

    while (stepper.distanceToGo() != 0) {
        stepper.run();            // must be called repeatedly
    }
}

// void buttonClickTest(){
//     if (digitalRead(32) == HIGH) {
//         moveStepper(3200);        // 1 revolution at 200 steps/rev × 8 microsteps
//         delay(300);
//     } else if (digitalRead(33) == HIGH) {
//         moveStepper(-3200);        // 1 revolution at 200 steps/rev × 8 microsteps
//         delay(300);
//     }
// }



void setup() {
    Serial.begin(115200);
    delay(1000);

    //IMU
    Wire.begin(33, 32);
    delay(100);
    mpu.initialize();
    Serial.println(mpu.testConnection() ? "MPU6050 connected" : "MPU6050 failed");
    currentTime = millis();
    digitalWrite(EN_PIN, HIGH);   // disable driver while configuring 

    // ------------- Driver------------- 
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

    // //LEDs and other shit
    // pinMode(25, OUTPUT);
    // pinMode(32, INPUT_PULLDOWN); //button with pulldown resistor in place -> prevents floating (noise, nearby wires etc.)
    // pinMode(33, INPUT_PULLDOWN); // pull back button for testing
}


void loop() {
    Angles angles = getAngle();
    Serial.print("Roll: ");
    Serial.println(angles.roll);
    Serial.print("Pitch: ");
    Serial.println(angles.pitch);
    delay(50);

    // TESTING STEPPER
    // stepper.run();

    // button press at valid angle
//     Angles angles = getAngle();
//     if(isValidAngle(angles.roll,angles.pitch)){
//         buttonClickTest();
//     };
}


