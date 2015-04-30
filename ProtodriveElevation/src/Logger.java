import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.*;


public class Logger {
	private static Logger instance;
	private String filename = "log.txt";
	private BufferedWriter writer;
	
	private Logger() {
		try {
			writer = new BufferedWriter(new FileWriter(filename));
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}
	
	public static Logger getInstance() {
		if(instance == null) {
			instance = new Logger();
		}
		return instance;
	}
	
	public void log(String text) {
		String timestamp = new SimpleDateFormat("MM/dd/yyyy h:mm:ss a").format(new Date());
		try {
			writer.write(timestamp + ") " + text);
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}
	
	public void close() {
		try {
			writer.close();
		} catch (IOException e) {
			System.out.println(e.getMessage());
		}
	}
}
