#include <iostream>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <pigpiod_if2.h>
#include "craft_hardware/angular_servo.hpp"

using namespace std;

// TODO revisit private vs protected vars - maybe swap to private 

Servo::Servo(int pi, int pin) {
    __pi = pi;
    __pin = pin;
    int rc;
    rc = set_mode(__pi, __pin, PI_OUTPUT);
    if (rc < 0){
        throw "Invalid GPIO pin or mode Error!";
    }
    
    // initializing to zero with pigpio is no input
    rc = set_servo_pulsewidth(__pi, __pin, 0);
    if (rc < 0) {
        throw "Invalid user GPIO pin or pulsewidth Error!";
    }
}

int Servo::getPulseWidth() {
    int rc = get_servo_pulsewidth(__pi, __pin);
    if (rc < 0) {
        throw "Invalid user GPIO pin Error!";
    }
    return rc;
}

void Servo::setPulseWidth(int pulseWidth){
    int rc = set_servo_pulsewidth(__pi, __pin, pulseWidth);
    if (rc < 0) {
        throw "Invalid user GPIO pin or pulsewidth Error!";
    }
}

AngularServo::AngularServo(int pi, int pin, float minAngle, float maxAngle, int minPulseWidthUs, int maxPulseWidthUs) : Servo(pi, pin) {
            __minAngle = minAngle;
            __maxAngle = maxAngle;
            __minPulseWidthUs = minPulseWidthUs;
            __maxPulseWidthUs = maxPulseWidthUs;
}

void AngularServo::setAngle(float angle){
    __angle = angle;
    // make sure angle is within bounds
    if (__angle < __minAngle){
        __angle = __minAngle;
    }
    if (__angle > __maxAngle){
        __angle = __maxAngle;
    }
    int pulseWidth = __minPulseWidthUs + (__angle - __minAngle) * (__maxPulseWidthUs - __minPulseWidthUs) / (__maxAngle - __minAngle);

    setPulseWidth(pulseWidth);
}

int AngularServo::getAngle(){
    int pulseWidth = getPulseWidth();
    float angle = __minAngle + (pulseWidth - __minPulseWidthUs) * (__maxAngle - __minAngle) / (__maxPulseWidthUs - __minPulseWidthUs);
    return angle;
}

bool isPigpiodRunning() {
    int result = system("pgrep pigpiod");

    if (result == 0) {
        // pigpiod daemon is running
        return true;
    } else {
        // pigpiod daemon is not running
        return false;
    }
}

void killPigpiod() {
    if (isPigpiodRunning()) {
        int result = system("sudo killall pigpiod -q");

        if (result == 0) {
            // Successfully killed the pigpiod daemon
            cout << "pigpiod daemon killed successfully" << endl;
        } else {
            // An error occurred while trying to kill the pigpiod daemon
            cerr << "Error killing pigpiod daemon. Return code: " << result << endl;
        }
    } else {
        cout << "pigpiod daemon is not running" << endl;
    }
}

void startPigpiod() {
    if (!isPigpiodRunning()) {
        cout << "pigpio daemon not running. attempting to start it." << endl;
        system("sudo systemctl start pigpiod");
        sleep(1);

        if (isPigpiodRunning()) {
            // Successfully started the pigpiod daemon
            cout << "pigpiod daemon started successfully" << endl;
        } else {
            // An error occurred while trying to start the pigpiod daemon
            cerr << "Error starting pigpiod daemon." << endl; 
        }
    } else {
        cout << "pigpiod daemon is already running" << endl;
    }
}
