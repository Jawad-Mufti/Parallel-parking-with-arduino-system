#include <Smartcar.h>


Car car;

SR04 frontSonar;
SR04 sideSonar;
SR04 backSonar;
GP2D120 backIR; 
Gyroscope gyro(-5);


const int fSpeed = 30; //70% of the full speed forward
const int bSpeed = -30; //70% of the full speed backward
const int lDegrees = -75; //degrees to turn left
const int rDegrees = 75; //degrees to turn right

Odometer encoderLeft(210), encoderRight(210);
//Servo myservo;

const int frontTrigPin = 6; 
const int frontEchoPin = 7;
const int sideTrigPin = A11;
const int sideEchoPin = A12;
const int backTrigPin = A9;
const int backEchoPin = A10;

const int backIrPin = A0;

const int encoderLeftPin = 2;
const int encoderRightPin = 3;


//const int SERVO_PIN = 50;

int spotSize;
int backDistanceInCm;
int frontDistanceInCm;
int sideDistanceInCm;
int irDistanceInCm;  

int maxBackDistance = 10;
int maxFrontDistance = 10; 
int maxSideDistance = 10;
int maxIrbackDistance = 6; 

int pos = 0; // variable to store the servo position
int const offset = -5;


void setup() {
  Serial3.begin(9600);
  car.begin(); //initialize the car using the encoders and the gyro
  gyro.attach();
  gyro.begin();   
  sideSonar.attach(sideTrigPin, sideEchoPin);
  backSonar.attach(backTrigPin, backEchoPin);
  frontSonar.attach(frontTrigPin,frontEchoPin);
  encoderLeft.attach(encoderLeftPin);
  encoderRight.attach(encoderRightPin);
//  myservo.attach(SERVO_PIN);
  backIR.attach(backIrPin);
  car.begin(encoderLeft,encoderRight);
  car.begin(gyro);
  
}

void loop() {
  handleInput();
}

void handleInput() { //handle serial input if there is any
  if (Serial3.available()) {
    char input = Serial3.read(); //read everything that has been received so far and log down the last entry
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
        findSpot();
        break;
      case 'p': //go back
      parkInSpot();
        break;
      default: //if you receive something that you don't know, just stop
        car.setSpeed(0);
        car.setAngle(0);
    }
  }
  
     if (Serial.available()) {
    String inputRPi = Serial.readStringUntil('\n');
     
    if (inputRPi==("redLight")) {
       car.setSpeed(0);     
    }
    else if (inputRPi==("greenLight")) {
     car.setSpeed(fSpeed) ;  
    }
    else {
      car.setSpeed(0);
    }
  }
    
}

void findSpot(){
  const int ENOUGH_SPACE = 50;
  int spotStartLeft,spotStartRight,spotEndRight,spotEndLeft,rightDistance,totalLengthLeft, totalLengthRight;

  while(car.getSpeed()!= 0) {
    
     rightDistance = sideSonar.getMedianDistance();
     
     
    if(rightDistance == 0 || rightDistance > 30){ 
      
       encoderLeft.begin();
       encoderRight.begin();
       Serial3.println(" WOW let me check this Spot!");
       spotStartLeft = encoderLeft.getDistance();
       spotStartRight = encoderLeft.getDistance();
       

       while(rightDistance == 0 || rightDistance > 30) {
           rightDistance = sideSonar.getMedianDistance();
          Serial3.println(" Still Checking The SPOT ");
          
      }
      
      spotEndLeft = encoderLeft.getDistance();
      spotEndRight = encoderRight.getDistance();
      
       totalLengthLeft =  (spotEndLeft+spotStartLeft);
       totalLengthRight = (spotEndRight+spotStartRight);
       
       Serial3.println(" Spot length on right odometer is :");
       Serial3.println(totalLengthRight);
       Serial3.println(" Spot length on left odometer is :");
       Serial3.println(totalLengthLeft);
       
       
       if(totalLengthLeft > ENOUGH_SPACE || totalLengthRight > ENOUGH_SPACE) {
        car.setSpeed(0);
        Serial3.println("Stop waiting to park");
//        spotSize = totalLengthLeft; should do a comparision between 
//  the left and right distance from the odometer!! 

        sideDistanceInCm = sideSonar.getDistance();
        delay(1000);
        car.go(-9);
        break;
      } else {
          Serial3.println(" The spot length is not long Enough ");
        }
    }
    else {
      Serial3.println(" NO spot detected ! ");
      car.setSpeed(fSpeed);
      car.setAngle(0);
      
    }
  }

 
}

void driveBackward(){
  
  irDistanceInCm = backIR.getDistance();
  backDistanceInCm = backSonar.getDistance();
  
  Serial3.println("Distance of IR sensor is "); 
  
  if((irDistanceInCm > maxIrbackDistance || irDistanceInCm == 0) && (backDistanceInCm > maxBackDistance || backDistanceInCm ==0)){
    
    car.setSpeed(bSpeed);
    car.setAngle(0);

    
    while(car.getSpeed()< 0){
      irDistanceInCm = backIR.getDistance();
      backDistanceInCm = backSonar.getDistance();
      Serial3.println(irDistanceInCm);
      if((irDistanceInCm < maxIrbackDistance  && irDistanceInCm !=0)||(backDistanceInCm < maxBackDistance && backDistanceInCm !=0)){
        car.setSpeed(0); 
        break;
      }
      
    }
    
  }
  else {
    car.setSpeed(0);
    Serial3.println("I can NOT Man, I will Crash! ");
  }
  
 
 }
 
void parkInSpot(){  //need delay 1000 in many places or else it will not turn as it should
//  double value = (double) (spotSize/2)/(sideDistanceInCm + 15); 
//  int rotateDegree = -(90-atan(value)/3.14*180);   We do NOT use this !
int rotateDegree; 
  gyro.update();
  Serial3.println(gyro.getAngularDisplacement());
  
  if(sideDistanceInCm > 13){
     rotateDegree = -35;
     Serial3.println("side distance is ");
     Serial3.println(sideDistanceInCm);
  rotateOnSpot(rotateDegree);
  }
  else{
    rotateDegree = -35;
    Serial3.println("side distance is ");
     Serial3.println(sideDistanceInCm);
    rotateOnSpot(rotateDegree);
  }
  
  gyro.update();
  Serial3.println(gyro.getAngularDisplacement());
  
  delay(1000);
  
 driveBackward();
  
//  sqrt(pow((spotSize/3),2)+pow((sideDistanceInCm),2)); //weird, cuz the size of the spot comes out as very long

  delay(1000);
  
   car.rotate(-(rotateDegree/3));

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
