void setup() {
  /***  PINMODES  ***/
  
  /***  END OF PINMODES  ***/
  //ears
    minThres[0] = 1200;
    maxThres[0] = 1800;
    minThres[1] = 1200;
    maxThres[1] = 1800;
  //eyes
    minThres[2] = 1200;
    maxThres[2] = 1800;
    minThres[3] = 1200;
    maxThres[3] = 1800;
  //neckP
    minThres[4] = 1200;
    maxThres[4] = 1800;
  //neckYs
    minThres[5] = 1200;
    maxThres[5] = 1800;


  /** Initialize variables **/
  counter = donothing;
  
  /** Define timer functions **/
  /* each function will be called every 20ms */
  timer.setInterval(20, moveEyes);
  timer.setInterval(20, moveYaw);
  timer.setInterval(20, movePitch);
  timer.setInterval(20, moveEars);
  
  Serial.begin(9600);
  Serial.println("  Waiting for Commands");
  
}

void loop() {
  
  /* run timer */
  timer.run();
  
  /* If no gestures, detach servos and motors */
  if (counter == donothing) {
    for (int i=0;i<6;i++)
    {
        servo[i].detach(); 
    }
   
    //digitalWrite(NSTBY, LOW);
    //digitalWrite(ESTBY, LOW);
    /* Instructions are only read when not inside another gesture */
    instruction = (char) Serial.read();
  }
  
  /* Switch to identify which gesture to do */
  switch(instruction) {
    /** Gesture Calls **/
    case 'Z':
      // Continuing Current Gesture
      break;
    
    case 'H':
      Serial.println("Rolling");
      counter = H1;
      break;
      
    case 'T':
      Serial.println("Fully Bored");
      counter = T1;
      break;
    
    case 'W':
      Serial.println("Wheee");
      counter = W1;
      break;
    
    case 'G':
      Serial.println("Giggle");
      counter = G1;
      break;
    
    case 'R':
      Serial.println("Refusing");
      counter = R1;
      break;
    
    case 'S':
      Serial.println("Shivering");
      counter = S1;
      break;
    
    case 'D':
      Serial.println("I am so sad");
      counter = D1;
      break;
      
    case 'L':
      Serial.println("So relaxing");
      counter = L1;
      break;
    
    case 'N':
      Serial.println("Nodding Away");
      counter = N1;
      break;
    
    case 'J':
      Serial.println("You surprised me!");
      counter = J1;
      break;
    
    case 'Y':
      Serial.println("Is it bedtime yet?");
      counter = Y1;
      break;
    
    /** Test Calls **/
    
    case '1':
      Serial.println("Head Upper Left");
      counter = UL;
      break;
    
    case '2': 
      Serial.println("Head Upper Middle");
      counter = UM;
      break;
    
    case '3':
      Serial.println("Head Upper Right");
      counter = UR;
      break;
    
    case '4':
      Serial.println("Head Centre Left");
      counter = CL;
      break;
      
    case '5':
      Serial.println("Head Centre Middle");
      counter = CM;
      break;
    
    case '6':
      Serial.println("Head Centre Right");
      counter = CR;
      break;
    
    case '7':
      Serial.println("Head Down Left");
      counter = DL;
      break;
    
    case '8':
      Serial.println("Head Down Middle");
      counter = DM;
      break;
    
    case '9':
      Serial.println("Head Down Right");
      counter = DR;
      break;
    
    case 'P':
      Serial.println("Ears Straight");
      counter = DEF;
      break;
    
    case 'B':
      Serial.print("Left Ear");
      counter = LB;
      break;
    
    case 'F':
      Serial.println("Left Ear");
      counter = LF;
      break;
    
    case 'M':
      Serial.println("Right Ear");
      counter = RB;
      break;
    
    case 'K':
      Serial.println ("Right Ear");
      counter = RF;
      break;
    
    case 'O':
      Serial.println("Eyes Open");
      counter = EO;
      break;
    
    case 'C':
      Serial.println("Eyes Close");
      counter = EC;
      break; 
  }
  
  if (counter != donothing && counter != prevCounter)  {
    Serial.print("Counter Value: ");
    Serial.println(counter);
    
    /* Identify target action */
    targetAction = actionDictionary[counter];
    
    /* After specified timeout, update counter values */
    timer.setTimeout(targetAction.hold, updateCounter);
    instruction = 'Z';
  }
}
