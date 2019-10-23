#include <Smartcar.h>
#include<math.h>


Car car;

SR04 frontSonar;
SR04 sideSonar;
SR04 backSonar;
GP2D120 backIR;
Gyroscope gyro(-5);

//Setting needed constants
const int fSpeed = 30; 
const int bSpeed = -25;
const int lDegrees = -75; 
const int rDegrees = 75; 


const int frontTrigPin = 6;
const int frontEchoPin = 7;
const int sideTrigPin = A11;
const int sideEchoPin = A12;
const int backTrigPin = A9;
const int backEchoPin = A10;

const int backIrPin = A0;

const int encoderLeftPin = 2;
const int encoderRightPin = 3;

int const offset = -5;
const int SERVO_PIN = 50;

//Initializing odometer and servo
Odometer encoderLeft(210), encoderRight(210);
Servo myservo;

//Setting needed variables
int spotSize;
int backDistanceInCm;
int frontDistanceInCm;
int sideDistanceInCm;
int irDistanceInCm;

int maxBackDistance = 15;
int maxFrontDistance = 20;
int maxSideDistance = 10;
int maxIrbackDistance = 8;

int pos = 0; // Variable to store the servo position

boolean isOn = false;

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  car.begin(); //Initialize the car using the encoders and the gyro
  gyro.attach();
  gyro.begin();
  sideSonar.attach(sideTrigPin, sideEchoPin);
  backSonar.attach(backTrigPin, backEchoPin);
  frontSonar.attach(frontTrigPin,frontEchoPin);
  encoderLeft.attach(encoderLeftPin);
  encoderRight.attach(encoderRightPin);
  myservo.attach(SERVO_PIN);
  backIR.attach(backIrPin);
  car.begin(encoderLeft,encoderRight);
  car.begin(gyro);
}

// What is executed here depends on the input of the user.
void loop() {
  handleInput();
}


/*
This method handles input from both the Serial or the USB Cable between the Raspberry Pi and Arduino, and the 
input from the Blutooth modeule as Serial3.
*/
void handleInput() {
  if (Serial3.available()) {
    char input = Serial3.read(); //Read everything that has been received so far and log down the last entry.
    switch (input) {
      case 'a': //Find spot and park in spot
        findSpot();
        delay(1000);
        parkInSpot();
        break;

      case 'r': //Turn clock-wise
        car.setSpeed(fSpeed);
        car.setAngle(rDegrees);
        break;

      case 's': //Go forward and look for parking spot
        car.setSpeed(fSpeed);
        car.setAngle(0);
        findSpot();
        break;

      case 'p': //Park in spot
         parkInSpot();
        break;
        
      default: //If input is something that is not 'a', 'r', 's', or 'p', the car stops
        car.setSpeed(0);
        car.setAngle(0);
    }
  }
  //If you have contact with the Raspberry Pi, see if it sends you some input//
  if(Serial.available()){
    int input  = Serial.read();
    switch (input) {

      case '1': //green sign
        if(!isOn){
          car.setSpeed(fSpeed);
          isOn = true;
        }
     //else do nothing, because the car is already driving
      break;

      case '0': //red sign
        if(isOn){
          car.setSpeed(0);
          isOn = false;  
        }
      //else do nothing, because the car has already stopped
      break;
        
      case '3': //we want the car to just drive forward and start looking for signs when we start the camera program
         car.setSpeed(fSpeed);
         isOn = true;
      break;  

      default:
      car.setSpeed(0);

    }
  }
}

// Method to find a sufficiently large parking spot (50cm).
void findSpot(){
  const int ENOUGH_SPACE = 45;                            //Value for minimum spot length that is enough to park in
  int spotStartLeft,spotStartRight,spotEndRight,spotEndLeft,rightDistance,totalLengthLeft, totalLengthRight;

  while(car.getSpeed()!= 0) {

     rightDistance = sideSonar.getMedianDistance();
    if(rightDistance == 0 || rightDistance > 30){         // Start measuring distance of gap, once a gap is found.
       encoderLeft.begin();
       encoderRight.begin();

       Serial3.println(" WOW let me check this Spot!");

       while(rightDistance == 0 || rightDistance > 30) {  // Keep measuring gap.
           rightDistance = sideSonar.getMedianDistance();
          Serial3.println(" checking spot! ");
      }

      totalLengthLeft = encoderLeft.getDistance();
      totalLengthRight = encoderRight.getDistance();


       Serial3.println(" Spot length on right odometer is :");
       Serial3.println(totalLengthRight);
       Serial3.println(" Spot length on left odometer is :");
       Serial3.println(totalLengthLeft);


       if(totalLengthLeft > ENOUGH_SPACE || totalLengthRight > ENOUGH_SPACE) { //Stop when gap length is bigger than set value for minimum spotsize.
        car.setSpeed(0);
        car.setSpeed(0);
        Serial3.println("Stop waiting to park");
        spotSize = fmin(totalLengthLeft,totalLengthRight);
        break;
      }
    }
    else {
      Serial3.println(" NO spot detected ! ");              //If gap is found but is less than set value for minimum spotsize, it is not recognized as a parking spot.
      car.setSpeed(fSpeed);                                 //The car keeps driving forward.
      car.setAngle(0);

    }
  }


}

void driveBackwardOnSpot(){

  irDistanceInCm = backIR.getDistance();
  backDistanceInCm = backSonar.getDistance();

  myservo.write(0);
  delay(500);

    if((backIR.getMedianDistance() > maxIrbackDistance  || backIR.getMedianDistance()==0) && (backSonar.getMedianDistance() > maxBackDistance || backSonar.getMedianDistance() ==0)){

    car.setSpeed(bSpeed);
    car.setAngle(0);

    while(car.getSpeed()< 0){
//      irDistanceInCm = backIR.getMedianDistance();
//      backDistanceInCm = backSonar.getMedianDistance();
      if((backIR.getMedianDistance() < maxIrbackDistance  && backIR.getMedianDistance()!=0)||(backSonar.getMedianDistance() < maxBackDistance && backSonar.getMedianDistance() !=0)){
        car.setSpeed(0);
        break;
      }

    }

  }
  else {
    car.setSpeed(0);
    Serial3.println("I can NOT Man, I will Crash! ");
  }

  delay(500);
  myservo.write(55);


 }

// method to park the car once i stopped after finding a parking spot
void parkInSpot(){

  int rotateDegree = -35;
  gyro.update();
  Serial3.println(gyro.getAngularDisplacement());
  driveBack();
  delay(2000);
  rotateOnSpot(rotateDegree/2);
  gyro.update();
  Serial3.println(gyro.getAngularDisplacement());
  delay(1000);
  rotateOnSpot(rotateDegree/2);
  delay(2000);
  driveBackwardOnSpot();
  delay(3000);
  rotateOnSpot(-rotateDegree/3);
  delay(1000);
  rotateOnSpot(-rotateDegree/3);
//  delay(1000);
//  rotateOnSpot(-rotateDegree/4);

  delay(2000);
  middlePark();

}

void rotateOnSpot(int targetDegrees) {
  targetDegrees %= 360;
  if (!targetDegrees){
     return;
  }
  if (targetDegrees > 0) {
    car.setMotorSpeed(fSpeed, 0);
  } else {
    car.setMotorSpeed(-fSpeed, fSpeed);
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
 car.setSpeed(0);
}

 // Not sure yet if this works!
// attempt to compensate for the car that is does not drive straight on its own
void straightenCar() {
  gyro.update();
  int turn = gyro.getAngularDisplacement();
  Serial3.println("car deviated " + turn);
  delay(500);
  if(turn > offset) {
    car.setAngle(- turn - (-offset));
  } else if(turn < offset) {
    car.setAngle(-turn -offset);
  }

}

// once the car is parked in the spot this method is called to 
// make the car position itself in the middle between the two objects on the front and back
void middlePark(){
  int frontDistance = frontSonar.getMedianDistance();
  Serial3.println("the front distance is");
  Serial3.println(frontDistance);
  int backDistance = backSonar.getMedianDistance();
  Serial3.println("the back distance is");
  Serial3.println(backDistance);

  int distanceToGo =  frontDistance - backDistance ;
  Serial3.println("the To GO distance is");
  Serial3.println(distanceToGo/2);
  delay(300);
  car.go((distanceToGo/4));
}

void driveBack(){
  sideDistanceInCm = sideSonar.getMedianDistance();
  while(sideDistanceInCm < 30 && sideDistanceInCm != 0){
    car.go(-2);
    sideDistanceInCm = sideSonar.getMedianDistance();
    delay(200);
  }
  delay(300);
  sideDistanceInCm = sideSonar.getMedianDistance();
  if(sideDistanceInCm < 30 && sideDistanceInCm != 0)
  car.go(-3);

}
