#include "mbed.h"

/*
 * Motor Control Elements:
 * 2 PWMs for each of the two motors
 * 2 DigitalOut signals to control the motor direction
 */
PwmOut driveMotor(p24);
PwmOut loadMotor(p23);
DigitalOut loadControl(p5);
InterruptIn encoder(p6);
Ticker encoderTicker;
float ticketInterval = 0.025f;
float drivePulsewidth;
float loadPulsewidth;
float accelerationFactor = 0.25f;

AnalogIn batteryVoltage(p15);
AnalogIn batteryRefVoltage(p16);
AnalogIn superCapVoltage(p17);
AnalogIn superCapRefVoltage(p18);
Ticker batteryStats;
Ticker superCapStats;
float circuitStatInterval = 0.025f;

float wheelRadius = 25.6; // Assuming a wheel radius of 25.6 inches

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

/*
 * Triggers every time we get a rising edge
 * and counts the number of triggers in a given
 * time period.
 */
void encoderRise() {
	encoderHighCount++;
}

/*
 * Uses the number of high triggers from the encoder signal
 * and calculates the speed in miles per hour according to 
 * a pre-specified wheel radius
 */
void encoderRPMCalculator() {
	encoderFrequency = encoderHighCount / ticketInterval;
	float circumfrence = 2 * 3.1415 * wheelRadius;
	float inchesPerMinute = encoderFrequency * circumfrence;
	float inchesPerHour = inchesPerMinute * 60;
	currentMPH = ((inchesPerHour / 12) / 5280);
}

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

void sendDriveSignal() {
	driveMotor.write(drivePulsewidth);
}
void sendLoadSignal() {
	loadMotor.write(loadPulsewidth);
}

void calculateTargetPulsewidth() {
	float desiredInchesPerHour = desiredMPH * 5280 * 12;
	float desiredInchesPerMinute = desiredInchesPerHour / 60;
	float desiredFrequency = desiredInchesPerMinute / 160;
	targetPulsewidth = desiredFrequency / 840;
}

void speedPIDController(int drive) {
	float speedDifference = desiredMPH - currentMPH;
	calculateTargetPulsewidth();
	float pulsewidthDiff;

	if(speedDifference > 0) {
		if(drive) {
			pulsewidthDiff = targetPulsewidth - drivePulsewidth;
			if(pulsewidthDiff < 0.15f) {
				drivePulsewidth = targetPulsewidth;
			} else {
				drivePulsewidth += pulsewidthDiff * accelerationFactor;
			}
		} else {
			pulsewidthDiff = targetPulsewidth - loadPulsewidth;
			if(pulsewidthDiff < 0.15f) {
				loadPulsewidth = targetPulsewidth;
			} else {
				loadPulsewidth += pulsewidthDiff * accelerationFactor;
			}
		}
	} else if (speedDifference < 0) {
		if(drive) {
			pulsewidthDiff = drivePulsewidth - targetPulsewidth;
			if(pulsewidthDiff < 0.15f) {
				drivePulsewidth = targetPulsewidth;
			} else {
				drivePulsewidth -= pulsewidthDiff * accelerationFactor;
			}
		} else {
			pulsewidthDiff = loadPulsewidth - targetPulsewidth;
			if(pulsewidthDiff < 0.15f) {
				loadPulsewidth = targetPulsewidth;
			} else {
				loadPulsewidth -= pulsewidthDiff * accelerationFactor;
			}
		}
	}
}

int main()
{
    boostPWM.period(0.001f); // Set period for all PWMs to 1 ms
    boostPWM.write(0.0f); // Initialize to low
    buckPWM.write(0.0f); // Initialize to low
    driveMotor.write(0.0f); // Initialize to low
    loadMotor.write(0.0f); // Initialize to low

    /* Initialize encoder ticker and interrupt */
    encoder.rise(&encoderRise());
    encoderTicker.attach(&encoderRPMCalculator, tickerInterval);

    /* Initialize battery information ticker */
    batteryStats.attach(&calculateBatteryCurrent, circuitStatInterval);
    superCapStats.attach(&calculateSuperCapCurrent, circuitStatInterval);

    while(1) {
        
    }
}