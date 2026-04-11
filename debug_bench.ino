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

bool test_flipflop(int clr, int d, int clk, int pre, int q, int nq, int id) {

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

  if (ok) Serial.println("✔️ OK");

  return ok;
}

bool test_74HC74() {

  Serial.println("\n=== Тест 74HC74 ===");

  bool ok1 = test_flipflop(1, 2, 3, 4, 5, 6, 1);
  bool ok2 = test_flipflop(13, 12, 11, 10, 9, 8, 2);

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
  Serial.println("HC00 HC08 HC32 HC02 HC74");
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
