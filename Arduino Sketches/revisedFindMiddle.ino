void findMiddle(){
  
   while(frontSonar.getMedianDistance() > 20){
    
    car.setSpeed(30);
  
    if(frontSonar.getMedianDistance() <= 20 && frontSonar.getMedianDistance() != 0){  
      Serial3.println("In middle after frontS greater");
      car.setSpeed(0);
      break;
    }
    delay(500);
   } 
   while(backSonar.getMedianDistance() > 20){
      
      Serial3.println("In back greater than 15");
      car.setSpeed(-30);
     
      if(backSonar.getMedianDistance() <= 20 && backSonar.getMedianDistance() != 0){    
        Serial3.println("in backsonar less than 15");
        car.setSpeed(0);
        break;
      }
    }
   car.setSpeed(0);
}
