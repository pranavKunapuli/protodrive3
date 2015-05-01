import serial
import matplotlib.pyplot as plt
import numpy as np

previousLine = ""
superCapVoltage = []
batteryVoltage = []
elevation = []
index = []
indexCount = 0

def compareToLast(line, previousLine, elevation, superCapVoltage, batteryVoltage, speed):
	newData = line.split()
	previousLine = line
	if(len(newData) == 4):
		elevation.append(newData[0])
		newSCValue = float(newData[1])
		newSpeed = float(newData[2])
		newBValue = float(newData[3])
		superCapVoltage.append(newSCValue)
		batteryVoltage.append(newBValue)
		speed.append(newSpeed)
		return True
	else:
		return False

def setIndex(length):
	index = []
	for i in range(0, length):
		index.append(i)
	return index

def drawDataPoints(superCapVoltage, batteryVoltage, elevation, sampleLimit, speed):
	plt.figure(1)
	plt.subplot(211)
	index = setIndex(len(superCapVoltage))
	plt.plot(index, superCapVoltage, "ro")
	plt.ylabel("Super Capacitor Voltage")
	plt.axis([0, 100, 0, 23])
	
	plt.subplot(212)
	index = setIndex(len(batteryVoltage))
	plt.plot(index, batteryVoltage, "bs")
	plt.ylabel("Battery Voltage")
	plt.axis([0, sampleLimit, 0, 18])

	plt.figure(2)
	plt.subplot(211)
	index = setIndex(len(elevation))
	plt.plot(index, elevation, "bo")
	plt.ylabel("Elevation (feet)")
	plt.axis([0, sampleLimit, 0, 80])

	plt.subplot(212)
	index = setIndex(len(speed))
	plt.plot(index, speed, "rs")
	plt.ylabel("Speed (mph)")
	plt.axis([0, sampleLimit, 0, 80])
	plt.show()

def main():	
	previousLine = ""
	superCapVoltage = []
	batteryVoltage = []
	elevation = []
	speed = []
	mbed = serial.Serial("/dev/tty.usbmodem1412", 9600)
	n = 0
	sampleLimit = 200

	while(n < sampleLimit):
		line = mbed.readline()
		print(line)
		result = compareToLast(line, previousLine, elevation, 
			superCapVoltage, batteryVoltage, speed)
		n += 1

	drawDataPoints(superCapVoltage, batteryVoltage, elevation, sampleLimit, speed)

main()