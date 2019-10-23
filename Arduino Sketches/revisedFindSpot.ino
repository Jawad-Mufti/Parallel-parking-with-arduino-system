void findSpot(){
  const int ENOUGH_SPACE = 50;
  int spotStartLeft,spotStartRight,spotEndRight,spotEndLeft,rightDistance,totalLengthLeft, totalLengthRight;
  boolean firstLoop= true;

  car.setSpeed(fSpeed);
  car.setAngle(0);
  encoderLeft.begin(); 
  encoderRight.begin();
  
  while(car.getSpeed()!= 0) { 
    
    rightDistance = sideSonar.getMedianDistance();     
    
    if(rightDistance == 0 || rightDistance > 30){    
       
      if(firstLoop==true){ //maybe remove
        encoderLeft.begin();
        encoderRight.begin();
      }
       Serial3.println(" WOW let me check this Spot!");
       spotStartLeft = encoderLeft.getDistance();
       spotStartRight = encoderRight.getDistance();
       
       while(rightDistance == 0 || rightDistance > 30) {
          rightDistance = sideSonar.getMedianDistance();
          Serial3.println(" Still Checking The SPOT ");    
       }   
    }                
       if((encoderLeft.getDistance()+5) >= ENOUGH_SPACE || ( encoderRight.getDistance()+5) >= ENOUGH_SPACE || (sideSonar.getMedianDistance() <= 20 && encoderLeft.getDistance() > 25)) { //added that last parameter
        car.setSpeed(0);
        Serial3.println("Stop waiting to park");
        Serial3.println(encoderLeft.getDistance());
        Serial3.println("left get distance");
        Serial3.println( encoderLeft.getDistance());
        Serial3.println("spotEndRight");
        Serial3.println(spotEndRight);
       
        delay(300);
        
        while(sideSonar.getMedianDistance() > 25 || sideSonar.getDistance() == 0){
          car.go(5); 
          if(sideSonar.getMedianDistance() < 20){
            break;
          }
        }
        sideDistanceInCm = sideSonar.getDistance();
        delay(1000);
        car.go(-6); //this number used to be 9 but even at 6 i feel like it can still be too much
        break;
      } else {
          Serial3.println(" The spot length is not long Enough ");
          firstLoop = false;
        }
    }
    delay(200);
}
