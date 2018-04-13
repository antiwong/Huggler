#include <Servo.h>
#include <SimpleTimer.h>

/***  LOGIC  ***/

/*  Each gesture is defied by a series of actions.
    Each action is encoded by the following information:
      * speed of yaw (pitch speed not affected, otherwise the 
        motion will be out of balance due to the larger yaw stroke.
      * yaw angle
      * pitch angle
      * left ear pwm
      * right ear pwm
      * boolean eye position (open or close)
      * timeout value
      * reference to the next action sequence
    
    The yaw, pitch and eye will be controlled using a P-loop
    The next action will be called only when timeout is over.  
      * NOT dependent on whether the current desired position 
        is reached > to prevent the system from getting stuck.
      * If current desired position is reached quickly, system
        will hold position until timeout is over.  So can be 
        used for gestures like yawn > to hold the position for 
        a while.
    
    Timer:  every 20ms, 4 functions will be called
      * Function to regulate yaw
      * Function to regulate pitch
      * Function to regulate eyes
      * Function to regulate ears
    
    Timeout will call action update after specified time value. 
    
    NOTE:  The motion for the motors will be continuous, but the
           P-loop (i.e. updating of pwm) is only executed 20ms.
    
    All variables for the 4 functions have to be global.
    Need to install SimpleTimer library for script
    GitHub: https://github.com/schinken/SimpleTimer
    Documentation: http://playground.arduino.cc/Code/SimpleTimer

*/

// Servos for eyes, ears and pitch and yaw (6)


/***  END OF LOGIC  ***/

/***  PIN CONNECTIONS  ***/

/** Neck (Shield 1) **/
const int NSTBY = 10;

/* Yaw (Left - Right) */
//Motor
const int YPIN1 = 11;
const int YPIN2 = 12;
const int YPWM  = 13;
//Position Sensor
const int YPOS  = A0;

/* Pitch (Up - Down) */
//Motor
const int PPIN1 = 9;
const int PPIN2 = 8;
const int PPWM  = 7;
//Position Sensor
const int PPOS  = A1;

/** Eye (Shield 2) **/
//Motor
const int ESTBY = 1;
const int EPIN1 = 2;
const int EPIN2 = 3;
const int EPWM  = 4;
//Position Sensor
const int EPOS  = A2;

/** Ears (Servos) **/
/*Order for servo
Servo EAR1;
Servo EAR2;
Servo EYEL;
Servo EYEL;
Servo NECKY;
Servo NECKP;
*/
Servo servo[6];

#define pinEAR1 45;
#define pinEAR2 44;
#define pinEYE1 43;
#define pinEYE2 42;
#define pinNECKP 41;
#define pinNECKY 40;

//EAR
#define minEAR 1200;
#define maxEAR 1800;

#define minEYE 1200;
#define maxEYE 1800;

#define minNECKP 1200;
#define maxNECKP 1800;

#define minNECKY 1200;
#define maxNECKY 1800;


/***  END OF PIN CONNECTIONS  ***/

/***  POSITION LIMITS  ***/

/*  Mot Limits are the encoder values at each extreme
    Pos Limits are the angular values at each extreme
    Mot values will be used to map to the pos values
    > See moveYaw and movePitch functions for mapping
    Cen (Centre) is the default position
    Max and Min are to allowe for tolerance.
    
    Recommended:  When zeroing/ calibrating before working
    the bear -
      * YawMotCen and PitchMotCen should be between 400 to 600
      * YawMot Left & Right should be +/- 200 to 250 from Cen
      * PitchMot Up & Down should be +/- 150 to 180 from Cen
      * Min and Max for Yaw and Pitch should be +/- 50 from Up/Down/Left/Right
      * Give more tolerance for the eyes

/* Yaw Motors Limits */
int YMotRight = 250; 
int YMotCen = 500; 
int YMotLeft = 750;    
int YPosRight = 0;   
int YPosCen = 60;
int YPosLeft = 120;         
int YawMax = 800;
int YawMin = 200;

/* Pitch Motor Limits */
int PMotUp = 400;
int PMotCen = 500;
int PMotDown = 600;
int PPosUp = 0;
int PPosCen = 25;
int PPosDown = 50;
int PitchMax = 650;
int PitchMin = 350;

/* Eye Motor Limits */
// Sensor to angle mapping not necessary for eye
int EyeOpen = 250;
int EyeClose = 580; 
int EyeMax = 620;
int EyeMin = 180;

/* Precision Intervals */
int neckPrecision = 5;  // in degrees
int eyePrecision =  10;  // in raw encoder values

/***  END OF POSITION LIMITS ***/

/***  GESTURES  ***/

/*  Each gesture ahs a specific sequence of actions labelled in order
    > E.g headroll1, headroll2, headroll3 etc.
    headroll1 will refernce to headroll2, and headroll2 and so on. 
    The last action will refer to default.  default will refer to donothing
    All the actions are stored in the actionDictionary array - so they can 
    be easily referenced to by the enumeration.
*/

/** Enum for gesture reference **/

enum actNo {
  
  /* Always leave DEF (default) as the zeroth element.  The order must here 
     must be identical to that of actionDictionary below, EXCEPT  
     donothing (always the last element) > donothing MUST NOT refer to anything
     in the actionDictionary.
  */
  
  DEF,
  /** GESTURES **/
  /* ROLL_ALL   */ H1, H2, H3, H4, H5,
  /* TURN_BLINK */ T1, T2, T3, T4, T5, T6, T7, T8, T9, T10,
  /* WHEEE      */ W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14,
  /* GIGGLE     */ G1, G2, G3, G4, G5, G6, G7, G8,
  /* REFUSE     */ R1, R2,
  /* SHIVER     */ S1, S2, S3, S4, S5, S6,
  /* DROOP      */ D1,
  /* LOOKABOUT  */ L1, L2, L3, L4, L5, L6,L7,
  /* NOD        */ N1,
  /* JERK       */ J1,
  /* YAWN       */ Y1,
  
  /** TEST MODE**/ 
  /* Head       */ UL, UM, UR, CL, CM, CR, DL, DM, DR,
  /* Ears       */ LB, LF, RB, RF,
  /* Eyes       */ EO, EC,
  
  donothing
};

actNo counter, prevCounter;

typedef struct action_information {
  int yawSpeed;
  int yawAngle;
  int pitchAngle;
  int leftEar;
  int rightEar;
  boolean eyes;
  long hold;
  actNo next;
} action;

/* DEFAULT */
action def = {3, 60, 25, 1650, 1650, true, 1000, donothing};

/* ROLL_ALL */
int rollSpeed = 3;
action headroll1 = {rollSpeed, 105, 10, 2000, 1000, false, 500, H2};  //1
action headroll2 = {rollSpeed, 105, 38, 2000, 1000, false, 500, H3};
action headroll3 = {rollSpeed, 15 , 38, 1650, 1650, true , 500, H4};
action headroll4 = {rollSpeed, 15 , 10, 2000, 1000, true , 500, H5};
action headroll5 = {rollSpeed, 60 , 10, 2000, 1000, true , 250, DEF}; //5

/* TURN_BLINK */
int turnSpeed = 5;
action turn1 = {turnSpeed, 105, 25, 1650, 1650, true , 500 , T2};   //6
action turn2 = {turnSpeed, 105, 25, 1650, 1650, false, 1500, T3};
action turn3 = {turnSpeed, 105, 25, 1650, 1650, true , 1500, T4};
action turn4 = {turnSpeed, 105, 25, 1650, 1650, false, 1500, T5};
action turn5 = {turnSpeed, 105, 25, 1650, 1650, true , 1500, T6};  //10
action turn6 = {turnSpeed, 105, 25, 1650, 1650, false, 1500, T7};
action turn7 = {turnSpeed, 15 , 25, 1650, 1650, false, 500 , T8};
action turn8 = {turnSpeed, 15 , 25, 1650, 1650, true , 1500, T9};
action turn9 = {turnSpeed, 15 , 25, 1650, 1650, false, 1500, T10};
action turn10 ={turnSpeed, 15 , 25, 1650, 1650, true , 1500, DEF}; //15

/* WHEEE */
int wheeeSpeed = 5;
action wheee1 = {wheeeSpeed, 60, 5, 1000, 2000, true, 200, W2};  //16
action wheee2 = {wheeeSpeed, 75,35, 1000, 2000, true, 200, W3};
action wheee3 = {wheeeSpeed, 90, 5, 1000, 2000, true, 200, W4};
action wheee4 = {wheeeSpeed, 75,35, 1000, 2000, true, 200, W5};
action wheee5 = {wheeeSpeed, 60, 5, 1000, 2000, true, 200, W6};  //20
action wheee6 = {wheeeSpeed, 45,35, 1000, 2000, true, 200, W7};
action wheee7 = {wheeeSpeed, 30, 5, 1000, 2000, true, 200, W8};
action wheee8 = {wheeeSpeed, 45,35, 1000, 2000, true, 200, W9};
action wheee9 = {wheeeSpeed, 65,25, 1000, 2000, true, 200, W10};
action wheee10 ={wheeeSpeed, 65,25, 2000, 1000, true, 250, W11}; //25
action wheee11 ={wheeeSpeed, 65,25, 1000, 2000, true, 250, W12};
action wheee12 ={wheeeSpeed, 65,25, 2000, 1000, true, 250, W13};
action wheee13 ={wheeeSpeed, 65,25, 1000, 2000, true, 250, W14}; 
action wheee14 ={wheeeSpeed, 65,25, 2000, 1000, true, 250, DEF};


/* GIGGLE */
int giggleSpeed = 1;
action giggle1 = {giggleSpeed, 60, 5, 1900, 1400, false, 300, G2};  //30
action giggle2 = {giggleSpeed, 60, 5, 1400, 1900, false, 300, G3};
action giggle3 = {giggleSpeed, 60,35, 1900, 1400, false, 300, G4};
action giggle4 = {giggleSpeed, 60,35, 1400, 1900, false, 300, G5};  
action giggle5 = {giggleSpeed, 60, 5, 1900, 1400, true , 300, G6};
action giggle6 = {giggleSpeed, 60, 5, 1400, 1900, true , 300, G7};  //35
action giggle7 = {giggleSpeed, 60,35, 1900, 1400, true , 300, G8};
action giggle8 = {giggleSpeed, 60,35, 1400, 1900, true , 300, DEF};


/* REFUSE */
int refuseSpeed = 3;
action refuse1 = {refuseSpeed, 100, 25, 1400, 1900, true , 250, R2};  //38
action refuse2 = {refuseSpeed, 20 , 25, 1400, 1900, true , 250, DEF};


/* SHIVER */
int shiverSpeed = 100;
action shiver1 = {shiverSpeed, 67, 40, 1400, 1900, false, 250, S2};  //40
action shiver2 = {shiverSpeed, 53, 40, 1400, 1900, false, 250, S3};
action shiver3 = {shiverSpeed, 67, 40, 1400, 1900, false, 250, S4};
action shiver4 = {shiverSpeed, 53, 40, 1400, 1900, false, 250, S5};
action shiver5 = {shiverSpeed, 67, 40, 1400, 1900, false, 250, S6};
action shiver6 = {shiverSpeed, 53, 40, 1400, 1900, false, 250, DEF}; //45

/* DROOP */
int droopSpeed = 1;
action droop1 = {droopSpeed, 60, 40, 2000, 1000, false, 2000, DEF};  //46

/* LOOKABOUT */
int lookSpeed = 5;
action look1 = {lookSpeed, 15, 5, 1900, 1400, true , 500, L2};  //47
action look2 = {lookSpeed, 15, 5, 2000, 1000, true , 500, L3};
action look3 = {lookSpeed, 15, 5, 2000, 1000, true , 500, L4};
action look4 = {lookSpeed, 60,35, 1900, 1400, false, 500, L5};  //50
action look5 = {lookSpeed, 105,5, 1900, 1400, true , 500, L6};
action look6 = {lookSpeed, 105,5, 2000, 1000, false, 500, L7};
action look7 = {lookSpeed, 105,5, 2000, 1000, true , 500, DEF};

/* NOD */
int nodSpeed;
action nod1 = {lookSpeed, 60, 33, 1000, 2000, true, 500, DEF}; //54

/* JERK */
int jerkSpeed = 1;
action jerk1 = {jerkSpeed, 60, 2, 2000, 1300, true, 500, DEF}; //55

/* YAWN */
int yawnSpeed = 1;
action yawn1 = {yawnSpeed, 60, 5, 1000, 2000, false, 3000, DEF}; //56

/* Test */
int testSpeed = 3;

action head_ul = {testSpeed, 5  , 5, 1650, 1650, true, 500, donothing};
action head_um = {testSpeed, 60 , 5, 1650, 1650, true, 500, donothing};
action head_ur = {testSpeed, 115, 5, 1650, 1650, true, 500, donothing};

action head_cl = {testSpeed, 5  , 25, 1650, 1650, true, 500, donothing};
action head_cm = {testSpeed, 60 , 25, 1650, 1650, true, 500, donothing};
action head_cr = {testSpeed, 115, 25, 1650, 1650, true, 500, donothing};

action head_dl = {testSpeed, 5  , 50, 1650, 1650, true, 500, donothing};
action head_dm = {testSpeed, 60 , 50, 1650, 1650, true, 500, donothing};
action head_dr = {testSpeed, 115, 50, 1650, 1650, true, 500, donothing};

action left_back = {testSpeed, 60, 25, 2000, 1650, true, 500, donothing};
action left_forw = {testSpeed, 60, 25, 1650, 1000, true, 500, donothing};

action right_back = {testSpeed, 60, 25, 1650, 1000, true, 500, donothing};
action right_forw = {testSpeed, 60, 25, 1650, 2000, true, 500, donothing};

action eyes_open = {testSpeed, 60, 25, 1650, 1650, true, 500, donothing};
action eyes_close ={testSpeed, 60, 25, 1650, 1650, false, 500, donothing};

/** DICTIONARY **/
action actionDictionary[] = {
  def,
  headroll1, headroll2, headroll3, headroll4, headroll5,
  turn1, turn2, turn3, turn4, turn5, turn6, turn7, turn8, turn9, turn10,
  wheee1, wheee2, wheee3, wheee4, wheee5, wheee6, wheee7, wheee8, wheee9, wheee10, wheee11, wheee12, wheee13, wheee14,
  giggle1, giggle2, giggle3, giggle4, giggle5, giggle6, giggle7, giggle8,
  refuse1, refuse2, 
  shiver1, shiver2, shiver3, shiver4, shiver5, shiver6,
  droop1, 
  look1, look2, look3, look4, look5, look6, look7, 
  nod1,
  jerk1,
  yawn1,
  head_ul, head_um, head_ur, head_cl, head_cm, head_cr, head_dl, head_dm, head_dr, 
  left_back, left_forw, right_back, right_forw, eyes_open, eyes_close
};

/***  END OF GESTURES  ***/

/** Variables **/
char instruction;
action targetAction;
SimpleTimer timer;
