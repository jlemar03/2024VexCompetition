#include "vex.h"
#include "Timer.h"
#include <thread>

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain Brain;

// VEXcode device constructors
motor leftDrive = motor(PORT9, ratio18_1, false);
motor rightDrive = motor(PORT10, ratio18_1, true);
gyro TurnGyroSmart = gyro(Brain.ThreeWirePort.D);
inertial InterialSensor = inertial(PORT11, left);
smartdrive Drivetrain = smartdrive(leftDrive, rightDrive,
                                   TurnGyroSmart, 319.19, 320, 130, mm, 1);

// VEXcode generated functions

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 *
 * This should be called at the start of your int main function.
 */


static bool stopVexProgram = false; // Below below
long long currentTime = 0; // And below
Timer checkErrors{"AAA"}; // Outside since vex's thread implementation is a little less free than C++'s
double totDegrees = 0; // Outside for the same reason as above


void startSecondThread() {
  while(!stopVexProgram) {
    Brain.Screen.printAt(10, 40, "Angle: %.2f", InterialSensor.rotation(degrees));
    long long time = checkErrors.getTime() - currentTime;
    if(time >= 5000) {
      Brain.Screen.printAt(10, 20, "Main thread is stalling!");
    } else {
      Brain.Screen.printAt(10, 20, "Main thread is working! ");
    }
    this_thread::sleep_for(20);
  }
}

void multiDrive(double totDegrees) {
}

void vexcodeInit(void) {
  InterialSensor.calibrate();
  while(InterialSensor.isCalibrating()) {
    wait(25, msec);
  }
  checkErrors.start();
  thread errorChecker{startSecondThread};
  Timer programLoop{"Total runtime"};
  programLoop.start();
  int tick = 0;
  //InterialSensor.resetHeading();
  while(!stopVexProgram) {
    if(tick >= 4) {
      stopVexProgram = true;
      break;
    }
    // Loop inside here
    // 30 * 3.14 * 1/4 = each wheel
    // 180 degrees = 10.5
    double distanceBetweenWheels = 29.5; // Centimeters
    double wheelDiameter = 10.5; // Centimeters
    double distance = distanceBetweenWheels * M_PI_2; // We don't divide by 4 since we're using 2 wheels to turn
    double rotations = distance / wheelDiameter;
    // totDegrees is outside of the function due to some lambda weirdness down below
    totDegrees = (rotations * 180. / M_PI);

    Drivetrain.setDriveVelocity(30, percent);
    Drivetrain.driveFor(6*12, inches);
    Drivetrain.setDriveVelocity(10, percent);
    if(tick < 3) { // Don't do the last rotation to save time
      // Run one of the motors on another thread so both can turn simultaneously using a lambda
     // -90 180 90 0

      while (InterialSensor.rotation(degrees) < 82) {
        rightDrive.spin(forward);
        leftDrive.spin(reverse);
      }
      
      rightDrive.stop();
      leftDrive.stop();
      InterialSensor.setRotation(0, degrees);

    }
    currentTime = checkErrors.getTime();
    tick++;
  }
  double totalTime = (((double) programLoop.getTime()) / 1000.);
  Brain.Screen.printAt(10, 40, "Total time to run program: %f", totalTime);
}