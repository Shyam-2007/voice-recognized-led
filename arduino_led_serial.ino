

const int PIN_RED   = 11;
const int PIN_GREEN = 9;
const int PIN_BLUE  = 10;

const int BLINK_TIMES    = 3;
const int BLINK_ON_MS    = 300;
const int BLINK_OFF_MS   = 200;

//Serial buffer 
String inputBuffer = "";
void setup() {
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);

  // Start with all LEDs off
  allOff();

  Serial.begin(9600);

  // Signal to the program that Arduino is ready. 
  Serial.println("READY");
}

void loop() {
  // Read incoming serial bytes into the buffer
  while (Serial.available()) {
    char c = (char)Serial.read();

    if (c == '\n') {
      // Full command received from voice module— process it
      inputBuffer.trim();
      if (inputBuffer.length() > 0) {
        handleCommand(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }
}

// ─── Command handler ───────────────────────────────────────────────────────────
void handleCommand(String cmd) {

  Serial.print("CMD:");
  Serial.println(cmd);  

  // Control individual LEDs connected to the arduino
  if (cmd == "R1") { digitalWrite(PIN_RED,   HIGH); Serial.println("OK:RED_ON");   }

  else if (cmd == "R0") { digitalWrite(PIN_RED,   LOW);  Serial.println("OK:RED_OFF");  }

  else if (cmd == "G1") { digitalWrite(PIN_GREEN, HIGH); Serial.println("OK:GRN_ON");   }

  else if (cmd == "G0") { digitalWrite(PIN_GREEN, LOW);  Serial.println("OK:GRN_OFF");  }

  else if (cmd == "B1") { digitalWrite(PIN_BLUE,  HIGH); Serial.println("OK:BLU_ON");   }

  else if (cmd == "B0") { digitalWrite(PIN_BLUE,  LOW);  Serial.println("OK:BLU_OFF");  }

  //All LEDs on or Off
  else if (cmd == "A1") { allOn();  Serial.println("OK:ALL_ON");  }
  else if (cmd == "A0") { allOff(); Serial.println("OK:ALL_OFF"); }

  //  Blink modes
  else if (cmd == "BR") { blinkLED(PIN_RED);   Serial.println("OK:BLINK_RED");   }

  else if (cmd == "BG") { blinkLED(PIN_GREEN); Serial.println("OK:BLINK_GRN");   }

  else if (cmd == "BB") { blinkLED(PIN_BLUE);  Serial.println("OK:BLINK_BLU");   }

  else if (cmd == "BA") { blinkAll();           Serial.println("OK:BLINK_ALL");   }

  else if (cmd == "W") { rgbCycle(); Serial.println("OK:WAKE_ANIM"); } // cycles the colors we have until the commnad is spoken
  else {
    Serial.print("ERR:UNKNOWN_CMD:");
    Serial.println(cmd);
  }
}

// LED controllers
void allOn() {
  digitalWrite(PIN_RED,   HIGH);
  digitalWrite(PIN_GREEN, HIGH);
  digitalWrite(PIN_BLUE,  HIGH);
}

void allOff() {
  digitalWrite(PIN_RED,   LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE,  LOW);
}

void blinkLED(int pin) {
  bool wasOn = (digitalRead(pin) == HIGH);
  for (int i = 0; i < BLINK_TIMES; i++) {
    digitalWrite(pin, HIGH);
    delay(BLINK_ON_MS);
    digitalWrite(pin, LOW);
    delay(BLINK_OFF_MS);
  }

  if (wasOn) digitalWrite(pin, HIGH);
}

void rgbCycle() {
    bool redWas   = (digitalRead(PIN_RED)   == HIGH);
    bool greenWas = (digitalRead(PIN_GREEN) == HIGH);
    bool blueWas  = (digitalRead(PIN_BLUE)  == HIGH);
    // Fade red up, then down
    for (int i = 0; i <= 200; i++) {
        analogWrite(PIN_RED, i);
        delay(5);
    }
    for (int i = 200; i >= 0; i--) {
        analogWrite(PIN_RED, i);
        delay(5);
    }

    // Fade green up, then down
    for (int i = 0; i <= 200; i++) {
        analogWrite(PIN_GREEN, i);
        delay(5);
    }
    for (int i = 200; i >= 0; i--) {
        analogWrite(PIN_GREEN, i);
        delay(5);
    }

    // Fade blue up, then down
    for (int i = 0; i <= 200; i++) {
        analogWrite(PIN_BLUE, i);
        delay(5);
    }
    for (int i = 200; i >= 0; i--) {
        analogWrite(PIN_BLUE, i);
        delay(5);
    }
    if (redWas)   digitalWrite(PIN_RED,   HIGH);
    if (greenWas) digitalWrite(PIN_GREEN, HIGH);
    if (blueWas)  digitalWrite(PIN_BLUE,  HIGH);
}
void blinkAll() {
  bool redWas   = (digitalRead(PIN_RED)   == HIGH);
  bool greenWas = (digitalRead(PIN_GREEN) == HIGH);
  bool blueWas  = (digitalRead(PIN_BLUE)  == HIGH);

  for (int i = 0; i < BLINK_TIMES; i++) {
    allOn();
    delay(BLINK_ON_MS);
    allOff();
    delay(BLINK_OFF_MS);
  }

  if (redWas)   digitalWrite(PIN_RED,   HIGH);
  if (greenWas) digitalWrite(PIN_GREEN, HIGH);
  if (blueWas)  digitalWrite(PIN_BLUE,  HIGH);
}
