#include "mbed.h"
#include "string"

/*
 * Motor Control Elements:
 * 2 PWMs for each of the two motors
 * 2 DigitalOut signals to control the motor direction
 */
PwmOut driveMotor(p24);
PwmOut loadMotor(p23);
DigitalOut loadControl(p5);
DigitalOut superCapCharge(p10);
AnalogIn encoder(p20);
Timer encoderTimer;
float tickerInterval = 0.025f;
float drivePulsewidth;
float loadPulsewidth;
float accelerationFactor = 0.4f;
Ticker speedPIDTicker;
Ticker loadSimulation;
float loadPWMValues [13] = {0.0f, 0.1f, 0.1f, 0.1f, 0.0f, 0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Pulsewidths for load motor
int loadControlValues[13] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1};
float elevations[13] = {0, 10, 20, 30, 40, 50, 50, 50, 30, 30, 20, 10, 0};
float elevations2[13] = {0, 15, 31, 15, 0, 10, 20, 30, 75, 30, 20, 10, 0};
int loadTickerIndex;
float loadInterval = 1.0f;
float elevation;

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
DigitalOut boostLED(p7);
DigitalOut buckLED(p8);

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

int sim1;

void setBatteryVoltage() {
    currentBatteryVoltage = batteryRefVoltage.read() * 3.3f * 11;
}

void calculateBatteryCurrent() {
    setBatteryVoltage();
    currentBatteryCurrent = (currentBatteryVoltage - 
        (batteryVoltage.read() * 3.3f * 11)) / 6.953f;
}

void setSuperCapVoltage() {
    currentSuperCapVoltage = superCapRefVoltage.read() * 3.3f * 11;
}

void calculateSuperCapCurrent() {
    setSuperCapVoltage();
    currentSuperCapCurrent = (currentSuperCapVoltage -
        (superCapVoltage.read() * 3.3f * 11)) / 6.953f;
}

/*
 * **************** Motor Control Methods *****************
 */
 
void encoderRise() {
    float timePassed = encoderTimer.read();
    encoderFrequency = 1 / timePassed;
    if(encoderFrequency < 1000) {
        //pc.printf("Time Passed: %f\r\n", timePassed);
        //pc.printf("Frequency: %f\r\n", encoderFrequency);
    }
    encoderTimer.reset();
}
 
void sendDriveSignal() {
    driveMotor.write(drivePulsewidth);
}
void sendLoadSignal() {
    loadMotor.write(loadPulsewidth);
}

void loadTicker() {
    loadPulsewidth = loadPWMValues[loadTickerIndex];
    loadControl = loadControlValues[loadTickerIndex];
    loadTickerIndex++;
    sendLoadSignal();
}

void calculateTargetPulsewidth() {
    float desiredInchesPerHour = targetMPH * 5280 * 12;
    float desiredInchesPerMinute = desiredInchesPerHour / 60;
    float desiredFrequency = desiredInchesPerMinute / wheelCircumfrence;
    targetPulsewidth = desiredFrequency / 1129;
}

void updateCurrentMPH() {
    float frequency = drivePulsewidth * 1129;
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
    //pc.printf("Current MPH: %f\r\n", currentMPH);
    //pc.printf("Target MPH: %f\r\n", targetMPH);
    //pc.printf("Target PW: %f\r\n", targetPulsewidth);
    //pc.printf("Current PW: %f\r\n", drivePulsewidth);
}

void sendBuckSignal(float pulsewidth) {
    buckPWM.write(pulsewidth);
    boostPWM.write(0.00f);
    buckLED = 1;
    boostLED = 0;
    superCapCharge = 1;
}

void sendBoostSignal(float pulsewidth) {
    buckPWM.write(0.00f);
    boostPWM.write(pulsewidth);
    buckLED = 0;
    boostLED = 1;
    superCapCharge = 0;
}

void buckBoostZero() {
    buckPWM.write(0.0f);
    boostPWM.write(0.0f);
    buckLED = 0;
    boostLED = 0;
    superCapCharge = 0;
}

void updateElevation() {
    /*
    float currentMPS = currentMPH / 60;
    float currentFPS = currentMPS * 5280;
    float elevationChange = (currentFPS * loadInterval * loadPulsewidth);
    if(loadControl) { elevationChange = elevationChange * -1; } 
    elevation += elevationChange*/
    if(sim1) {
        elevation = elevations[loadTickerIndex];
    } else {
        elevation = elevations2[loadTickerIndex];
    }
}

void sendMatlabData() {
    char superCapVoltageBuffer[20];
    char currentMPHBuffer[20];
    char batteryVoltageBuffer[20];
    char elevationBuffer[20];
    char dataPacketBuffer[100];

    sprintf(elevationBuffer, "%f", elevation);
    sprintf(superCapVoltageBuffer, " %f", currentSuperCapVoltage);
    sprintf(currentMPHBuffer, " %f", currentMPH);
    sprintf(batteryVoltageBuffer, " %f", currentBatteryVoltage);

    strcpy(dataPacketBuffer, elevationBuffer);
    strcat(dataPacketBuffer, superCapVoltageBuffer);
    strcat(dataPacketBuffer, currentMPHBuffer);
    strcat(dataPacketBuffer, batteryVoltageBuffer);
    
    pc.printf("%s\n", dataPacketBuffer);
}

/*
 * *************** Main Function ***************
 */

int main()
{
    
    // Initialize global variables */
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
    loadPulsewidth = 0.00f;
    loadTickerIndex = 0;
    elevation = 0;
    sim1 = 1;
   
    // Initialize PWM global period and individual pulsewidths */
    boostPWM.period(0.001f); // Set period for all PWMs to 1 ms
    boostPWM.write(0.25f);
    buckPWM.write(0.00f); // Initialize to low
    sendDriveSignal();
    sendLoadSignal();

    // Initialize speed/encoder ticker and interrupt */
    encoderTimer.start();
    speedPIDTicker.attach(&speedPIDController, 0.6f);
    loadSimulation.attach(&loadTicker, loadInterval);

    // Initialize battery information ticker */
    batteryStats.attach(&calculateBatteryCurrent, circuitStatInterval);
    superCapStats.attach(&calculateSuperCapCurrent, circuitStatInterval);
    
    int readyToRead = 1;
    int readCount = 0;
    elevation = 0.0f;
    
    pc.printf("Initialized all variables\r\n");
    buckLED = 0;
    boostLED = 1;

    while(1) {
        calculateTargetPulsewidth();
        updateCurrentMPH();
        updateElevation();
        sendMatlabData();
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
        
        if(sim1) {
            if(loadTickerIndex > 1 && loadTickerIndex < 6) {
                sendBoostSignal(0.25f);
            } else if(loadTickerIndex > 6) {
                sendBuckSignal(0.25f);
            } else if(loadTickerIndex >= 11) {
                buckBoostZero();
            }
        } else {
            if(loadTickerIndex > 5 && loadTickerIndex < 8) {
                sendBoostSignal(0.25f);
            } else if(loadTickerIndex > 8) {
                if(loadTickerIndex > 11) { 
                    buckBoostZero();
                } else {
                    sendBuckSignal(0.25f);
                {
            } 
        }
        
        
        // Control System Conditions
        if(currentBatteryCurrent < 0) {
            sendBuckSignal(0.25f);
            //pc.printf("Bucking ==> Reversed Current\r\n");
        } else if(currentBatteryCurrent > 0.10f) {
            if(currentBatteryCurrent > 0.20f) {
                sendBoostSignal(0.5f);
            } else if(currentBatteryCurrent > 0.1875f) {
                sendBoostSignal(0.4f);
            } else if(currentBatteryCurrent > 0.175f) {
                sendBoostSignal(0.3f);
            } else if(currentBatteryCurrent > 0.1625f) {
                sendBoostSignal(0.2f);
            } else if(currentBatteryCurrent > 0.15f) {
                sendBoostSignal(0.1f);
            }
            //pc.printf("Boosting ==> More Current Needed\r\n");
        } else {
            
            // Load motor assisting the drive motor
            if(loadControl || currentMPH < 20 || currentMPH > 80) {
                if(loadControl) {
                    sendBuckSignal(loadPulsewidth / 10);
                    //pc.printf("Bucking ==> Downhill\r\n");
                } 
                
                if(currentMPH > 80.0f && currentSuperCapVoltage > 14) {
                    sendBoostSignal(0.25f);
                    //pc.printf("Boosting ==> Draining Super Cap\r\n");
                } else if(currentMPH < 20 && currentSuperCapVoltage < 16) {
                    sendBuckSignal(0.25f);
                    //pc.printf("Bucking ==> Charging Super Cap\r\n");
                }
            } else {
                buckBoostZero();
            }
        }
    } 
}