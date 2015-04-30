import urllib2
import json
import serial

def formatAddressData (raw_address):
	replacedSpace = raw_address.replace(" ","+")
	return replacedSpace[1:-1]

geocodeURL = "http://maps.googleapis.com/maps/api/geocode/json?address="
elevationURL = "https://maps.googleapis.com/maps/api/elevation/json?"

address = "\"" + raw_input("Enter your address: ") + "\""
city = "\"" + raw_input("Enter your city: ") + "\""
state = "\"" + raw_input("Enter your state (two-lettered format): ") + "\""

formattedAddress = formatAddressData(address)
formattedCity = ",+" + formatAddressData(city)
formattedState = ",+" + formatAddressData(state)
formattedData = formattedAddress + formattedCity + formattedState
sensorString = "&sensor=false"
keyString = "&key=AIzaSyDOmcetVxUPj3vekfHCLbt69cBLs2-5B1o"

fullGeocodeURL = geocodeURL + formattedData + sensorString
geocodeResponse = urllib2.urlopen(fullGeocodeURL)
geocodeJSON = json.loads(geocodeResponse.read())
longitude = geocodeJSON["results"][0]["geometry"]["location"]["lng"]
latitude = geocodeJSON["results"][0]["geometry"]["location"]["lat"]

locationString = "locations=" + str(latitude) + "," + str(longitude)
fullElevationURL = elevationURL + locationString + sensorString
elevationResponse = urllib2.urlopen(fullElevationURL)
elevationJSON = json.loads(elevationResponse.read())
elevation = elevationJSON["results"][0]["elevation"]

mbed = serial.Serial("/dev/tty.usbmodem1412", 9600)
mbed.write(elevation)