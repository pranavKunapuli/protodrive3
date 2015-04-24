#include "mbed.h"

/*
 * Motor Control Elements:
 * 2 PWMs for each of the two motors
 * 2 DigitalOut signals to control the motor direction
 */
PwmOut driveMotor(p24);
PwmOut loadMotor(p23);
DigitalOut loadControl(p5);
AnalogIn encoder(p20);
Timer encoderTimer;
float tickerInterval = 0.025f;
float drivePulsewidth;
float loadPulsewidth;
float accelerationFactor = 0.4f;
Ticker speedPIDTicker;
Ticker loadSimulation;
float loadPWMValues [9] = {0.0f, 0.1f, 0.1f, 0.1f, 0.0f, 0.1f, 0.1f, 0.1f, 0.0f}; // Pulsewidths for load motor
int loadControlValues [9] = {0, 0, 0, 0, 0, 1, 1, 1, 1}; // 0 ==> Oppose drive motor | 1 ==> Assist load motor
int loadTickerIndex;

AnalogIn batteryVoltage(p15);
AnalogIn batteryRefVoltage(p16);
AnalogIn superCapVoltage(p17);
AnalogIn superCapRefVoltage(p18);
Ticker batteryStats;
Ticker superCapStats;
float circuitStatInterval = 0.020f;

float wheelRadius = 25.6f; // Assuming a wheel radius of 25.6 inches
float wheelCircumfrence = 160.85f;

PwmOut buckPWM(p22);
PwmOut boostPWM(p21);

float encoderHighCount; // How fast the wheels are spinning
float encoderFrequency;
float currentMPH;
float targetMPH;
float targetPulsewidth;

float currentBatteryVoltage;
float currentSuperCapVoltage;
float currentBatteryCurrent;
float currentSuperCapCurrent;

Serial pc(USBTX, USBRX);

void setBatteryVoltage() {
    currentBatteryVoltage = batteryRefVoltage.read() * 11;
}

void calculateBatteryCurrent() {
    setBatteryVoltage();
    currentBatteryCurrent = (currentBatteryVoltage - 
        (batteryVoltage.read() * 11)) / 6.9;
}

void setSuperCapVoltage() {
    currentSuperCapVoltage = superCapRefVoltage.read() * 11;
}

void calculateSuperCapCurrent() {
    setSuperCapVoltage();
    currentSuperCapCurrent = (currentSuperCapVoltage -
        (superCapVoltage.read() * 11)) / 6.9;
}

/*
 * **************** Motor Control Methods *****************
 */

void loadTicker() {
    loadPulsewidth = loadPWMValues[loadTickerIndex];
    loadControl = loadControlValues[loadTickerIndex];
    loadTickerIndex++;
    sendLoadSignal();
}

void encoderRise() {
    float timePassed = encoderTimer.read();
    encoderFrequency = 1 / timePassed;
    if(encoderFrequency < 1000) {
        pc.printf("Time Passed: %f\r\n", timePassed);
        pc.printf("Frequency: %f\r\n", encoderFrequency);
    }
    encoderTimer.reset();
}
 
void sendDriveSignal() {
    driveMotor.write(drivePulsewidth);
}
void sendLoadSignal() {
    loadMotor.write(loadPulsewidth);
}

void calculateTargetPulsewidth() {
    float desiredInchesPerHour = targetMPH * 5280 * 12;
    float desiredInchesPerMinute = desiredInchesPerHour / 60;
    float desiredFrequency = desiredInchesPerMinute / wheelCircumfrence;
    targetPulsewidth = desiredFrequency / 1129;
}

void updateCurrentMPH() {
    float frequency = 1129 * drivePulsewidth;
    float inchesPerMinute = frequency * wheelCircumfrence;
    float inchesPerHour = inchesPerMinute * 60;
    currentMPH = ((inchesPerHour / 12) / 5280);
}

void speedPIDController() {
    float speedDifference = targetMPH - currentMPH;
    float pulsewidthDiff;

    if(speedDifference > 0) {
        pulsewidthDiff = targetPulsewidth - drivePulsewidth;
        if(pulsewidthDiff < 0.05f) {
            drivePulsewidth = targetPulsewidth;
        } else {
            drivePulsewidth += pulsewidthDiff * accelerationFactor;
        }
    } else if (speedDifference < 0) {
        pulsewidthDiff = drivePulsewidth - targetPulsewidth;
        if(pulsewidthDiff < 0.05f) {
            drivePulsewidth = targetPulsewidth;
        } else {
            drivePulsewidth -= pulsewidthDiff * accelerationFactor;
        }
    }
    
    sendDriveSignal(); // Push the changed pulsewidth to the motor
    updateCurrentMPH();
    pc.printf("Current MPH: %f\r\n", currentMPH);
    pc.printf("Target MPH: %f\r\n", targetMPH);
    pc.printf("Target PW: %f\r\n", targetPulsewidth);
    pc.printf("Current PW: %f\r\n", drivePulsewidth);
}

/*
 * *************** Main Function ***************
 */

int main()
{
    /* Initialize global variables */
    encoderHighCount = 0; // How fast the wheels are spinning
    encoderFrequency = 0;
    currentMPH = 86;
    targetMPH = 86;
    targetPulsewidth = 0;
    currentBatteryVoltage = 0;
    currentSuperCapVoltage = 0;
    currentBatteryCurrent = 0;
    currentSuperCapCurrent = 0;
    drivePulsewidth = 0.50f;
    loadPulsewidth = 0.1f;
    loadTickerIndex = 0;
   
    /* Initialize PWM global period and individual pulsewidths */
    boostPWM.period(0.001f); // Set period for all PWMs to 1 ms
    boostPWM.write(0.0f); // Initialize to low
    buckPWM.write(0.0f); // Initialize to low
    sendDriveSignal();
    sendLoadSignal();

    /* Initialize speed/encoder ticker and interrupt */
    encoderTimer.start();
    speedPIDTicker.attach(&speedPIDController, 0.6f);
    loadSimulation.attach(&loadTicker, 1.0f);

    /* Initialize battery information ticker */
    batteryStats.attach(&calculateBatteryCurrent, circuitStatInterval);
    superCapStats.attach(&calculateSuperCapCurrent, circuitStatInterval);
    
    int readyToRead = 1;
    int readCount = 0;
    
    pc.printf("Initialized all variables\r\n");

    while(loadTickerIndex < 9) {
        calculateTargetPulsewidth();
        updateCurrentMPH();
        float encoderVoltage = encoder.read() * 3.3;
        
        if(readyToRead && encoderVoltage > 2) {
            readCount++;
            float timePassed = encoderTimer.read();
            float tempFreq = 1 / timePassed;
            if(tempFreq < 800.0f) {
                encoderFrequency = ((encoderFrequency * (readCount - 1)) + tempFreq) / readCount;
                //pc.printf("Frequency: %f\r\n", encoderFrequency);
            } 
            encoderTimer.reset();
            readyToRead = 0;
        } else if(!readyToRead && encoderVoltage < 0.8) {
            readyToRead = 1;
        }
        
        //pc.printf("Encoder Voltage: %f\r\n", encoderVoltage);
        
        /* Control System Conditions */
        if(currentBatteryCurrent < 0) {
            buckPWM.write(0.25f);
            boostPWM.write(0.0f);
        } else if(currentBatteryCurrent > 0.15f) {
            buckPWM.write(0.0f);
            boostPWM.write(0.25f);
        } else {
            // Load motor assisting the drive motor
            if(loadControl) {
                buckPWM.write(loadPulsewidth / 5);
            } else {
                buckPWM.write(0.0f);
                boostPWM.write(0.0f);
            }
        }
    }
}