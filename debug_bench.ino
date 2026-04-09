// ==========================
// Универсальный тестер 74HCxx (DIP-14) на ESP32
// ==========================

// Жёсткая привязка PINx → GPIO
int PIN[15] = {
  -1,   // 0 - не используется
  16,   // PIN1
  17,   // PIN2
  12,   // PIN3
  18,   // PIN4
  19,   // PIN5
  13,   // PIN6
  -1,   // PIN7 = GND
  14,   // PIN8
  21,   // PIN9
  22,   // PIN10
  15,   // PIN11
  23,   // PIN12
  25,    // PIN13
  -1// PIN14 = VCC
};

struct Gate {
  int A_pin;  
  int B_pin;
  int Y_pin;
};

// ---------- Логические функции ----------

bool fNAND(int a, int b) { return !(a & b); }
bool fAND (int a, int b) { return  (a & b); }
bool fOR  (int a, int b) { return  (a | b); }
bool fNOR (int a, int b) { return !(a | b); }

// ---------- Топологии микросхем ----------

// 74HC00 
Gate HC00[4] = {
  {1, 2, 3},
  {4, 5, 6},
  {9, 10, 8},
  {12, 13, 11}
};


// 74HC02 — NOR (другая топология)
Gate HC02[4] = {
  {2, 3, 1},
  {5, 6, 4},
  {8, 9, 10},
  {11, 12, 13}
};

// ---------- Универсальный тест ----------

void test_chip(const char* name, Gate gates[], bool (*logic)(int,int)) {

  Serial.printf("\n=== Тест %s ===\n", name);

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

        Serial.printf("A=%d B=%d -> Y=%d (ожидалось %d) %s\n",
                      a, b, y, y_exp,
                      (y == y_exp ? "OK" : "FAIL"));

        if (y != y_exp) ok = false;
      }
    }

    Serial.printf("Итог элемента %d: %s\n",
                  g + 1, ok ? "OK" : "FAIL");
  }

  Serial.println("\n----");
}

// ---------- SETUP ----------

void setup() {
  Serial.begin(115200);

  for (int i = 1; i <= 13; i++) {
    if (PIN[i] != -1)
      pinMode(PIN[i], INPUT);
  }
  
  Serial.println("=== Универсальный тестер DIP-14 (74HCxx) ===");
}

// ---------- LOOP ----------

void loop() {

  // Выбирай, что стоит в панельке:

  test_chip("74HC00 (NAND)", HC00, fNAND);
  test_chip("74HC08 (AND)",  HC00, fAND);
  test_chip("74HC32 (OR)",   HC00, fOR);
  test_chip("74HC02 (NOR)",  HC02, fNOR);

  delay(5000);
}
