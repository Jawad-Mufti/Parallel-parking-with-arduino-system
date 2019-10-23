#include <Smartcar.h>

Car car;
SR04 frontSonar;
SR04 backSonar;
GP2Y0A21 sideFrontIR;                    //can only measure between 5 and 25 cm, closer or futher than that is 0 (ithink) other naem:GP2D120
Odometer encoderLeft, encoderRight;
Gyroscope gyro;

const int forwardSpeed = 30;                   //Speed forward
const int backwardSpeed = -30;                  //Speed backward
const int leftDegrees = -65;               //degrees to turn left
const int rightDegrees = 65;                //degrees to turn right

const int frontTrigPin = 6; 
const int frontEchoPin = 7;
const int backTrigPin = A9;
const int backEchoPin = A10;
const int sideFrontPinForIR = A0;
const int encoderLeftPin = 2;
const int encoderRightPin = 3;

int frontDistanceInCm;
int backDistanceInCm;
int irDistanceInCm;                      
int maxFrontDistance = 25;               
int maxBackDistance = 25;
int maxSideDistance = 20;
int odometerLeftDistance, odometerRightDistance;

int spotSize;

void setup() {
  Serial3.begin(9600);
  gyro.attach();   
  
  //attaching pins                     
  frontSonar.attach(frontTrigPin,frontEchoPin);
  backSonar.attach(backTrigPin, backEchoPin);
  sideFrontIR.attach(sideFrontPinForIR);
  attachBeginOdometer();

  car.begin(gyro);
  car.begin(encoderLeft, encoderRight);

}

void loop() {
  handleInput(); 
}

void handleInput() {
  if (Serial3.available()) {
    char input = Serial3.read();
   
    switch (input) {
      case 's':               
        findSpot();           
        break;
      case 'p':               
        parkInSpot();      
        break;
      case 'a':
        findSpotAndPark();
        break;

      default:
        stopCar();
    }
  }
}

//to use park in spot we need to use the findSpot method first (as it is now)
void parkInSpot(){ //need delay 1000 in many places or else it will not turn as it should
  rotateOnSpot(-35);
  Serial3.print("After rotation");
  delay(200);
  getBackDistance();

  int lengthToGoBack = (spotSize / 3)+5; //weird, cuz the size of the spot comes out as very long
  
  if(lengthToGoBack < 5 || lengthToGoBack > 30){
    delay(200);
    Serial.print(lengthToGoBack);
    delay(200);
  }
  else{
    car.go(-lengthToGoBack); //car.go(-25);
    while(car.getSpeed() > 0) {  
    }
    stopCar();
    delay(1000);
    rotateOnSpot(35);
    
    //int turn = gyro.getAngularDisplacement();
   /* if(turn < 0){ //Idk if this if else is correct
      rotateOnSpot(-(turn));
    }else if(turn > 0){
      rotateOnSpot(-turn);
    } */ 
    delay(1000);
    //findMiddle();
    //while(car.getSpeed() > 0) {  
    //}
  }
}

void findSpotAndPark(){ //need to add findSpot() method to use this
  findSpot();
  parkInSpot();
}

////// methods for moving the car //////
void driveForward(){
  car.setSpeed(forwardSpeed);
  car.setAngle(0);
}
void driveBackward(){
  car.setSpeed(backwardSpeed);
  car.setAngle(0);
}
void driveLeft(){
   car.setSpeed(forwardSpeed);
   car.setAngle(leftDegrees);
}
void driveRight(){
  car.setSpeed(forwardSpeed);
  car.setAngle(rightDegrees);
}
void stopCar(){
  car.setSpeed(0);
}

void attachBeginOdometer(){
  encoderLeft.attach(encoderLeftPin);
  encoderRight.attach(encoderRightPin);
  encoderLeft.begin();
  encoderRight.begin();
}

//methods to get distances from sensors
void getFrontDistance(){
  frontDistanceInCm = frontSonar.getDistance(); 
}
void getBackDistance(){
  backDistanceInCm = backSonar.getDistance();
}
void getIRdistance(){
  irDistanceInCm = sideFrontIR.getDistance();
}
void getOdometerLeftDistance(){
  odometerLeftDistance = encoderLeft.getDistance();
}
void getOdometerRightDistance(){
  odometerRightDistance = encoderRight.getDistance();
}

//method to make the car turn
void rotateOnSpot(int targetDegrees) {
  targetDegrees %= 360; 
  if (!targetDegrees){
     return; 
  }
  if (targetDegrees > 0) { 
    car.setMotorSpeed(forwardSpeed, -forwardSpeed); 
  } else { 
    car.setMotorSpeed(-forwardSpeed, forwardSpeed); 
  }
  
  unsigned int initialHeading = gyro.getAngularDisplacement(); 
  int degreesTurnedSoFar = 0; 
  
  while (abs(degreesTurnedSoFar) < abs(targetDegrees)) { 
    gyro.update();
    int currentHeading = gyro.getAngularDisplacement(); 
    
    if ((targetDegrees < 0) && (currentHeading > initialHeading)) { 
      currentHeading -= 360; 
    } else if ((targetDegrees > 0) && (currentHeading < initialHeading)) {
      currentHeading += 360;
    }
    
    degreesTurnedSoFar = initialHeading - currentHeading;   
  }
  stopCar(); 
}
