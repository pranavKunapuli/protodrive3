import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Scanner;

import it.units.GoogleCommon.GeocodeException;
import it.units.GoogleCommon.Location;
import it.units.GoogleElevation.ElevationRequestor;
import it.units.GoogleElevation.ElevationResponse;
import it.units.GoogleGeocoding.*;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.List;

import org.apache.commons.io.IOUtils; 
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import org.mbed.RPC.SerialRPC;

public class Main {

	private static final String URL = "http://maps.googleapis.com/maps/api/geocode/json"; 
	
	public static String getJSONFromGoogle(String fullAddress) {
		URL url = null;
		ByteArrayOutputStream output = null;
		try {
			url = new URL(URL + "?address=" + URLEncoder.encode(fullAddress, "UTF-8")+ "&sensor=false");
			URLConnection connection = url.openConnection();
			output = new ByteArrayOutputStream(1024);
			IOUtils.copy(connection.getInputStream(), output);
			output.close();
		} catch (MalformedURLException | UnsupportedEncodingException e) {
			System.out.println(e.getMessage());
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}

		return output.toString();
	}
	
	public static String getFormattedAddress() {
		Scanner addressInput = new Scanner(System.in);
		String address, city, state, zip, formattedAddress;
		
		System.out.print("Enter an address: ");
		address = addressInput.nextLine().trim();
		System.out.print("Enter the city: ");
		city = addressInput.nextLine().trim();
		System.out.print("Enter the state (two-lettered format): ");
		state = addressInput.nextLine().trim();
		System.out.print("Enter the 5-digit zip code: ");
		zip = addressInput.nextLine().trim();
		
		formattedAddress = address + "," + city + "," + state + "," + zip;
		
		return formattedAddress;
	}
	
	public static void main(String[] args) {
		String formattedAddress = getFormattedAddress();
		Logger.getInstance().log("Requested Address - " + formattedAddress);
		
		Location hamco = new Location();
		
		String googleJSON = getJSONFromGoogle(formattedAddress);
		
		JSONParser parser = new JSONParser();
		double longitude = 0;
		double latitude = 0;
		
		try {
			JSONObject object = (JSONObject) parser.parse(googleJSON);
			JSONArray results = (JSONArray) object.get("results");
			Object[] data = results.toArray();
			JSONObject allData = (JSONObject) data[0];
			JSONObject geometry = (JSONObject) allData.get("geometry");
			JSONObject location = (JSONObject) geometry.get("location");
			longitude = (double) location.get("lng");
			latitude = (double) location.get("lat");
		} catch (ParseException e) {
			System.out.println(e.getMessage());
		}
		
		hamco.setLng(((float) longitude));
		hamco.setLat(((float) latitude));
		System.out.println("Longitude: " + longitude);
		System.out.println("Latitude: " + latitude);
		
		ElevationRequestor elevationRequestor = new ElevationRequestor();
		ElevationResponse elevationResponse;
		int elevation = -1;
		
		try {
			elevationResponse = elevationRequestor.getElevation(hamco);
			String[] results = elevationResponse.toString().split(",");
			System.out.println("Elevation: " + results[2]);
			Logger.getInstance().log("Elevation - " + results[2]);
			elevation = Integer.parseInt(results[2]);
		} catch (GeocodeException e) {
			System.out.println(e.getMessage());
		}
		
		Communicator mbed = new Communicator();
		mbed.connect();
		if(mbed.initializeOutputStream()) {
			if(elevation < 0) {
				mbed.send(elevation);
			}
		} else {
			System.out.println("Could not connect to mBed");
			Logger.getInstance().log("Could not connect to mBed");
		}
	}
}
