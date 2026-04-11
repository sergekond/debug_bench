// ==========================
// Универсальный тестер 74HCxx + 74HC74
// ==========================

#define LED_PIN 2

int PIN[15] = {
  -1,
  16, 17, 12, 18, 19, 13,
  -1,
  14, 21, 22, 15, 23, 25,
  -1
};

int PIN16[17] = {
  -1,
  16, // 1 /CLR1
  17, // 2 J1
  12, // 3 /K1
  18, // 4 CLK1
  19, // 5 /PRE1
  13, // 6 Q1
  26, // 7 /Q1
  -1, // 8 GND
  27, // 9 /Q2
  14, // 10 Q2
  21, // 11 /PRE2
  22, // 12 CLK2
  15, // 13 /K2
  23, // 14 J2
  25, // 15 /CLR2
  -1  // 16 VCC
};

struct Gate {
  int A_pin;  
  int B_pin;
  int Y_pin;
};

// ---------- Логика ----------

bool fNAND(int a, int b) { return !(a & b); }
bool fAND (int a, int b) { return  (a & b); }
bool fOR  (int a, int b) { return  (a | b); }
bool fNOR (int a, int b) { return !(a | b); }

// ---------- Топологии ----------

Gate HC00[4] = {
  {1, 2, 3},
  {4, 5, 6},
  {9, 10, 8},
  {12, 13, 11}
};

Gate HC02[4] = {
  {2, 3, 1},
  {5, 6, 4},
  {8, 9, 10},
  {11, 12, 13}
};

// ---------- LED ----------

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

// ---------- Универсальный тест логики ----------

bool test_chip(const char* name, Gate gates[], bool (*logic)(int,int)) {

  Serial.printf("\n=== Тест %s ===\n", name);

  bool all_ok = true;

  for (int g = 0; g < 4; g++) {

    bool ok = true;

    for (int a = 0; a <= 1; a++) {
      for (int b = 0; b <= 1; b++) {

        int pinA = PIN[gates[g].A_pin];
        int pinB = PIN[gates[g].B_pin];
        int pinY = PIN[gates[g].Y_pin];

        pinMode(pinA, OUTPUT);
        pinMode(pinB, OUTPUT);
        pinMode(pinY, INPUT);

        digitalWrite(pinA, a);
        digitalWrite(pinB, b);
        delay(10);

        int y = digitalRead(pinY);
        int y_exp = logic(a, b);

        if (y != y_exp) {
          Serial.printf("Элемент %d ошибка A=%d B=%d\n", g + 1, a, b);
          ok = false;
        }
      }
    }

    if (!ok) {
      Serial.printf("НЕИСПРАВЕН элемент %d\n", g + 1);
      all_ok = false;
    }
  }

  return all_ok;
}

// ---------- ТЕСТ 74HC74 ----------

bool test_d_flipflop(int clr, int d, int clk, int pre, int q, int nq, int id) {

  Serial.printf("\nТриггер %d\n", id);

  int pinCLR = PIN[clr];
  int pinD   = PIN[d];
  int pinCLK = PIN[clk];
  int pinPRE = PIN[pre];
  int pinQ   = PIN[q];
  int pinNQ  = PIN[nq];

  pinMode(pinCLR, OUTPUT);
  pinMode(pinD, OUTPUT);
  pinMode(pinCLK, OUTPUT);
  pinMode(pinPRE, OUTPUT);
  pinMode(pinQ, INPUT);
  pinMode(pinNQ, INPUT);

  bool ok = true;

  digitalWrite(pinCLR, HIGH);
  digitalWrite(pinPRE, HIGH);

  // RESET
  digitalWrite(pinCLR, LOW);
  delay(5);
  digitalWrite(pinCLR, HIGH);
  delay(5);

  int qVal = digitalRead(pinQ);
  int nqVal = digitalRead(pinNQ);

  if (qVal != 0 || nqVal != 1) {
    Serial.println("Ошибка RESET");
    ok = false;
  }

  // SET
  digitalWrite(pinPRE, LOW);
  delay(5);
  digitalWrite(pinPRE, HIGH);
  delay(5);

  qVal = digitalRead(pinQ);
  nqVal = digitalRead(pinNQ);

  if (qVal != 1 || nqVal != 0) {
    Serial.println("Ошибка SET");
    ok = false;
  }

  // WRITE 1
  digitalWrite(pinD, HIGH);
  digitalWrite(pinCLK, HIGH);
  delay(5);
  digitalWrite(pinCLK, LOW);
  delay(5);

  qVal = digitalRead(pinQ);
  nqVal = digitalRead(pinNQ);

  if (qVal != 1 || nqVal != 0) {
    Serial.println("Ошибка записи 1");
    ok = false;
  }

  // WRITE 0
  digitalWrite(pinD, LOW);
  digitalWrite(pinCLK, HIGH);
  delay(5);
  digitalWrite(pinCLK, LOW);
  delay(5);

  qVal = digitalRead(pinQ);
  nqVal = digitalRead(pinNQ);

  if (qVal != 0 || nqVal != 1) {
    Serial.println("Ошибка записи 0");
    ok = false;
  }

  // Проверка инверсии
  if (qVal == nqVal) {
    Serial.println("Q и /Q не инверсны");
    ok = false;
  }

  if (ok) Serial.println("OK");

  return ok;
}

bool test_74HC74() {

  Serial.println("\n=== Тест 74HC74 ===");

  bool ok1 = test_d_flipflop(1, 2, 3, 4, 5, 6, 1);
  bool ok2 = test_d_flipflop(13, 12, 11, 10, 9, 8, 2);

  return ok1 && ok2;
}

// ---------- helper: safe clock pulse ----------
void clkPulse(int clkPin) {
  digitalWrite(clkPin, LOW);
  delay(2);
  digitalWrite(clkPin, HIGH);
  delay(2);
  digitalWrite(clkPin, LOW);
}

// ---------- stable init ----------
void init109(int clr, int pre) {
  digitalWrite(clr, LOW);
  digitalWrite(pre, HIGH);
  delay(5);
  digitalWrite(clr, HIGH);
  delay(5);
  digitalWrite(pre, HIGH);
  delay(5);
}

bool test_jk_flipflop(int clr, int j, int k, int clk, int pre, int q, int nq, int id) {

  Serial.printf("\n=== Тест 74HC109 %d ===", id);

  int pCLR = PIN16[clr];
  int pJ   = PIN16[j];
  int pK   = PIN16[k];   
  int pCLK = PIN16[clk];
  int pPRE = PIN16[pre];
  int pQ   = PIN16[q];
  int pNQ  = PIN16[nq];

  pinMode(pCLR, OUTPUT);
  pinMode(pPRE, OUTPUT);
  pinMode(pJ, OUTPUT);
  pinMode(pK, OUTPUT);
  pinMode(pCLK, OUTPUT);

  pinMode(pQ, INPUT);
  pinMode(pNQ, INPUT);

  bool ok = true;

  // safe idle
  digitalWrite(pPRE, HIGH);
  digitalWrite(pCLR, HIGH);

  // =========================
  // RESET (CLR active LOW)
  // =========================
  digitalWrite(pCLR, LOW);
  delay(5);
  digitalWrite(pCLR, HIGH);
  delay(5);

  if (digitalRead(pQ) != 0 || digitalRead(pNQ) != 1) {
    Serial.println("RESET FAIL");
    ok = false;
  }

  // =========================
  // SET (PRE active LOW)
  // =========================
  digitalWrite(pPRE, LOW);
  delay(5);
  digitalWrite(pPRE, HIGH);
  delay(5);

  if (digitalRead(pQ) != 1 || digitalRead(pNQ) != 0) {
    Serial.println("SET FAIL");
    ok = false;
  }

  // =========================
  // J=1 K=0 (/K inactive = HIGH)
  // =========================
  digitalWrite(pJ, HIGH);
  digitalWrite(pK, HIGH); // /K = 1 (inactive)

  clkPulse(pCLK);

  if (digitalRead(pQ) != 1) {
    Serial.println("J=1 /K=0 FAIL");
    ok = false;
  }

  // =========================
  // J=0 K=1 (/K active LOW)
  // =========================
  digitalWrite(pJ, LOW);
  digitalWrite(pK, LOW); // /K = 0 → active

  clkPulse(pCLK);

  if (digitalRead(pQ) != 0) {
    Serial.println("J=0 /K=1 FAIL");
    ok = false;
  }

  // =========================
  // TOGGLE (J=1 K=1 → /K=1)
  // =========================
  digitalWrite(pJ, HIGH);
  digitalWrite(pK, HIGH);

  int before = digitalRead(pQ);

  clkPulse(pCLK);

  int after = digitalRead(pQ);

  if (before == after) {
    Serial.println("TOGGLE FAIL");
    ok = false;
  }

  // =========================
  // CHECK /Q
  // =========================
  if (digitalRead(pQ) == digitalRead(pNQ)) {
    Serial.println("Q != /Q FAIL");
    ok = false;
  }

  if (ok) Serial.println("OK");

  return ok;
}


bool test_SN74HC109() {

  Serial.println("\n=== Тест SN74HC109 (JK) ===");

  bool ok1 = test_jk_flipflop(1, 2, 3, 4, 5, 6, 7, 1);
  bool ok2 = test_jk_flipflop(15, 14, 13, 12, 11, 10, 9, 2);

  return ok1 && ok2;
}

String cmd = "";

void handleCommand(String command) {
  command.trim();
  command.toUpperCase();

  bool result;

  if (command == "HC00") {
    result = test_chip("74HC00", HC00, fNAND);
  }
  else if (command == "HC08") {
    result = test_chip("74HC08", HC00, fAND);
  }
  else if (command == "HC32") {
    result = test_chip("74HC32", HC00, fOR);
  }
  else if (command == "HC02") {
    result = test_chip("74HC02", HC02, fNOR);
  }
  else if (command == "HC74") {
    result = test_74HC74();
  }
  else if (command == "HC109") {
    result = test_SN74HC109();
  }
  else {
    Serial.println("Неизвестная команда");
    return;
  }

  if (result) {
    Serial.println("OK");
    blinkLED(2);
  } else {
    Serial.println("FAIL");
    blinkLED(4);
  }
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("=== TESTER ===");
  Serial.println("HC00 HC08 HC32 HC02 HC74 HC109");
}

// ---------- LOOP ----------

void loop() {

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmd.length()) {
        handleCommand(cmd);
        cmd = "";
      }
    } else {
      cmd += c;
    }
  }
}
