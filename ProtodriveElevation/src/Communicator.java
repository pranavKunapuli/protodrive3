import gnu.io.*;

import java.awt.Color;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.TooManyListenersException;

public class Communicator {

    // References the selected port
    private CommPortIdentifier selectedPortIdentifier = null;
    private SerialPort serialPort = null;

    //Output streams for sending data
    private OutputStream output = null;

    //just a boolean flag that i use for enabling
    //and disabling buttons depending on whether the program
    //is connected to a serial port or not
    private boolean bConnected = false;

    //the timeout value for connecting with the port
    final static int TIMEOUT = 2000;

    //some ascii values for for certain things
    final static int SPACE_ASCII = 32;
    final static int DASH_ASCII = 45;
    final static int NEW_LINE_ASCII = 10;
    
    public void connect() {
    	try {
			selectedPortIdentifier = 
					CommPortIdentifier.getPortIdentifier("/dev/tty.usbmodem1412");
		} catch (NoSuchPortException e1) {
			System.out.println(e1.getMessage());
		}
    	
    	CommPort communicationPort = null;
    	
    	try {
    		communicationPort = selectedPortIdentifier.open("mBed", Communicator.TIMEOUT);
    		serialPort = (SerialPort) communicationPort;
    	} catch (Exception e) {
    		System.out.println(e.getMessage());
    	}
    }
    
    public boolean initializeOutputStream() {
    	try {
    		output = serialPort.getOutputStream();
    		return true;
    	} catch (Exception e) {
    		System.out.println(e.getMessage());
    		return false;
    	}
    }
    
    public void send(int elevation) {
    	try {
			output.write(elevation);
			output.flush();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
    
    public void disconnect() {
    	try {
    		output.close();
    	} catch (Exception e) {
    		System.out.println(e.getMessage());
    	}
    }
}
