#include <Smartcar.h>


Car car;

SR04 frontSonar;
SR04 sideSonar;
SR04 backSonar;
Gyroscope gyro(-5);
const int fSpeed = 40; //70% of the full speed forward
const int bSpeed = -30; //70% of the full speed backward
const int lDegrees = -75; //degrees to turn left
const int rDegrees = 75; //degrees to turn right
const int frontTrigPin = 6; 
const int frontEchoPin = 7;
const int sideTrigPin = A11;
const int sideEchoPin = A12;
const int backTrigPin = A9;
const int backEchoPin = A10;

const int backIrPin = A0;

int maxBackDistance = 15;
int maxFrontDistance = 10; 
int maxSideDistance = 5;
int maxIrbackDistance = 6; 
int spotSize;
int backDistanceInCm;
int frontDistanceInCm;
int sideDistanceInCm;
int irDistanceInCm; 
int leftTurn ;
int rightTurn;

void setup() {
  Serial.begin(9600);
  car.begin(); //initialize the car 
   gyro.attach();
  gyro.begin();   
  sideSonar.attach(sideTrigPin, sideEchoPin);
  backSonar.attach(backTrigPin, backEchoPin);
  frontSonar.attach(frontTrigPin,frontEchoPin);
  }


  void loop() {
  handleInput();
}

void handleInput() { //handle serial input if there is any
  if (Serial.available()) {
    char input = Serial.read(); //read everything that has been received so far and log down the last entry
    switch (input) {
      case 'l': //rotate counter-clockwise going forward
        car.setSpeed(fSpeed);
        car.setAngle(lDegrees);
        break;
      case 'r': //turn clock-wise
        car.setSpeed(fSpeed);
        car.setAngle(rDegrees);
        break;
      case 's': //go ahead
        car.setSpeed(fSpeed);
        car.setAngle(0);
        parallelCruise();
        break;
      case 'p': //go back
      car.setSpeed(bSpeed);
        car.setAngle(0);
        break;

        case 't': //for testing the sensor
        testM();
     
        break;
      default: //if you receive something that you don't know, just stop
        car.setSpeed(0);
        car.setAngle(0);
        
    }
  }
}
void testM(){

      Serial.println( (backSonar.getDistance()));  //sensor testing
         delay(1000);
          

Serial.println( sideSonar.getDistance());



  
}



void parallelCruise(){

  while(car.getSpeed()!=0){
  
  sideDistanceInCm = sideSonar.getDistance();
  backDistanceInCm = (backSonar.getDistance()+7); // + 7 because the position of the back sensor is more far away than the side sensoer to the side obsticle
  leftTurn= backDistanceInCm - sideDistanceInCm ;      /// leftTurn is the amount of degrees the car should turn left when the front side sensor is closer to an obsticle than the back side sensor.
  rightTurn= (sideDistanceInCm - backDistanceInCm );       /// rightTurn is the amount of degrees the car should turn right when the back side sensor is closer to an obsticle than the front side sensor.
 // Serial.println("Distance of the back sensor is "+ backDistanceInCm); 
  
if((sideDistanceInCm < maxSideDistance  && sideDistanceInCm !=0)||(backDistanceInCm  < maxBackDistance && backDistanceInCm !=0)){
  delay(1000);  
    //fixAngle();
    car.setSpeed(0);
    delay(1000);
    break;
        Serial.println( (backSonar.getDistance()+7)+"   " );

}
   
   
if (sideDistanceInCm < backDistanceInCm) {
  rotateOnSpot(-leftTurn/2);
   gyro.update();
  Serial.println(gyro.getAngularDisplacement());
  delay(1000);




    
}
  else if (sideDistanceInCm > backDistanceInCm) {   
      rotateOnSpot(rightTurn/2);
       gyro.update();
 Serial.println(gyro.getAngularDisplacement());
 delay(1000);
     
     }

     else {
    car.setSpeed(0);
   
  }

    
  }
  
}





  
//  void parallelDrive(){ 

//if (backSonar.getDistance() > frontSonar.getDistance()) {
//  while(backSonar.getdistance()!= frontSonar.getDistance()){
//    drive_right();
//  }
//}
//if (backsonar.getDistance() < frontSonar.getDistance()) {
//  while(backSonar.getdistance()!= frontSonar.getDistance()){
//    drive_left();
//  }
//
//
//}
//  }

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
