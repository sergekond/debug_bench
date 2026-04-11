// ==========================
// Универсальный тестер 74HCxx (DIP-14) на ESP32
// ==========================

#define LED_PIN 2  // D2

// Жёсткая привязка PINx → GPIO
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

// ---------- Мигание ----------

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

// ---------- Тест ----------

bool test_chip(const char* name, Gate gates[], bool (*logic)(int,int)) {

  Serial.printf("\n=== Тест %s ===\n", name);

  bool all_ok = true;

  for (int g = 0; g < 4; g++) {

    bool ok = true;
    Serial.printf("\nЭлемент %d:\n", g + 1);

    for (int a = 0; a <= 1; a++) {
      for (int b = 0; b <= 1; b++) {

        int pinA = PIN[gates[g].A_pin];
        int pinB = PIN[gates[g].B_pin];
        int pinY = PIN[gates[g].Y_pin];

        if (pinA < 0 || pinB < 0 || pinY < 0) {
          Serial.printf("Ошибка: элемент %d использует недопустимый PIN\n", g + 1);
          ok = false;
          continue;
        }

        pinMode(pinA, OUTPUT);
        pinMode(pinB, OUTPUT);
        pinMode(pinY, INPUT);

        digitalWrite(pinA, a);
        digitalWrite(pinB, b);
        delay(50);

        int y = digitalRead(pinY);
        int y_exp = logic(a, b);

        if (y != y_exp) {
          Serial.printf("Элемент %d ошибка: A=%d B=%d -> Y=%d (ожидалось %d)\n",
                        g + 1, a, b, y, y_exp);
          ok = false;
        }
      }
    }

    if (!ok) {
      Serial.printf("НЕИСПРАВЕН элемент %d\n", g + 1);
      all_ok = false;
    } else {
      Serial.printf("Элемент %d OK\n", g + 1);
    }
  }

  Serial.println("\n----");

  return all_ok;
}

// ---------- Команды ----------

String cmd = "";

void handleCommand(String command) {
  command.trim();
  command.toUpperCase();

  bool result;

  if (command == "HC00") {
    result = test_chip("74HC00 (NAND)", HC00, fNAND);
  }
  else if (command == "HC08") {
    result = test_chip("74HC08 (AND)", HC00, fAND);
  }
  else if (command == "HC32") {
    result = test_chip("74HC32 (OR)", HC00, fOR);
  }
  else if (command == "HC02") {
    result = test_chip("74HC02 (NOR)", HC02, fNOR);
  }
  else if (command == "HELP") {
    Serial.println("\nКоманды: HC00 HC08 HC32 HC02");
    return;
  }
  else {
    Serial.println("Неизвестная команда");
    return;
  }

  // ---------- Индикация ----------
  if (result) {
    Serial.println("ВСЁ ИСПРАВНО");
    blinkLED(2);
  } else {
    Serial.println("ОБНАРУЖЕНЫ ОШИБКИ");
    blinkLED(4);
  }
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  for (int i = 1; i <= 13; i++) {
    if (PIN[i] != -1)
      pinMode(PIN[i], INPUT);
  }

  Serial.println("=== Тестер 74HCxx ===");
  Serial.println("Введите команду (HELP)");
}

// ---------- LOOP ----------

void loop() {

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmd.length() > 0) {
        handleCommand(cmd);
        cmd = "";
      }
    } else {
      cmd += c;
    }
  }
}
