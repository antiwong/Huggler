/***  SUPPORT FUNCTIONS  ***/

void updateCounter(){
  prevCounter = counter;
  counter = targetAction.next;
}

void moveEyes() {
  /* The p-control for the eye is a but more simplified than
     that for the neck > cos of the boolean state (open/ close)
     Important to note the polarity due to this 
  */
  if (counter == donothing) return;
  
  boolean pos = targetAction.eyes;
  
  int curEye = analogRead(EPOS);
  delay(1);
  
  digitalWrite(ESTBY, HIGH);
  int eyeDiff;
  
  if(pos) eyeDiff = EyeOpen - curEye;
  else if (!pos) eyeDiff = EyeClose - curEye;
  
  /* NOTE: Polarity might change depending on encoder values */
  if (abs(eyeDiff) > eyePrecision) {
    if (eyeDiff > 0) {
      digitalWrite(EPIN1, HIGH);
      digitalWrite(EPIN2, LOW);
    }
    else if (eyeDiff < 0) {
      digitalWrite(EPIN1, LOW);
      digitalWrite(EPIN2, HIGH);
    }
    
    /* Don't set the power too high > will fray cable/ gears */
    int eyePower = constrain(abs(eyeDiff), 100, 200);
    analogWrite(EPWM, eyePower);
  }
}

void moveEars() {
  if (counter == donothing) return;
  
  EAR1.attach(E1, minThresh, maxThresh);
  EAR2.attach(E2, minThresh, maxThresh);
  
  EAR1.write(targetAction.leftEar);
  EAR2.write(targetAction.rightEar);
}

void moveAll(){
  if (counter == donothing) return;
  
  for (int i=0;i<6;i++)
    {
        servo[i].attach(pinServo[i], minThresh[i], maxThresh[i]);
    }
    
  servo[0].write(targetAction.leftEar);
  servo[1].write(targetAction.rightEar);

  
}

void moveYaw() {
  /* Again, very important to take note of polarity, dep on encoder values */
  if (counter == donothing) return;
  
  int rawYaw = analogRead(YPOS);
  delay(1);
  int curYaw = map(rawYaw, YMotLeft, YMotRight, YPosLeft, YPosRight);
  
  int yawSign = 1;
  
  if (rawYaw < YawMin || rawYaw > YawMax) {
    stopYaw();
    yawSign = -1;
  }
  
  int yawDiff = targetAction.yawAngle - curYaw;
  if (abs(yawDiff) > neckPrecision) {
    setPowerYaw(targetAction.yawSpeed * yawSign * yawDiff);
  } else stopYaw(); 
}

void movePitch() {
  /* Again, very important to take note of polarity, dep on encoder values */
  if (counter == donothing) return;
  
  int rawPitch = analogRead(PPOS);
  delay(1);
  int curPitch = map(rawPitch, PMotDown, PMotUp, PPosDown, PPosUp);
  
  int pitchSign = 1;
  
  if (rawPitch < PitchMin || rawPitch > PitchMax) {
    stopPitch();
    pitchSign = -1;
  }
  
  int pitchDiff = targetAction.pitchAngle - curPitch;
  if (abs(pitchDiff) > neckPrecision) {
    setPowerPitch(pitchSign * pitchDiff);
  } else stopPitch(); 
}

void setPowerYaw(int power) {
  digitalWrite(NSTBY, HIGH);
  if (power > 0) {
    digitalWrite(YPIN1, HIGH);
    digitalWrite(YPIN2, LOW);
  }
  else if (power < 0) {
    digitalWrite(YPIN1, LOW);
    digitalWrite(YPIN2, HIGH);
  }
  int absPower = (int) (2*abs(power));
  absPower = constrain(power,240, 255);
  analogWrite(YPWM, absPower);
}

void setPowerPitch(int power) {
  digitalWrite(NSTBY, HIGH);
  if(power < 0) {
    digitalWrite(PPIN1, HIGH);
    digitalWrite(PPIN2, LOW);
  }
  else if (power > 0) {
    digitalWrite(PPIN1, LOW);
    digitalWrite(PPIN2, HIGH);
  }
  int absPower = (int) (1.2*abs(power));
  absPower = constrain(absPower, 120, 140);
  analogWrite(PPWM, absPower);
}

void stopYaw(){
  analogWrite(YPWM, 0);
  digitalWrite(YPIN1, LOW);
  digitalWrite(YPIN2, LOW);
}

void stopPitch(){
  analogWrite(PPWM, 0);
  digitalWrite(PPIN1, LOW);
  digitalWrite(PPIN2, LOW);
}

void stopEyes(){
  analogWrite(EPWM, 0);
  digitalWrite(EPIN1, LOW);
  digitalWrite(EPIN2, LOW);
}
