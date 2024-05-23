#include <Stepper.h>

const int stepsPerRevolution = 2048; //POSSIBLY CHANGE THIS was 2048
const int stepsBetweenFloors = 3400; //CHANGE THIS
const int fullSpeed = 14;
const int topButtonPin = 2;
const int floor0ButtonPin = 3;
const int floor1ButtonPin = 4;
const int floor2ButtonPin = 5;
const int floor3ButtonPin = 6; 
int currentPosition = 0;
int calledFloors[4];
int index = 0;
int direction;
int smallStep = 5;

Stepper stepperMotor = Stepper(stepsPerRevolution, 8, 9, 10, 11);

void setup() { 
  //Securitate pentru cand se taie curentul. Liftul e urcat pana cand atinge butonul de sus.A
  //Apoi este coborat pana la parter
  Serial.begin(9600);
  stepperMotor.setSpeed(fullSpeed); //setam la o viteza neutra



  while (digitalRead(topButtonPin) == LOW) {
    stepperMotor.step(-1);
  }
  currentPosition = 3 * stepsBetweenFloors - 120;

  index = 0;
  direction = 0;
  for (int i = 1; i < 4; i++) {
    calledFloors[i] = -1;
  }
  addFloorToQueue(0);
}

bool etajAlreadyIn(int etaj) {

  for(int i = 0; i < 4; i++) {
    if(calledFloors[i] == etaj) {
      return true;
    }
  }

  return false;
}

void addFloorToQueue(int floor) {
  calledFloors[index] = floor;
  index++;

  printQueue();
}

void onFloorButtonPressed(int floor) {
  bool floorAlreadyCalled = etajAlreadyIn(floor);

  if (!floorAlreadyCalled) {
    addFloorToQueue(floor);
  } 
}

void popQueue() {
  index--;
  for (int i = 0; i < 3; i++) {
    calledFloors[i] = calledFloors[i + 1];
  }
  calledFloors[3] = -1;  
}

int peeqQueue() {
  return calledFloors[0];
}

int queueHas(int value) {
  for (int i = 0; i < 4; i++) {
    if (calledFloors[i] == value) {
      return true;
    }
  }

  return false;
}

void queueRemove(int value) {
  for (int i = 0; i < 4; i++) {
    if (calledFloors[i] == value) {
      for (int j = i; j < 3; j++) {
        calledFloors[j] = calledFloors[j + 1];
      }
      calledFloors[3] = -1;
      index--;
    }
  }
}

void printQueue() {
  Serial.print("QUEUE: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(calledFloors[i]);
    Serial.print(" ");
  }
  Serial.print("\n");
}

int getLowestFloorAboveCurrentPosition() {
  int min = 4 * stepsBetweenFloors;
  for (int i =0; i<4; i++) {
    if (calledFloors[i] * stepsBetweenFloors > currentPosition){
      if (calledFloors[i] * stepsBetweenFloors < min) {
        min = calledFloors[i] * stepsBetweenFloors;
      }
    }
  }

  if (min != 4 * stepsBetweenFloors )
    return min;
  else return -1;
}

int getHighestFloorBelowCurrentPosition() {
  int max = -1;
  for (int i = 0; i < 4; i++) {
    if (calledFloors[i] * stepsBetweenFloors < currentPosition) {
      if (calledFloors[i] * stepsBetweenFloors > max){
        max = calledFloors[i] * stepsBetweenFloors;
      }
    }
  }

  return max;
}

int computeSteps(int nextFloor) {
  int delta = currentPosition - nextFloor;
  if (delta < 0)
    delta = -delta;
  
  return min(delta, smallStep);
}

void loop() {

  if (digitalRead(floor0ButtonPin) == HIGH) {
    onFloorButtonPressed(0);
  }

  if (digitalRead(floor1ButtonPin) == HIGH) {
    onFloorButtonPressed(1);
  }

  if (digitalRead(floor2ButtonPin) == HIGH) {
    onFloorButtonPressed(2);
  }

  if (digitalRead(floor3ButtonPin) == HIGH) {
    onFloorButtonPressed(3);
    // printQueue();
  }

  if (currentPosition % stepsBetweenFloors == 0 && queueHas(currentPosition / stepsBetweenFloors)) {
    queueRemove(currentPosition / stepsBetweenFloors);
    delay(3000);
  }

  int stepsRemaining = 0;
  if (direction == 0) {
    int nextFloorAbove = getLowestFloorAboveCurrentPosition();

    if(nextFloorAbove != -1){
      direction = 1;
      stepsRemaining = computeSteps(nextFloorAbove);
    }

    else {
      int nextFloorBelow = getHighestFloorBelowCurrentPosition();

      if (nextFloorBelow != -1) {
        direction = -1;
        stepsRemaining = computeSteps(nextFloorBelow);
      }
    }

  }

  else if (direction == -1) {
    int nextFloorBelow = getHighestFloorBelowCurrentPosition();
    if (nextFloorBelow != -1) {
      stepsRemaining = computeSteps(nextFloorBelow);
    }

    else {
      direction = 0;
    }
  }

  else if (direction == 1) {
    int nextFloorAbove = getLowestFloorAboveCurrentPosition();
    if (nextFloorAbove != -1) {
      stepsRemaining = computeSteps(nextFloorAbove);
    }

    else {
      direction = 0l;
    }
  }

  if (stepsRemaining != 0) {
    stepperMotor.step(-direction * stepsRemaining);
    currentPosition += direction * stepsRemaining;
  }
}