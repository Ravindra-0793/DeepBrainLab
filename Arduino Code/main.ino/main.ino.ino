#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_GFX.h> // Include the GFX library
#include <Arduino.h>

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define DATAPIN   11 // Changed from 6 to 5
#define CLOCKPIN  12

#define SYNC_LED_PIN A0 // Define A0 as the LED pin
#define STROBE_FREQUENCY 60 // Strobe frequency in Hz
#define STROBE_DURATION 1000 // Duration of strobe in milliseconds
#define SYNC_LED_BRIGHTNESS 250 // Brightness level (0 to 255)



# define strobeLEDContinuousPIN A1
# define strobeLEDContinuousFrequency 2

Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
    MATRIX_WIDTH, MATRIX_HEIGHT, DATAPIN, CLOCKPIN,
    DS_MATRIX_TOP + DS_MATRIX_LEFT +
    DS_MATRIX_COLUMNS + DS_MATRIX_PROGRESSIVE,
    DOTSTAR_BRG);


int tempSequenceIndex[4] = {0, 0, 0, 0}; // Track index progress for each sequence
bool activeSequences[4] = {false, false, false, false}; // Track active sequences based on common initial elements
int finalizedSequence = -1; // To hold the index of the finalized sequence (-1 means not finalized)

unsigned long rewardDelayStart = 0; // Variable to store the start time of the delay
bool rewardDelayActive = false;
unsigned long strobe_previousMillis = 0; // Stores the last time the LED was updated
unsigned long strobe_startTime = 0; // Stores the start time of the strobe
bool isStrobing = false; // Flag to indicate if the strobe is active
bool ledState = false; // Current state of the LED (on or off)

const int totalButtons = 9;
int buttons[totalButtons] = {24, 23, 22, 27, 26, 25, 30, 29, 28};
int Button_ledPins[totalButtons] = {4, 3, 2, 7, 6, 5, 10, 9, 8}; // Assign appropriate pins
int correctSequence[totalButtons];
int enteredSequence[totalButtons];
bool buttonPressed[totalButtons];
bool buttonReleased[totalButtons];
int last_button_pressed=2;
int correctSequenceLength = 0;
const int debounceTime = 1800;//third change debounce time from 10 to 2000
int sequenceIndex = 0;
int n=0;
int k=0;
int p= 0;
int lastPressedButton = -1; // Track the last pressed button (-1 means none)
int currentButtonState ;
int lastButtonState[totalButtons];
int RewardPin = 14; // This is the additional LED pin you already had
int Button_brightness = 100;
int RewardLed_brightness  =255;
unsigned long lastButtonPressTime[totalButtons];
const unsigned long buttonPressTimeout = 30000000;
bool button_before = true;
bool isRewardDisplayed = false;
bool Random_chase = false;
bool reward_dispensing_begin = false;
bool marker_led =false;
bool protocol_looped = false;
bool protocol_check = false;
bool sessionActive = false; // Flag to indicate if the session is active
unsigned long sessionStartTime = 0; // Time when the session was started

bool sequenceConfigured = false;
bool lastButtonInSequence=false;
int successfulSequences = 0;
int incorrectTrial = 0;
bool isRewardReady = false;
bool rewardInProgress = false; // Flag to indicate if the reward process is ongoing
unsigned long rewardStartTime = 0;
unsigned long stobre_LED_start_begin=0;
unsigned long timestamp=0;
unsigned long timestamp1=0;
const unsigned long rewardDispensingDelay = 1200;  // Delay in milliseconds

bool rewardLedStatus = false; // false for Off, true for On
String rewardShape = "rectangle"; // Default shape
bool rewardSound = false; // false for Off, true for On
bool Sequence_Active = false;
bool Light_Guidance = false; // Default to false
bool Optional_Button_LED = false;
int rewardAmount = 1;
int newrewardAmount;
bool seq_retri_flag=false;
bool seq_learn_flag=false;
bool seq_learn_optional_flag= false;
bool experimentBegin = false; // Flag for the beginning of the experiment
bool experimentBeginFlag = false;
int nextLedIndex = -1; // Index for the next LED to light up after reward
bool Loop_count = false;
bool led_back_on= true;
bool AlternateSequence = false;
bool flashPath = false; // Default flash path
bool Random_seq = false;
int flashCount = 3; // Default flash count
int random_seq_count = 0;
bool Switching = true;
bool Optional_sequence = false;
bool newTrialbegin = true;
int trial_number = 0;
int learning_correct_count = 0;
int retrieval_error_count = 1;
int learning_switch_count= 0;
int retrieval_switch_count= 0;

int learning_correct_random_chase_count=0;
int retrieval_error_random_chase_count =1;
int sequence1Length = 0, sequence2Length = 0, sequence3Length = 0, sequence4Length = 0;
int sequence1[totalButtons];
int sequence2[totalButtons];
int sequence3[totalButtons];
int sequence4[totalButtons];
unsigned long timerStart = 0; // Variable to store the start time

void setup() {
  matrix.begin();
  matrix.setBrightness(10); // Adjust brightness as needed
  matrix.fillScreen(0);
  Serial.begin(19200);
  pinMode(RewardPin, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(SYNC_LED_PIN, OUTPUT); // Set the LED pin as an output
  resetRewardLED();
  for (int i = 0; i <= totalButtons; i++) {
    pinMode(buttons[i], INPUT);
    lastButtonPressTime[i] = 0;  // Initialize debounce timer for each button
  }

  while (!Serial);
  for (int i = 0; i <= totalButtons; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    lastButtonState[i] = HIGH;
    pinMode(Button_ledPins[i], OUTPUT);
    analogWrite(Button_ledPins[i], 0); // Initialize all sequence LEDs to OFF using PWM
  }
}

void loop() {

  if (!Light_Guidance && Sequence_Active && !Random_chase && !Optional_sequence) {
    seq_retri_flag=true;
  }

  if (Light_Guidance && Sequence_Active && !Random_chase && Optional_sequence) {
    seq_learn_optional_flag=true;
  }
  
  if (Light_Guidance && Sequence_Active  && !Random_chase && !Optional_sequence) {
    seq_learn_flag= true;
  }

  if (newTrialbegin){
    
    trial_number++;
    timestamp = (micros() - timerStart) / 1000;
    printTimestamped(timestamp,"trial_begin_number: " + String(trial_number));
    //strobeLED(SYNC_LED_PIN, STROBE_FREQUENCY, STROBE_DURATION, SYNC_LED_BRIGHTNESS);
    rewardAmount=newrewardAmount;
    marker_led =true;
    newTrialbegin= false;
  }
  if (protocol_check){
    Protocol_check(n);
  } 
  
  nonBlockingBlink(SYNC_LED_PIN,STROBE_FREQUENCY ,STROBE_DURATION);

  if (Switching ) {
    if (learning_correct_count >= learning_switch_count && Light_Guidance) {
        Light_Guidance = false;
        learning_correct_count = 0;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"switched to sequence retrieval ");
    } else if (retrieval_error_count >= retrieval_switch_count && !Light_Guidance) {
        Light_Guidance = true;
        retrieval_error_count = 0;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"switched to sequence learning ");
        turnOn_button_led();
    }
  }

  if (Random_chase || AlternateSequence) {
    
    if ( retrieval_error_random_chase_count >= retrieval_switch_count && !Random_chase) {
        //Serial.println("alternate_button_selected ");
        newrewardAmount=rewardAmount-2;
        Random_chase =true;
        AlternateSequence =false;
        learning_correct_random_chase_count = 0;
        lastButtonInSequence = false;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"switched to Alternate Button Press ");
    } else if (learning_correct_random_chase_count >= learning_switch_count && Random_chase) {
      //  Serial.println("sequence_selected ");
        lastButtonInSequence = false;
        memcpy(correctSequence, sequence1, sizeof(sequence1));
        correctSequenceLength = sequence1Length;
        Random_chase=false;
        AlternateSequence= true;
        newrewardAmount=rewardAmount+2;
        retrieval_error_random_chase_count = 0;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"switched to sequence learning ");
        //turnOn_button_led();
    }
  }



  if (Light_Guidance && experimentBegin && Sequence_Active) {       //protocol related setting to turn on /off led  on start
    analogWrite(Button_ledPins[correctSequence[0] - 1], Button_brightness);
    experimentBegin = false;
  }

  if (Light_Guidance && experimentBegin && !Sequence_Active) {   //protocol related setting to turn on /off led  on start
    for (int k = 0; k < correctSequenceLength; k++) {
      analogWrite(Button_ledPins[correctSequence[k] - 1], Button_brightness); // Turn on the LED for each button in the correct sequence
    }
    experimentBegin = false;
  }

  if (!Light_Guidance && experimentBegin && !Sequence_Active) {   //protocol related setting to turn on /off led  on start
    for (int k = 0; k < correctSequenceLength; k++) {
      analogWrite(Button_ledPins[correctSequence[k] - 1], 0); // Turn on the LED for each button in the correct sequence
    }
    experimentBegin = false;
  }

  if (!Light_Guidance && experimentBegin && Sequence_Active) {   //protocol related setting to turn on /off led  on start
    for (int k = 0; k < correctSequenceLength; k++) {
      analogWrite(Button_ledPins[correctSequence[k] - 1], 0); // Turn on the LED for each button in the correct sequence
    }
    experimentBegin = false;
  }



  if (isRewardDisplayed && millis() - rewardStartTime > 2000) {
    matrix.fillScreen(0);
    matrix.show();
    isRewardDisplayed = false;
    resetTrial(); // Reset the trial after reward is dispensed
    turnOn_button_led();
  }

  if (Serial.available() > 0) {
    if (timerStart == 0) {
      timerStart = micros(); // Start the timer when the first command is received
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Timer started");
    }
    String command = Serial.readStringUntil('\n'); // Read the command from the serial buffer
    command.trim(); // Remove any whitespace or newline characters from the end of the input

    if (command.startsWith("CONFIG,")) {
      parseConfig(command);
      experimentBegin = true; // Set the experimentBegin flag to true when CONFIG is received
     

    } else if (command.charAt(0) == 'B' || command.charAt(0) == 'b') {
      readBrightness();
    } else if (command == "start_session") {
      sessionActive = true;
      experimentBeginFlag = true;
      k=0;
      timerStart = micros();/// this line need to be checked on 16 august
      sessionStartTime = millis();
      timestamp1 = (micros() - timerStart) / 1000; // Record when the session started
      strobeLEDContinuousMicros(strobeLEDContinuousPIN, strobeLEDContinuousFrequency);
      printTimestamped(timestamp1,"Session started.");
      flashCorrectSequenceButtons();
    } else {
      readCorrectSequence();
    }
    timestamp = (micros() - timerStart) / 1000;
    printTimestamped(timestamp,"New command received.");
    trial_number = 0;
    resetLEDs(); // Reset the LEDs for the new command
    resetTrial();
  }

  if (experimentBeginFlag ){
    strobeLEDContinuousMicros(strobeLEDContinuousPIN, strobeLEDContinuousFrequency);
   
  }

  if (!rewardInProgress && led_back_on) {
    checkButtons();
  }

  unsigned long currentTime = millis();
  bool timeoutOccurred = false;

  for (int i = 0; i < totalButtons; i++) {
    if (currentTime - lastButtonPressTime[i] >= buttonPressTimeout) {
      timeoutOccurred = true;
      break; // Exit loop if any button timed out
    }
  }

  if (timeoutOccurred && sequenceIndex > 0) {
    timestamp = (micros() - timerStart) / 1000;
    printTimestamped(timestamp,"Timeout reached. Resetting sequence.");
    sequenceIndex = 0;
    digitalWrite(RewardPin, LOW);
    resetLEDs(); // Reset the LEDs on timeout
    for (int i = 0; i < totalButtons; i++) {
      lastButtonPressTime[i] = currentTime; // Correct way to update the time for all buttons
    }
    resetTrial();
  }

  


  if (rewardAmount ==0){        // turn off strobe led at the end  off the experimnet
    experimentBeginFlag= false;
    digitalWrite(strobeLEDContinuousPIN, LOW);
  }
 
  if (rewardDelayActive && millis() - rewardDelayStart >= 1000) {
    triggerReward();  // Trigger reward if the delay is complete
    rewardDelayActive = false;  // Reset the delay flag
  }

  reward();
}

void shuffleArray(int *array, int n) {
  for (int i = n - 1; i > 0; i--) {
    int j = random(0, i + 1); // Select a random index from 0 to i
    int temp = array[i]; // Swap array[i] with array[j]
    array[i] = array[j];
    array[j] = temp;
  }
}

void readConfig() {
  String command = Serial.readStringUntil('\n');
  if (command.startsWith("CONFIG,")) {
    parseConfig(command);
  }
  // Add more configuration parsing logic as needed
}

// void parseConfig(String command) {
//   // Remove "CONFIG," prefix and then split by semicolons
//   command.remove(0, 7);
//   int firstSemicolon = command.indexOf(';');
//   String sequencePart = command.substring(0, firstSemicolon);

//   // Process sequence part (assuming it's comma-separated integers)
//   int currIndex = 0, nextComma = 0;
//   correctSequenceLength = 0;
//   while ((nextComma = sequencePart.indexOf(',', currIndex)) != -1) {
//     int number = sequencePart.substring(currIndex, nextComma).toInt();
//     if (number > 0 && number <= totalButtons) {
//       correctSequence[correctSequenceLength++] = number;
//     }
//     currIndex = nextComma + 1;
//   }
//   // Add the last number (after the last comma)
//   int lastNumber = sequencePart.substring(currIndex).toInt();
//   if (lastNumber > 0 && lastNumber <= totalButtons) {
//     correctSequence[correctSequenceLength++] = lastNumber;
//   }

  
//   // Parse other settings
//   int secondSemicolon = command.indexOf(';', firstSemicolon + 1);
//   Button_brightness = command.substring(firstSemicolon + 1, secondSemicolon).toInt();
 

//   int thirdSemicolon = command.indexOf(';', secondSemicolon + 1);
//   RewardLed_brightness = command.substring(secondSemicolon + 1, thirdSemicolon).toInt();

//   int fourthSemicolon = command.indexOf(';', thirdSemicolon + 1);
//   rewardLedStatus = command.substring(thirdSemicolon + 1, fourthSemicolon) == "On";

//   int fifthSemicolon = command.indexOf(';', fourthSemicolon + 1);
//   rewardShape = command.substring(fourthSemicolon + 1, fifthSemicolon);

//   int sixthSemicolon = command.indexOf(';', fifthSemicolon + 1);
//   rewardSound  = command.substring(fifthSemicolon + 1, sixthSemicolon) == "On";

//   int seventhSemicolon = command.indexOf(';', sixthSemicolon + 1);
//   rewardAmount = command.substring(sixthSemicolon + 1, seventhSemicolon).toInt();

//   int eighthSemicolon = command.indexOf(';', seventhSemicolon + 1);
//   learning_switch_count = command.substring(seventhSemicolon + 1, eighthSemicolon).toInt();
 
//   int ninthSemicolon = command.indexOf(';', eighthSemicolon + 1);
//   retrieval_switch_count = command.substring(eighthSemicolon + 1, ninthSemicolon).toInt();
  

//   int tenthSemicolon = command.indexOf(';', ninthSemicolon + 1);
//   Sequence_Active  = command.substring(ninthSemicolon + 1, tenthSemicolon) == "Active";

//   int eleventhSemicolon = command.indexOf(';', tenthSemicolon + 1);
//   Light_Guidance = command.substring(tenthSemicolon + 1, eleventhSemicolon) == "On";

//   int twelfthSemicolon = command.indexOf(';', eleventhSemicolon + 1);
//   flashPath = command.substring(eleventhSemicolon + 1, twelfthSemicolon) == "On";

//   int thirteenthSemicolon = command.indexOf(';', twelfthSemicolon + 1);
//   Random_chase= command.substring(twelfthSemicolon + 1, thirteenthSemicolon) == "On";

//   int fourteenthSemicolon = command.indexOf(';', thirteenthSemicolon + 1);
//   Random_seq = command.substring(thirteenthSemicolon + 1, fourteenthSemicolon) == "On";

//   int fifteenthSemicolon = command.indexOf(';', fourteenthSemicolon + 1);
//   Switching = command.substring(fourteenthSemicolon + 1, fifteenthSemicolon) == "On";

//   int lastSemicolon = command.indexOf(';', fifteenthSemicolon + 1);
//   flashCount = command.substring(fifteenthSemicolon + 1, lastSemicolon).toInt();

//   if (Random_chase) {
//     shuffleArray(correctSequence, correctSequenceLength);
//   }
//   for (int i = 0; i < correctSequenceLength; i++) {
//     Serial.println(correctSequence[i]);
//   }

//   // Debugging output to confirm values
//   timestamp = (micros() - timerStart) / 1000;
//   printTimestamped(timestamp,"Sequence: ");
//   for (int i = 0; i < correctSequenceLength; i++) {
//     Serial.print(correctSequence[i]);
//     Serial.print(" ");
//   }




//   Serial.println();
//   Serial.println("Button Brightness: " + String(Button_brightness));
//   Serial.println("Reward LED Brightness: " + String(RewardLed_brightness));
//   Serial.println("Reward LED Status: " + String(rewardLedStatus ? "On" : "Off"));
//   Serial.println("Reward Shape: " + rewardShape);
//   Serial.println("Reward Sound: " + String(rewardSound ? "On" : "Off"));
//   Serial.println("Reward Amount: " + String(rewardAmount));
//   Serial.println("Learning Correct Count: " + String(learning_switch_count));
//   Serial.println("Retrieval Error Count: " + String(retrieval_switch_count));
//   Serial.println("Sequence Active: " + String(Sequence_Active ? "Active" : "Inactive"));
//   Serial.println("Light Guidance: " + String(Light_Guidance ? "On" : "Off"));
//   Serial.println("Flash Path: " + String(flashPath ? "On" : "Off"));
//   Serial.println("Random Sequence: " + String(Random_seq ? "On" : "Off"));
//   Serial.println("Random Chase: " + String(Random_chase ? "On" : "Off"));
//   Serial.println("Switching: " + String(Switching ? "On" : "Off"));
//   Serial.println("Flash Count: " + String(flashCount));




//   trial_number = 0;
//   incorrectTrial = 0;
// }

void resetSequences() {
  memset(sequence1, 0, sizeof(sequence1));  // Reset sequence1 array to zeros
  memset(sequence2, 0, sizeof(sequence2));  // Reset sequence2 array to zeros
  memset(sequence3, 0, sizeof(sequence3));  // Reset sequence3 array to zeros
  memset(sequence4, 0, sizeof(sequence4));  // Reset sequence4 array to zeros
  memset(correctSequence, 0, sizeof(correctSequence));  // Reset correctSequence array to zeros
  sequence1Length = sequence2Length = sequence3Length = sequence4Length = 0;  // Reset lengths
  correctSequenceLength = 0;
}






void parseConfig(String command) {
  // Remove "CONFIG," prefix and split by semicolons

  resetSequences(); 
  command.remove(0, 7); // Remove the initial "CONFIG," part

  // Arrays to hold sequences
  
  

  // Parse each sequence
  int semicolonIndex = 0;
  for (int seqNum = 1; seqNum <= 4; seqNum++) {
    int nextSemicolonIndex = command.indexOf(';', semicolonIndex);
    String sequencePart = command.substring(semicolonIndex, nextSemicolonIndex);

    int currIndex = 0, nextComma = 0;
    int sequenceIndex = 0;
    while ((nextComma = sequencePart.indexOf(',', currIndex)) != -1) {
      int number = sequencePart.substring(currIndex, nextComma).toInt();
      if (number > 0 && number <= totalButtons) {
        switch (seqNum) {
          case 1:
            sequence1[sequenceIndex++] = number;
            sequence1Length++;
            break;
          case 2:
            sequence2[sequenceIndex++] = number;
            sequence2Length++;
            break;
          case 3:
            sequence3[sequenceIndex++] = number;
            sequence3Length++;
            break;
          case 4:
            sequence4[sequenceIndex++] = number;
            sequence4Length++;
            break;
        }
      }
      currIndex = nextComma + 1;
    }
    // Add the last number (after the last comma)
    int lastNumber = sequencePart.substring(currIndex).toInt();
    if (lastNumber > 0 && lastNumber <= totalButtons) {
      switch (seqNum) {
        case 1:
          sequence1[sequenceIndex++] = lastNumber;
          sequence1Length++;
          break;
        case 2:
          sequence2[sequenceIndex++] = lastNumber;
          sequence2Length++;
          break;
        case 3:
          sequence3[sequenceIndex++] = lastNumber;
          sequence3Length++;
          break;
        case 4:
          sequence4[sequenceIndex++] = lastNumber;
          sequence4Length++;
          break;
      }
    }
    semicolonIndex = nextSemicolonIndex + 1;
  }

  // Set the default sequence to sequence_1
  memcpy(correctSequence, sequence1, sizeof(sequence1));
  correctSequenceLength = sequence1Length;

  // Debugging output to confirm sequences
  Serial.println("Parsed Sequences:");
  Serial.print("Sequence 1: ");
  for (int i = 0; i < sequence1Length; i++) {
    Serial.print(sequence1[i]);
    if (i < sequence1Length - 1) Serial.print(", ");
  }
  Serial.println();

  Serial.print("Sequence 2: ");
  for (int i = 0; i < sequence2Length; i++) {
    Serial.print(sequence2[i]);
    if (i < sequence2Length - 1) Serial.print(", ");
  }
  Serial.println();

  Serial.print("Sequence 3: ");
  for (int i = 0; i < sequence3Length; i++) {
    Serial.print(sequence3[i]);
    if (i < sequence3Length - 1) Serial.print(", ");
  }
  Serial.println();

  Serial.print("Sequence 4: ");
  for (int i = 0; i < sequence4Length; i++) {
    Serial.print(sequence4[i]);
    if (i < sequence4Length - 1) Serial.print(", ");
  }
  Serial.println();

  // Parse remaining settings
  Button_brightness = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)).toInt();
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  RewardLed_brightness = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)).toInt();
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  rewardLedStatus = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  rewardShape = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex));
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  rewardSound = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  rewardAmount = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)).toInt();
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  learning_switch_count = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)).toInt();
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  retrieval_switch_count = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)).toInt();
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  Sequence_Active = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "Active";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  Light_Guidance = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  flashPath = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  Random_chase = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  Random_seq = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  Switching = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1; 

  Optional_sequence = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  Optional_Button_LED = command.substring(semicolonIndex, command.indexOf(';', semicolonIndex)) == "On";
  semicolonIndex = command.indexOf(';', semicolonIndex) + 1;

  flashCount = command.substring(semicolonIndex, command.length()).toInt();



 // Debugging output to confirm values
  timestamp = (micros() - timerStart) / 1000;
  printTimestamped(timestamp,"Sequence: ");
  for (int i = 0; i < correctSequenceLength; i++) {
    Serial.print(correctSequence[i]);
    Serial.print(" ");
  }


  // If Random_chase is active, shuffle the correct sequence
  if (Random_chase) {
    //shuffleArray(correctSequence, correctSequenceLength);
  }
  for (int i = 0; i < correctSequenceLength; i++) {
    Serial.println(correctSequence[i]);
  }

  // Debugging output to confirm values
  timestamp = (micros() - timerStart) / 1000;
  printTimestamped(timestamp,"Sequence: ");
  for (int i = 0; i < correctSequenceLength; i++) {
    Serial.print(correctSequence[i]);
    Serial.print(" ");
  }

  // Print the configuration values for verification
  Serial.println("Configuration Settings:");
  Serial.println("Button Brightness: " + String(Button_brightness));
  Serial.println("Reward LED Brightness: " + String(RewardLed_brightness));
  Serial.println("Reward LED Status: " + String(rewardLedStatus ? "On" : "Off"));
  Serial.println("Reward Shape: " + rewardShape);
  Serial.println("Reward Sound: " + String(rewardSound ? "On" : "Off"));
  Serial.println("Reward Amount: " + String(rewardAmount));
  Serial.println("Learning Switch Count: " + String(learning_switch_count));
  Serial.println("Retrieval Switch Count: " + String(retrieval_switch_count));
  Serial.println("Sequence Active: " + String(Sequence_Active ? "Active" : "Inactive"));
  Serial.println("Light Guidance: " + String(Light_Guidance ? "On" : "Off"));
  Serial.println("Flash Path: " + String(flashPath ? "On" : "Off"));
  Serial.println("Random Chase: " + String(Random_chase ? "On" : "Off"));
  Serial.println("Random Sequence: " + String(Random_seq ? "On" : "Off"));
  Serial.println("Switching: " + String(Switching ? "On" : "Off"));
  Serial.println("Optional_sequence: " + String(Optional_sequence ? "On" : "Off"));
  Serial.println("Optional_Button_LED: " + String(Optional_Button_LED ? "On" : "Off"));
  Serial.println("Flash Count: " + String(flashCount));

  // Reset trial variables
  newrewardAmount=rewardAmount;
  trial_number = 0;
  incorrectTrial = 0;
  sequenceConfigured = true;
}


void readBrightness() {
  if (Serial.available()) {
    Serial.read(); // Read the 'B' character
    int newBrightness = Serial.parseInt();
    if (newBrightness >= 0 && newBrightness <= 255) {
      Button_brightness = newBrightness;
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Brightness set to " + String(Button_brightness));
    }
  }
}

void readCorrectSequence() {
  // Check if a sequence is already configured
  if (sequenceConfigured) {
    Serial.println("Sequence already configured by parseConfig. Skipping readCorrectSequence.");
    return;  // Skip reading from serial
  }

  correctSequenceLength = 0;

  while (Serial.available()) {
    int number = Serial.parseInt();
    if (number == -1) break;  // End of sequence (if -1 is used as the terminator)
    if (number > 0 && number <= totalButtons) {
      correctSequence[correctSequenceLength++] = number;
    }
    if (correctSequenceLength >= totalButtons) break;
  }

  printCorrectSequence();
}







void checkButtons() {
  unsigned long currentTime = millis(); // Get the current time
  
  for (int i = 0; i < totalButtons; i++) {
    currentButtonState = digitalRead(buttons[i]);
    Serial.print(" ");
    Serial.print(" ");
    
    // Check if the button state has changed
    if (currentButtonState == LOW && lastButtonState[i] == HIGH) {
      buttonReleased[i] = false;

      // Allow press if debounce time passed or another button was pressed
      if (currentTime - lastButtonPressTime[i] > debounceTime || (lastPressedButton != -1 && lastPressedButton != i)) {
        if (currentButtonState == LOW) { // Assuming active-low buttons
          buttonPressed[i] = true;
          timestamp = (micros() - timerStart) / 1000;
          last_button_pressed = i + 1;
          n = i;
          p = 0;
          protocol_check = true;
          lastButtonPressTime[i] = currentTime; // Update the last press time for button i
          printTimestamped(timestamp, "Button Pressed: " + String(i + 1));

          // Update the last pressed button
          lastPressedButton = i;
        }
      }
    } else if (currentButtonState == HIGH && lastButtonState[i] == LOW) {
      timestamp = (micros() - timerStart) / 1000;
      Loop_count = false;

      // Button was just released
      buttonReleased[i] = true;
      if (p == 0 && protocol_looped) { // Added to solve button bounce
        printTimestamped(timestamp, "Button Released: " + String(i + 1));
        p++;
        protocol_looped = false;
      }
    }
    
    lastButtonState[i] = currentButtonState; // Update the last known state
  }

  // Clear the debounce period for the last pressed button if another button is pressed
  for (int i = 0; i < totalButtons; i++) {
    if (lastPressedButton != -1 && lastPressedButton != i && digitalRead(buttons[i]) == LOW) {
      lastButtonPressTime[lastPressedButton] = 0; // Reset debounce for the previous button
      break;
    }
  }
}


void triggerReward() {
  rewardInProgress = true;
  isRewardReady = true;  // Set the flag to start the reward process
  rewardStartTime = millis();  // Record the start time
}

void triggerRewardWithNextLed(int currentButtonIndex) {
  isRewardReady = true;  // Set the flag to start the reward process
  rewardStartTime = millis();  // Record the start time
  rewardInProgress = true;
  nextLedIndex = currentButtonIndex + 1;  // Save the next LED index to light up after reward
}

void resetLEDs() {
  for (int i = 0; i < totalButtons; i++) {
    analogWrite(Button_ledPins[i], 0);
  }
}



void resetTrial() {
  Button_release_check();
    if (Random_chase) {
        // Shuffle until the first element is different from the last button pressed
        lastButtonInSequence = false;

        // Check if last_button_pressed is part of the correctSequence
        for (int i = 0; i < correctSequenceLength; i++) {
            if (correctSequence[0] == last_button_pressed) {
                lastButtonInSequence = true;
                break;
            }
        }

      // Only proceed with swapping if last_button_pressed is in the sequence
      if (lastButtonInSequence && trial_number >=1 ) {
        // Rotate the sequence to the left by one position in a circular fashion
        int firstElement = correctSequence[0];
        for (int i = 0; i < correctSequenceLength - 1; i++) {
            correctSequence[i] = correctSequence[i + 1];
        }
        correctSequence[correctSequenceLength - 1] = firstElement;

        // Debugging output to confirm the new order
        Serial.print("Rotated sequence: ");
        for (int i = 0; i < correctSequenceLength; i++) {
            Serial.print(correctSequence[i]);
            Serial.print(" ");
        }
        Serial.println();
    }

        // Print the shuffled sequence
        for (int i = 0; i < correctSequenceLength; i++) {
            Serial.println(correctSequence[i]);
        }

        // Debugging output to confirm values
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"Sequence: ");
        for (int i = 0; i < correctSequenceLength; i++) {
            Serial.print(correctSequence[i]);
            Serial.print(" ");
        }
        Serial.println();
    }



  if (Light_Guidance) {
    resetLEDs();
  }

  sequenceIndex = 0;
  for (int i = 0; i < totalButtons; i++) {
    buttonPressed[i] = false;
  }
  timestamp = (micros() - timerStart) / 1000;
  printTimestamped(timestamp,"trial_end_number: " + String(trial_number));
  turnOn_button_led();
  newTrialbegin= true;
}

void printCorrectSequence() {
  timestamp = (micros() - timerStart) / 1000;
  printTimestamped(timestamp,"Correct Sequence: ");
  for (int i = 0; i < correctSequenceLength; i++) {
    Serial.print(correctSequence[i]);
    if (i < correctSequenceLength - 1) {
      Serial.print(" -> ");
    }
  }
  Serial.println();
}

void turnOn_button_led() {
  experimentBegin = true;

  if (Light_Guidance && experimentBegin && Sequence_Active) {       //protocol related setting to turn on /off led  on start
    analogWrite(Button_ledPins[correctSequence[0] - 1], Button_brightness);
    experimentBegin = false;
  }

  if (Light_Guidance && experimentBegin && !Sequence_Active) {   //protocol related setting to turn on /off led  on start
    for (int k = 0; k < correctSequenceLength; k++) {
      analogWrite(Button_ledPins[correctSequence[k] - 1], Button_brightness); // Turn on the LED for each button in the correct sequence
    }
    experimentBegin = false;
  }

  if (!Light_Guidance && experimentBegin && !Sequence_Active) {   //protocol related setting to turn on /off led  on start
    for (int k = 0; k < correctSequenceLength; k++) {
      analogWrite(Button_ledPins[correctSequence[k] - 1], 0); // Turn on the LED for each button in the correct sequence
    }
    experimentBegin = false;
  }

  if (!Light_Guidance && experimentBegin && Sequence_Active) {   //protocol related setting to turn on /off led  on start
    for (int k = 0; k < correctSequenceLength; k++) {
      analogWrite(Button_ledPins[correctSequence[k] - 1], 0); // Turn on the LED for each button in the correct sequence
    }
    experimentBegin = false;
  }

  led_back_on= true;

}




void reward() {
  Button_release_check();
  
  static unsigned long lastToggleTime = 0;    // To keep track of the last time the LED was toggled
  static int currentRewardCount = 0;          // To count the number of rewards dispensed
  static bool soundPlayed = false;            // Flag to ensure sound is played only once
  static int rewardState = 0;                 // State variable for reward dispensing
  static int currentRewardIndex = 0;          // Track the current reward count
  
  if (isRewardReady) {
    led_back_on = false;
    timestamp = (micros() - timerStart) / 1000;

    // Play the reward sound only once when reward starts
    if (!soundPlayed) {
      printTimestamped(timestamp, "reward_sound");
      soundPlayed = true; // Set the flag to true to prevent playing the sound again
      timestamp = (micros() - timerStart) / 1000;
      // Initial print when starting the reward
      printTimestamped(timestamp, "Reward_dispensing: " + String(rewardAmount));
    }

    // Non-blocking reward dispensing logic using states
    if ((millis() - rewardStartTime >= rewardDispensingDelay)|| reward_dispensing_begin) {
      reward_dispensing_begin =true;
      Button_release_check();
      displayReward();
      resetLEDs();
      
      // State machine for reward dispensing
      switch (rewardState) {
        case 0:
          // Turn on the reward pin
          digitalWrite(RewardPin, HIGH);
          lastToggleTime = millis();
          rewardState = 1; // Move to the next state
          break;

        case 1:
          // Keep the pin HIGH for 40 ms
          if (millis() - lastToggleTime >= 40) {
            digitalWrite(RewardPin, LOW);  // Turn off the reward pin
            lastToggleTime = millis();
            rewardState = 2;  // Move to the next state
          }
          break;

        case 2:
          // Keep the pin LOW for 50 ms (waiting time)
          if (millis() - lastToggleTime >= 300) {
            currentRewardCount++;  // Increment the count of rewards dispensed
            currentRewardIndex++;  // Move to the next reward

            // Check if we've reached the desired reward amount
            if (currentRewardCount >= rewardAmount) {
              isRewardReady = false;
              rewardInProgress = false;
              currentRewardCount = 0;
              currentRewardIndex = 0;
              soundPlayed = false; // Reset the flag for the next reward
              rewardState = 0;     // Reset state
              reward_dispensing_begin = false;
            } else {
              rewardState = 0;  // Move back to the first state to start the next cycle
            }
          }
          break;
      }
    }
  }
}








void displayReward() {
  if(rewardLedStatus){
    matrix.fillScreen(0); // Clear the matrix
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        matrix.drawPixel(x, y, matrix.Color(RewardLed_brightness, 0, 0)); // Set each pixel to green
      }
    }
   
  }
  matrix.show(); // Update the display
  rewardStartTime = millis();
  timestamp = (micros() - timerStart) / 1000;
  printTimestamped(timestamp,"Reward LED On ");
  isRewardDisplayed = true;
}

void printSequence() {
  timestamp = (micros() - timerStart) / 1000;
  printTimestamped(timestamp,"Entered Sequence: ");
  for (int i = 0; i < sequenceIndex; i++) {
    Serial.print(enteredSequence[i]);
    if (i < sequenceIndex - 1) {
      Serial.print(" -> ");
    }
  }
  Serial.println();
}

void Protocol_check(int i) {

  protocol_check= false;

  //////////////////////////Sequence_Learning_protocol/////////////
  if (Light_Guidance && Sequence_Active && (Loop_count == false||seq_learn_flag) && !Random_chase && !Optional_sequence) {
    
    Loop_count = true;
    buttonReleased[i] = false;
    if (i + 1 == correctSequence[sequenceIndex]) {
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"audio_" + String(i + 1));
      enteredSequence[sequenceIndex] = i + 1;
      if (sequenceIndex > 0) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"Button_Led_Off: " + String(i + 1));
      }
      sequenceIndex++;
      if (sequenceIndex < correctSequenceLength) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
        analogWrite(Button_ledPins[correctSequence[sequenceIndex] - 1], Button_brightness); // Turn on the next LED
      }
      if (sequenceIndex == correctSequenceLength) {
        retrieval_error_random_chase_count=0;
        learning_correct_count++;
        Serial.println(learning_correct_count);
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0);
        //triggerReward();
        rewardDelayStart = millis();
        rewardDelayActive = true;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"Sequence complete!");
        successfulSequences++;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"SL_Successful_trial_count : " + String(successfulSequences));
        rewardInProgress = true; // Indicate that the reward process is starting
      }
    } else {
      retrieval_error_random_chase_count++;
      learning_correct_count=0;
      incorrectTrial++;
      Serial.println(learning_correct_count);
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"caution_sound");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Incorrect button. Resetting sequence.");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"SL_Incorrect_trial_count: " + String(incorrectTrial));
      digitalWrite(RewardPin, LOW);
      resetTrial(); // Reset the trial on incorrect button press
    }
  }



  
    //////////////////////////Sequence_Learning_protocol_multiple option/////////////
   if (Light_Guidance && Sequence_Active && (Loop_count == false||seq_learn_optional_flag) && !Random_chase && Optional_sequence) {
        Loop_count = true;
        buttonReleased[i] = false;

        // Check if the current button press matches any of the sequences
        bool correctPress = false;
        if (i + 1 == sequence1[sequenceIndex]) {
            correctPress = true;
            copySequence(correctSequence, sequence1, sequence1Length);
            correctSequenceLength = sequence1Length;
        } else if (i + 1 == sequence2[sequenceIndex]) {
            correctPress = true;
            copySequence(correctSequence, sequence2, sequence2Length);
            correctSequenceLength = sequence2Length;
        } else if (i + 1 == sequence3[sequenceIndex]) {
            correctPress = true;
            copySequence(correctSequence, sequence3, sequence3Length);
            correctSequenceLength = sequence3Length;
        } else if (i + 1 == sequence4[sequenceIndex]) {
            correctPress = true;
            copySequence(correctSequence, sequence4, sequence4Length);
            correctSequenceLength = sequence4Length;
        }

        if (correctPress) {
            timestamp = (micros() - timerStart) / 1000;
            printTimestamped(timestamp, "audio_" + String(i + 1));
            enteredSequence[sequenceIndex] = i + 1;

            // Turn off the LED for the pressed button immediately
            analogWrite(Button_ledPins[i], 0); // Turn off the LED for the current pressed button
            timestamp = (micros() - timerStart) / 1000;
            printTimestamped(timestamp, "Button_Led_Off (Pressed): " + String(i + 1));

            // Turn off the previous LED if it's not the first button in the sequence
            if (sequenceIndex > 0) {
                analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
                timestamp = (micros() - timerStart) / 1000;
                printTimestamped(timestamp, "Button_Led_Off: " + String(correctSequence[sequenceIndex - 1]));
            }

            // Move to the next element in the sequence
            sequenceIndex++;

            // Handle lighting up optional buttons
            int optionSet[4];  // Array to store optional button options
            int optionCount = 0; // Counter to track number of options

            // Check if the next element of each sequence matches the current index
            if (sequenceIndex < sequence1Length) optionSet[optionCount++] = sequence1[sequenceIndex];
            if (sequenceIndex < sequence2Length) optionSet[optionCount++] = sequence2[sequenceIndex];
            if (sequenceIndex < sequence3Length) optionSet[optionCount++] = sequence3[sequenceIndex];
            if (sequenceIndex < sequence4Length) optionSet[optionCount++] = sequence4[sequenceIndex];

            // Remove duplicates in optionSet
            for (int j = 0; j < optionCount; j++) {
                for (int k = j + 1; k < optionCount; k++) {
                    if (optionSet[j] == optionSet[k]) {
                        // Shift the array left to remove the duplicate
                        for (int m = k; m < optionCount - 1; m++) {
                            optionSet[m] = optionSet[m + 1];
                        }
                        optionCount--; // Decrease the option count
                        k--; // Check the new element at index k
                    }
                }
            }
            Serial.println(optionCount);
            // Turn on all LEDs for the options available at this step
            
            lightUpOptionalButtons(optionSet, optionCount, Button_brightness);
           

            // If the sequence is completed, handle successful sequence completion
            if (sequenceIndex == correctSequenceLength) {
                resetLEDs();
                learning_correct_count++;
                Serial.println(learning_correct_count);
                analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0);
                rewardDelayStart = millis();
                rewardDelayActive = true;
                timestamp = (micros() - timerStart) / 1000;
                printTimestamped(timestamp, "Sequence complete!");
                successfulSequences++;
                timestamp = (micros() - timerStart) / 1000;
                printTimestamped(timestamp, "SL_Successful_trial_count : " + String(successfulSequences));
                rewardInProgress = true; // Indicate that the reward process is starting
            }
        } else {
            // Handle incorrect button press
            learning_correct_count = 0;
            incorrectTrial++;
            Serial.println(learning_correct_count);
            timestamp = (micros() - timerStart) / 1000;
            printTimestamped(timestamp, "caution_sound");
            timestamp = (micros() - timerStart) / 1000;
            printTimestamped(timestamp, "Incorrect button. Resetting sequence.");
            timestamp = (micros() - timerStart) / 1000;
            printTimestamped(timestamp, "SL_Incorrect_trial_count: " + String(incorrectTrial));
            digitalWrite(RewardPin, LOW);
            resetTrial(); // Reset the trial on incorrect button press
        }
    }


  


   //////////////////////////Random_Sequence_Protocol/////////////

  // if (Light_Guidance && Sequence_Active && Loop_count == false && Random_seq && !Random_chase) {
  //   Loop_count = true;
  //   buttonReleased[i] = false;
  //   if (i + 1 == correctSequence[sequenceIndex]) {
  //     printTimestamped("audio_" + String(i + 1));
  //     enteredSequence[sequenceIndex] = i + 1;
  //     if (sequenceIndex > 0) {
  //       analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
  //       printTimestamped("Button_Led_Off: " + String(i + 1));
  //     }
  //     sequenceIndex++;
  //     if (sequenceIndex < correctSequenceLength) {
  //       analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
  //       analogWrite(Button_ledPins[correctSequence[sequenceIndex] - 1], Button_brightness); // Turn on the next LED
  //     }
  //     if (sequenceIndex == correctSequenceLength) {
  //       analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0);
  //       triggerReward();
  //       printTimestamped("Sequence complete!");
  //       successfulSequences++;
  //       printTimestamped("RS_Successful_trial_count : " + String(successfulSequences));
  //       rewardInProgress = true; // Indicate that the reward process is starting
  //     }
  //   } else {
  //     incorrectTrial++;
  //     printTimestamped("caution_sound");
  //     printTimestamped("Incorrect button. Resetting sequence.");
  //     printTimestamped("RS_Incorrect_trial_count : " + String(incorrectTrial));
  //     digitalWrite(RewardPin, LOW);
  //     resetTrial(); // Reset the trial on incorrect button press
  //   }
  // }

   //////////////////////////Sequence_Retrieval_protocol/////////////

  if (!Light_Guidance && Sequence_Active && (Loop_count == false||seq_retri_flag) && !Random_chase && !Optional_sequence) {
    Loop_count = true;
    buttonReleased[i] = false;
    if (i + 1 == correctSequence[sequenceIndex]) {
      
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"audio_" + String(i + 1));
      enteredSequence[sequenceIndex] = i + 1;
      if (sequenceIndex > 0) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
      }
      sequenceIndex++;
      if (sequenceIndex < correctSequenceLength) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
        analogWrite(Button_ledPins[correctSequence[sequenceIndex] - 1], 0); // Turn on the next LED
      }
      if (sequenceIndex == correctSequenceLength) {
        retrieval_error_count=0;
        Serial.println(retrieval_error_count);
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0);
        //triggerReward();
        rewardDelayStart = millis();
        rewardDelayActive = true;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"Sequence complete!");
        successfulSequences++;
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"SR_Successful_trial_count : " + String(successfulSequences));
        rewardInProgress = true; // Indicate that the reward process is starting
      }
    } else {
      retrieval_error_count++;
      incorrectTrial++;
      Serial.println(retrieval_error_count);
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"caution_sound");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Incorrect button. Resetting sequence.");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"SR_Incorrect_trial_count: " + String(incorrectTrial));
      digitalWrite(RewardPin, LOW);
      resetTrial(); // Reset the trial on incorrect button press
    }
  }

//  //////////////////////////Random_Chaseing_Protocol/////////////

//   if (Light_Guidance && Sequence_Active && Loop_count == false &&  Random_chase && !Optional_sequence) {
//     Loop_count = true;
//     buttonReleased[i] = false;
//     if (i + 1 == correctSequence[sequenceIndex]) {
//       timestamp = (micros() - timerStart) / 1000;
//       printTimestamped(timestamp,"audio_" + String(i + 1));
//       triggerReward();
//       timestamp = (micros() - timerStart) / 1000;
//       printTimestamped(timestamp,"Sequence complete!");
//       successfulSequences++;
//       timestamp = (micros() - timerStart) / 1000;
//       printTimestamped(timestamp,"RC_Successful_trial_count : " + String(successfulSequences));
//       rewardInProgress = true;
//       enteredSequence[sequenceIndex] = i + 1;
//       if (sequenceIndex > 0) {
//         analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
//         timestamp = (micros() - timerStart) / 1000;
//         printTimestamped(timestamp,"Button_Led_Off: " + String(i + 1));
//       }
//       sequenceIndex++;
//       if (sequenceIndex < correctSequenceLength) {
//         analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
//       }
//       if (sequenceIndex == correctSequenceLength) {
//         analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0);
//         //triggerReward();
//         rewardDelayStart = millis();
//         rewardDelayActive = true;
//         timestamp = (micros() - timerStart) / 1000;
//         printTimestamped(timestamp,"Sequence complete!");
//         successfulSequences++;
//         printTimestamped(timestamp,"RC_Successful_trial_count : " + String(successfulSequences));
//         rewardInProgress = true; // Indicate that the reward process is starting
//       }
//     } else {
//       incorrectTrial++;
//       timestamp = (micros() - timerStart) / 1000;
//       printTimestamped(timestamp,"caution_sound");
//       timestamp = (micros() - timerStart) / 1000;
//       printTimestamped(timestamp,"Incorrect button. Resetting sequence.");
//       timestamp = (micros() - timerStart) / 1000;
//       printTimestamped(timestamp,"RC_Incorrect_trial_count : " + String(incorrectTrial));
//       digitalWrite(RewardPin, LOW);
//       resetTrial(); // Reset the trial on incorrect button press
//     }
//   }



   //////////////////////////Alternate_button_reward and sequence_Protocol/////////////

  if (Light_Guidance && Sequence_Active && Loop_count == false &&  Random_chase && !Optional_sequence) {
    Loop_count = true;
    buttonReleased[i] = false;
    if (i + 1 == correctSequence[sequenceIndex]) {
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"audio_" + String(i + 1));
      //triggerReward();
      rewardDelayStart = millis();
      rewardDelayActive = true;
      learning_correct_random_chase_count++;
      Serial.println("learning_correct_random_chase_count");
      Serial.println(learning_correct_random_chase_count);
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Sequence complete!");
      successfulSequences++;
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"RC_Successful_trial_count : " + String(successfulSequences));
      rewardInProgress = true;
      enteredSequence[sequenceIndex] = i + 1;
      if (sequenceIndex > 0) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"Button_Led_Off: " + String(i + 1));
      }
      sequenceIndex++;
      if (sequenceIndex < correctSequenceLength) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0); // Turn off the previous LED
      }
      if (sequenceIndex == correctSequenceLength) {
        analogWrite(Button_ledPins[correctSequence[sequenceIndex - 1] - 1], 0);
        //triggerReward();
        
        rewardDelayStart = millis();
        rewardDelayActive = true;
        
        timestamp = (micros() - timerStart) / 1000;
        printTimestamped(timestamp,"Sequence complete!");
        successfulSequences++;
        printTimestamped(timestamp,"RC_Successful_trial_count : " + String(successfulSequences));
        rewardInProgress = true; // Indicate that the reward process is starting
      }
    } else {
      learning_correct_random_chase_count=0;
      incorrectTrial++;
      Serial.println("learning_correct_random_chase_count");
      Serial.println(learning_correct_random_chase_count);
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"caution_sound");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Incorrect button. Resetting sequence.");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"RC_Incorrect_trial_count : " + String(incorrectTrial));
      digitalWrite(RewardPin, LOW);
      resetTrial(); // Reset the trial on incorrect button press
    }
  }

   //////////////////////////Early_Training_Protocol/////////////

  if (Light_Guidance && !Sequence_Active && Loop_count == false && !Random_chase && !Optional_sequence) {
    buttonReleased[i] = false;
    bool isCorrectButton = false;

    // Check if the pressed button is in the correct sequence
    for (int j = 0; j < correctSequenceLength; j++) {
      if (i + 1 == correctSequence[j]) {
        isCorrectButton = true;
        break;
      }
    }

    if (isCorrectButton) {
      analogWrite(Button_ledPins[i], 0);
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"audio_" + String(i + 1));
      enteredSequence[sequenceIndex] = i + 1;
      sequenceIndex++;
      rewardDelayStart = millis();
      rewardDelayActive = true;
      // Give reward if the button is correct
      
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Correct button pressed!");
      successfulSequences++;
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"ET_Successful_trial_count : " + String(successfulSequences));
      rewardInProgress = true; // Indicate that the reward process is starting

    } else {
      incorrectTrial++;
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"caution_sound");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Incorrect button. Resetting sequence.");
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"ET_Incorrect_trial_count: " + String(incorrectTrial));
      digitalWrite(RewardPin, LOW);

      // Reset Loop_count and turn off all LEDs for the next sequence
      Loop_count = false;
      for (int k = 0; k < correctSequenceLength; k++) {
        analogWrite(Button_ledPins[correctSequence[k] - 1], 0); // Turn off the LED
      }
      resetTrial(); // Reset the trial on incorrect button press
    }
  }

  // if (!Light_Guidance && !Sequence_Active && Loop_count == false) {
  //   buttonReleased[i] = false;
  //   bool isCorrectButton = false;

  //   // Check if the pressed button is in the correct sequence
  //   for (int j = 0; j < correctSequenceLength; j++) {
  //     if (i + 1 == correctSequence[j]) {
  //       isCorrectButton = true;
  //       break;
  //     }
  //   }

  //   if (isCorrectButton) {
  //     timestamp = (micros() - timerStart) / 1000;
  //     printTimestamped(timestamp,"audio_" + String(i + 1));
  //     enteredSequence[sequenceIndex] = i + 1;
  //     sequenceIndex++;

  //     rewardDelayStart = millis();
  //     rewardDelayActive = true;
  //     timestamp = (micros() - timerStart) / 1000;
  //     printTimestamped(timestamp,"Correct button pressed!");
  //     successfulSequences++;
  //     timestamp = (micros() - timerStart) / 1000;
  //     printTimestamped(timestamp,"Successful_trial_count :" + String(successfulSequences));
  //     rewardInProgress = true; // Indicate that the reward process is starting

  //     // Turn off the LED of the correct button
  //     analogWrite(Button_ledPins[i], 0);
  //   } else {
  //     incorrectTrial++;
  //     timestamp = (micros() - timerStart) / 1000;
  //     printTimestamped(timestamp,"caution_sound");
  //     timestamp = (micros() - timerStart) / 1000;
  //     printTimestamped(timestamp,"Incorrect button. Resetting sequence.");
  //     timestamp = (micros() - timerStart) / 1000;
  //     printTimestamped(timestamp,"Incorrect_trial_count: " + String(incorrectTrial));
  //     digitalWrite(RewardPin, LOW);
  //     resetTrial(); // Reset the trial on incorrect button press

  //     // Reset Loop_count and turn off all LEDs for the next sequence
  //     Loop_count = false;
  //     for (int k = 0; k < correctSequenceLength; k++) {
  //       analogWrite(Button_ledPins[correctSequence[k] - 1], 0); // Turn off the LED
  //     }
  //   }
  // }
  protocol_looped= true;
}
void flashCorrectSequenceButtons() {
  static int flashStep = 0;               // To track the current step in the flashing process
  static int currentFlashCount = 0;       // To count how many times the sequence has been flashed
  static unsigned long lastFlashTime = 0; // To keep track of the last time the LEDs were toggled

  if (flashPath && flashCount > 0) {
    if (flashStep == 0) {
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Flash Path On ");
    }

    // Handle the flashing sequence without blocking
    if (flashStep == 0 && millis() - lastFlashTime >= 500) {
      // Turn on all correct sequence LEDs
      for (int j = 0; j < correctSequenceLength; j++) {
        analogWrite(Button_ledPins[correctSequence[j] - 1], Button_brightness);
      }
      lastFlashTime = millis();
      flashStep = 1; // Move to the next step (LEDs on)
    } 
    else if (flashStep == 1 && millis() - lastFlashTime >= 500) {
      // Turn off all correct sequence LEDs
      for (int j = 0; j < correctSequenceLength; j++) {
        analogWrite(Button_ledPins[correctSequence[j] - 1], 0);
      }
      lastFlashTime = millis();
      flashStep = 0; // Move to the next step (LEDs off)
      currentFlashCount++; // Increase the flash count

      // Check if the sequence has been flashed the required number of times
      if (currentFlashCount >= flashCount) {
        flashStep = 0;
        currentFlashCount = 0;
        flashPath = false; // End the flashing sequence
      }
    }
  }
}


// Define optionSetContains to check if an element is already in the option set
bool optionSetContains(int optionSet[], int optionCount, int element) {
    for (int i = 0; i < optionCount; i++) {
        if (optionSet[i] == element) {
            return true;
        }
    }
    return false;
}



void copySequence(int dest[], const int src[], int length) {
    for (int i = 0; i < length; i++) {
        dest[i] = src[i];
    }
}



void resetRewardLED() {
  matrix.fillScreen(0); // Clear the matrix (turn off all LEDs)
  matrix.show(); // Update the display
  rewardStartTime = millis();
  isRewardDisplayed = false;
}

void printTimestamped(unsigned long timestamp,String message) {
  
  Serial.print("[");
  Serial.print(timestamp);
  Serial.print(" ms] ");
  Serial.println(message);
}



// void printTimestamped(unsigned long timestamp, String message) {
//   // Sanitize the message to ensure it contains only valid characters
//   for (unsigned int i = 0; i < message.length(); i++) {
//     if (message[i] < 32 || message[i] > 126) {  // Remove non-printable characters
//       message[i] = ' ';
//     }
//   }

//   Serial.print("[");
//   Serial.print(timestamp);
//   Serial.print(" ms] ");
//   Serial.println(message);
// }







void Button_release_check(){
  currentButtonState = digitalRead(buttons[n]);
  if (currentButtonState == HIGH && lastButtonState[n] == LOW && p== 0 && protocol_looped) {//second change here protocol_looped
     
      timestamp = (micros() - timerStart) / 1000;
      printTimestamped(timestamp,"Button released: " + String(n + 1));
      p++;
      protocol_looped= false;
      //lastButtonState[n] = currentButtonState;
    }
    // Update the last known state
}


void printActiveSequences() {
  Serial.println("Possible correct sequences based on current state:");
  for (int seq = 0; seq < 4; seq++) {
    if (activeSequences[seq]) {
      switch (seq) {
        case 0:
          Serial.print("Sequence 1: ");
          for (int j = sequenceIndex; j < sequence1Length; j++) {
            Serial.print(sequence1[j]);
            if (j < sequence1Length - 1) Serial.print(" -> ");
          }
          Serial.println();
          break;
        case 1:
          Serial.print("Sequence 2: ");
          for (int j = sequenceIndex; j < sequence2Length; j++) {
            Serial.print(sequence2[j]);
            if (j < sequence2Length - 1) Serial.print(" -> ");
          }
          Serial.println();
          break;
        case 2:
          Serial.print("Sequence 3: ");
          for (int j = sequenceIndex; j < sequence3Length; j++) {
            Serial.print(sequence3[j]);
            if (j < sequence3Length - 1) Serial.print(" -> ");
          }
          Serial.println();
          break;
        case 3:
          Serial.print("Sequence 4: ");
          for (int j = sequenceIndex; j < sequence4Length; j++) {
            Serial.print(sequence4[j]);
            if (j < sequence4Length - 1) Serial.print(" -> ");
          }
          Serial.println();
          break;
      }
    }
  }
  Serial.println();  // Blank line for readability
}


// Function to handle the strobe effect
void strobeLED(int pin, int frequency, int duration, int brightness) {
  unsigned long currentMillis = millis();

  // Start the strobe if it's not already active
  if (!isStrobing) {
    isStrobing = true;
    strobe_startTime = currentMillis;
  }

  // Calculate the time interval for each strobe cycle (half period)
  unsigned long interval = 1000 / (2 * frequency);

  // Check if it's time to toggle the LED state
  if (currentMillis - strobe_previousMillis >= interval) {
    strobe_previousMillis = currentMillis; // Save the current time
    ledState = !ledState; // Toggle the LED state
    if (ledState) {
      analogWrite(pin, brightness); // Set the LED brightness
    } else {
      analogWrite(pin, 0); // Turn the LED off
    }
  }

  // Check if the strobe duration has ended
  if (currentMillis - strobe_startTime >= duration) {
    analogWrite(pin, 0); // Ensure the LED is turned off
    isStrobing = false; // Reset the strobe flag
  }
}

void nonBlockingBlink(int pin, int frequency, int duration) {
    static unsigned long previousMillis = 0;
    static unsigned long startTime = 0;
    static bool ledState = false;
    unsigned long currentMillis = millis();
    
    if (marker_led) {  // Check if marker_led is true
        if (startTime == 0) {
            startTime = currentMillis;
        }

        if (currentMillis - startTime < duration) {
            unsigned long interval = 1000 / (2 * frequency);

            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                ledState = !ledState; // Toggle the LED state
                digitalWrite(pin, ledState ? HIGH : LOW); // Turn LED on or off
            }
        } else {
            digitalWrite(pin, LOW); // Ensure the LED is off after the duration
            startTime = 0; // Reset for the next call
            marker_led = false; // Set marker_led to false after completing the duration
        }
    } else {
        digitalWrite(pin, LOW); // Ensure the LED is off if marker_led is false
        startTime = 0; // Reset startTime if the blinking was interrupted
    }
}



void strobeLEDContinuousMicros(int pin, int frequency) {
    static unsigned long previousMicros = 0;
    static bool ledState = false;
    unsigned long intervalMicros = 1000000 / (2 * frequency); // Interval for 2 Hz in microseconds (500,000 µs on, 500,000 µs off)
    unsigned long currentMicros = micros();

    if (currentMicros - previousMicros >= intervalMicros) {
        previousMicros = currentMicros;
        ledState = !ledState; // Toggle the LED state
        digitalWrite(pin, ledState ? HIGH : LOW); // Turn LED on or off
        stobre_LED_start_begin= (micros() - timerStart) / 1000;
        if ( k==0){
          Serial.print("[");
          Serial.print(stobre_LED_start_begin);
          Serial.print(" ms] ");
          Serial.println("Continous Strobe_LED_started");
          k++;
        }
    }
}

void lightUpOptionalButtons(const int optionSet[], int optionCount, int brightness) {
    if(!Optional_Button_LED && optionCount==1 ){  
      for (int i = 0; i < optionCount; i++) {
          analogWrite(Button_ledPins[optionSet[i] - 1], brightness);
      }
    } else if(!Optional_Button_LED && optionCount>1 ){  
      for (int i = 0; i < optionCount; i++) {
          analogWrite(Button_ledPins[optionSet[i] - 1], 0);
      }
    }else if(Optional_Button_LED ){  
      for (int i = 0; i < optionCount; i++) {
          analogWrite(Button_ledPins[optionSet[i] - 1], brightness);
      }
    } 
}

