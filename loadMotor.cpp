#include "mbed.h"

PwmOut loadMotor(p23);
AnalogIn loadEncoderYellow(p15);
AnalogIn loadEncoderGreen(p16);
DigitalOut loadDirection(p5);
Timer timer;
Timer freqReader;
Serial pc(USBTX, USBRX); // tx, rx

int loadDirectionLocal;
float loadDuty;
float loadEncoderValue;
float max;
float min;
int count;

void risingEdge() {
    float timePassed = freqReader.read();
    loadEncoderValue = count / timePassed;
    pc.printf("Frequency: %f\r\n", loadEncoderValue);
    freqReader.reset();
}

int main() {
    pc.printf("Starting up\r\n");
    timer.start();
    freqReader.start();
    loadMotor.period(0.01f); // 100 Hz
    loadMotor.write(0.1f); // Initially 50% duty cycle
    max = 0.0f;
    min = 1.0f;
    count = 0;
    
    loadDuty = 0.2f;
    loadDirection = 1;
    
    while(1) {
        float yellow = loadEncoderYellow.read();
        float green = loadEncoderGreen.read();
        float net = green - yellow;
        pc.printf("Net Output: %f\r\n", net);
        
        /* Maximum and minimum value calculations for testing */
        if(net > max) {
            max = net;
        }
        if(net < min) {
            min = net;
        }
        
        if(net > 0.5f) {
            count++;
        }
        
        if(count > 25) {
            risingEdge();
            count = 0;
        }
            
        
        float currentTime = timer.read();
        
        if(currentTime > 5) {
            break;
            loadDuty = loadDuty + 0.05f;
            timer.reset();
        }
        
        if(loadDirectionLocal < 0) {
            loadDirection = 0;
        } else {
            loadDirection = 1;
        }
            
        loadMotor.write(loadDuty);
    }
    
    pc.printf("Max: %f\r\n", max);
    pc.printf("Min: %f\r\n", min);
    risingEdge();
    pc.printf("Final freq: %f\r\n", loadEncoderValue);
}
