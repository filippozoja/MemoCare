#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h> 


// ——— PINOUT ———————————————————————————————————————————————
#define DIR_PIN       32
#define STEP_PIN      33
#define SWITCH_A_PIN  26  // finecorsa A (START)
#define SWITCH_B_PIN  27  // finecorsa B (STOP)
#define SS_RX_PIN 21

#define SS_TX_PIN 22
// ——— BUZZER ——————————————————————————————————————————————
#define BUZZER_PIN      2
#define BUZZER_CHANNEL  0
#define BUZZER_FREQ     2000
#define BUZZER_RES       8

// ——— PARAMETRI ——————————————————————————————————————————————
const float STEPPER_SPEED    = 600.0;     // passi/sec costanti
const int   DEBOUNCE_MS      = 100;       // debounce finecorsa (ms)
const long  TOTAL_STEPS      = 60000;     // passi totali definiti
const long  ACTIVE_START_SEC = 5 * 3600L;   // 05:00 = 5*3600 s
const long  ACTIVE_END_SEC   = 23 * 3600L;  // 23:00 = 23*3600 s
const long  ACTIVE_WINDOW    = ACTIVE_END_SEC - ACTIVE_START_SEC; // 18h in s

// ——— SOFT BLINK PARAMETERS —————————————————————————————————————
const unsigned long BLINK_INTERVAL_MS = 500;
unsigned long lastBlinkTime = 0;
bool blinkState = false;

long secs = 0;
int currHour = 0;

// ——— VARIABILI PER LED E FASCE ——————————————————————————————————————
bool activityWindow[6] = { false, false, false, false, false, false };
//   ^ ad ogni indice corrisponde una fascia:
//   idx 0 → 05:00–07:59
//   idx 1 → 08:00–10:59
//   idx 2 → 11:00–13:59
//   idx 3 → 14:00–16:59
//   idx 4 → 17:00–19:59
//   idx 5 → 20:00–22:59

const uint8_t ledPins[] = { 12, 4, 21, 25, 5, 18 };  // ordine dei pin per le 6 strisce
const uint8_t numLeds = sizeof(ledPins) / sizeof(ledPins[0]);

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
SoftwareSerial swSerial(SS_RX_PIN, SS_TX_PIN); // RX, TX

// ——— WI-FI & JSON ————————————————————————————————————————————
//const char* ssid       = "TIM-24867805";
//const char* password   = "tKt9fcHLdGg2gmcgMrnXpGsb";
const char* ssid       = "iPhone di Filippo";
const char* password   = "1qaz2wsx";
const char* serverURL  = "https://titani.filippozoja.com/php/get_attivita.php?username=filippo";
// Endpoint per invio feedback: usa il file post_feedback.php
const char* feedbackURL = "https://titani.filippozoja.com/php/post_feedback.php";
static const size_t JSON_DOC_SIZE = 8 * 1024;

long getSecondsSinceMidnight() {
  struct tm ti;
  if (!getLocalTime(&ti)) {
    return -1;
  }
  return ti.tm_hour * 3600L + ti.tm_min * 60L + ti.tm_sec;
}

// Debounce con runSpeed per mantenere il motore in movimento
bool debounceCheck(int pin, int target) {
  unsigned long t0 = millis();
  while (millis() - t0 < DEBOUNCE_MS) {
    if (digitalRead(pin) != target) return false;
    stepper.runSpeed();
  }
  return true;
}

void connectWiFi() {
  Serial.print("→ Connetto Wi-Fi '");
  Serial.print(ssid);
  Serial.println("' …");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("→ IP locale: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Scarica e stampa JSON
void fetchAndPrintActivities() {
  Serial.println("→ Scarico attività JSON…");
  HTTPClient http;
  http.begin(serverURL);
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("Errore HTTP %d\n", code);
    http.end();
    return;
  }
  String payload = http.getString();
  http.end();

  Serial.println("=== JSON ===");
  Serial.println(payload);
  Serial.println("============\n");

  DynamicJsonDocument doc(JSON_DOC_SIZE);
  if (deserializeJson(doc, payload)) {
    Serial.println("deserializeJson() fallita");
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  Serial.printf("Trovati %u record\n\n", arr.size());

  // Inizializzo a false (anche se già lo sono di default)
  for (int i = 0; i < 6; i++) {
    activityWindow[i] = false;
  }

  for (auto obj : arr) {
    // Stampa a schermo (come prima)
    Serial.println("-- Attività --");
    Serial.printf("slot:           %s\n", obj["slot"]          | "(nullo)");
    Serial.printf("nome:           %s\n", obj["nome"]          | "(nullo)");
    Serial.printf("ora_inizio:     %s\n", obj["ora_inizio"]    | "(nullo)");
    Serial.printf("durata:         %s\n", obj["durata"]        | "(nullo)");
    Serial.printf("preavviso:      %s\n", obj["preavviso"]     | "(nullo)");
    Serial.printf("dove:           %s\n", obj["dove"]          | "(nullo)");
    Serial.printf("conchi:         %s\n", obj["conchi"]        | "(nullo)");
    Serial.printf("tema:           %s\n", obj["tema"]          | "(nullo)");
    Serial.printf("icona:          %s\n", obj["icona"]         | "(nullo)");
    Serial.printf("data_attivita:  %s\n", obj["data_attivita"] | "(nullo)");
    Serial.printf("ultima_modifica:%s\n\n", obj["ultima_modifica"]|(const char*)"(nullo)");

    // ——— ECCO LA PARTE CHE POPOLA activityWindow[] ————————————————————
    const char* ora = obj["ora_inizio"]; 
    // Supponiamo il formato “HH:MM”. Estraggo solo le due cifre dell’ora:
    if (ora != nullptr && strlen(ora) >= 2) {
      int h = (ora[0] - '0') * 10 + (ora[1] - '0'); 
      // Controllo se rientra tra le fasce 05:00–22:59
      if (h >= 5 && h < 23) {
        int idx = (h - 5) / 3; 
        // idx varia da 0 a 5
        if (idx >= 0 && idx < 6) {
          activityWindow[idx] = true;
          Serial.printf("→ Attività nella fascia idx=%d (h=%d)\n", idx, h);
        }
      }
    }
  }
}

// Sends the current hour (HH) over Serial and swSerial
void sendHour() {
  long sec = getSecondsSinceMidnight();
  if (sec < 0) return; // NTP not ready or error
  int hour = sec / 3600;
  char hourStr[3];
  snprintf(hourStr, sizeof(hourStr), "%02d", hour);
  Serial.println(hourStr);
  swSerial.print(hourStr);
  swSerial.print("\n");
}

// Overloaded sendHour to force a specific hour
void sendHour(int forcedHour) {
  char hourStr[3];
  snprintf(hourStr, sizeof(hourStr), "%02d", forcedHour);
  Serial.println(hourStr);
  swSerial.print(hourStr);
  swSerial.print("\n");
}

// ----------------------------------------------------------
#define BUTTON_PIN 15
bool prevButtonState = false;
bool prevSwitchBState = false;
bool prevSwitchAState = false;
// Reads the button on BUTTON_PIN, prints change events, and returns the current state
bool readButton() {
  bool currState = digitalRead(BUTTON_PIN);
  if (currState != prevButtonState) {
    Serial.printf("Button state changed: %s\n", currState == LOW ? "PRESSED" : "RELEASED");
    prevButtonState = currState;
    // Solo quando il pulsante viene rilasciato (INPUT_PULLUP HIGH)
    if (currState == HIGH) {
      sendFeedback(currState, 5);
    }
  }
  return currState;
}


// Prototipo per funzione buzzer
void buzzSuccess(int duration_ms);

// Spegne tutte le luci LED
void turnOffLEDs() {
  for (uint8_t i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

// Aggiorna i LED in base all'ora specificata
void updateLEDsForHour(int hour) {
  turnOffLEDs();
  if (hour >= 5 && hour < 23) {
    int idx = (hour - 5) / 3;
    if (idx >= 0 && idx < numLeds && activityWindow[idx]) {
      digitalWrite(ledPins[idx], HIGH);
    }
  }
}

// Invia un feedback (stato pulsante) al server via HTTP POST
void sendFeedback(bool buttonState, int slot)
{
  turnOffLEDs();
  HTTPClient http;
  http.begin(feedbackURL);
  http.addHeader("Content-Type", "application/json");
  // Ottengo timestamp corrente
  struct tm ti;
  if (!getLocalTime(&ti)) {
    Serial.println("Errore NTP per feedback");
    http.end();
    return;
  }
  char ts[20];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02d %02d:%02d:%02d",
           ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
           ti.tm_hour, ti.tm_min, ti.tm_sec);
  // Costruisco JSON con slot e timestamp (ArduinoJson)
  DynamicJsonDocument fbDoc(128);
  fbDoc["username"] = "filippo";
  fbDoc["slot"] = slot;
  fbDoc["timestamp"] = ts;
  String payload;
  serializeJson(fbDoc, payload);
  // DEBUG: stampa il payload inviato al server
  Serial.println("Payload feedback: " + payload);
  int httpCode = http.POST(payload);
  String response = http.getString();
  if (httpCode > 0) {
    Serial.printf("Feedback inviato, codice HTTP: %d\n", httpCode);
    Serial.println("Risposta server: " + response);
    // Suono conferma invio feedback
    buzzSuccess(200);
  } else {
    Serial.printf("Errore invio feedback: %s\n", http.errorToString(httpCode).c_str());
    Serial.println("Dettaglio risposta server: " + response);
  }
  http.end();
}

/**
 * Emette un breve beep sul buzzer per indicare
 * invio del feedback riuscito.
 * @param duration_ms  durata del beep in millisecondi
 */
void buzzSuccess(int duration_ms) {
  // Emette un pattern di notifica positiva (3 beep ascendenti)
  int freqs[] = {1000, 1200, 1400};
  int durs[] = {100, 100, 100};
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, freqs[i], durs[i]);
    delay(durs[i] + 20);
    noTone(BUZZER_PIN);
  }
}

long lastInterval = -1;

// Sequencer per flusso custom
int sequenceState = 0;
int case1Dir = 0; // +1 or -1: direction used in case 1

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  swSerial.begin(115200);
  delay(100);

  // Inizializza il buzzer (nessuna configurazione necessaria con tone())

  
  // Configura tutti i pin come OUTPUT e li spegne all'avvio
  for (uint8_t i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }


  //WIFI
  connectWiFi();

  //JSON
  fetchAndPrintActivities();

  // 1) Inizializzo PIN
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(SWITCH_A_PIN, INPUT_PULLUP);
  pinMode(SWITCH_B_PIN, INPUT_PULLUP);
  // Configuro pulsante su pin 15
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  prevButtonState = digitalRead(BUTTON_PIN);
  prevSwitchBState = digitalRead(SWITCH_B_PIN);
  prevSwitchAState = digitalRead(SWITCH_A_PIN);

  // 2) Configuro lo stepper per velocità costante
  stepper.setMaxSpeed(STEPPER_SPEED);

  // 3) HOMING → torno a finecorsa A (START) usando +600 passi/sec
  stepper.setSpeed(+STEPPER_SPEED);

  Serial.println("=== HOMING: vado verso A (START) ===");
  int restA = digitalRead(SWITCH_A_PIN);
  while (true) {
    stepper.runSpeed();
    int a = digitalRead(SWITCH_A_PIN);
    if (a != restA && debounceCheck(SWITCH_A_PIN, a)) {
      Serial.println("→ Raggiunto START (A), azzero posizione a 0.");
      break;
    }
  }

  stepper.setCurrentPosition(0);
  delay(200);

  stepper.setSpeed(0);
  Serial.printf("  [DEBUG] Dopo homing → currentPosition = %ld\n", stepper.currentPosition());
  delay(200);

  //sincronizzo NTP
  configTzTime("CET-1CEST-2,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
  delay(200);

  Serial.print("→ Attendo orario NTP valido");
  struct tm ti;
  while (true) {
    if (getLocalTime(&ti) && (ti.tm_year + 1900 >= 2021)) {
      break;
    }
    Serial.print(".");
    delay(500);
  }
  Serial.println(" OK (NTP sincronizzato).");
  delay(200);

  // 5) POSIZIONAMENTO INIZIALE in base all'ora corrente
  secs = getSecondsSinceMidnight();
  Serial.printf("Ora: %02ld:%02ld:%02ld  (%ld s)\n\n",
                secs/3600, (secs%3600)/60, secs%60, secs);

  if (secs >= ACTIVE_START_SEC && secs < ACTIVE_END_SEC) {
    long elapsed = secs - ACTIVE_START_SEC;              // secondi dopo le 05:00
    float frac = float(elapsed) / float(ACTIVE_WINDOW);  // frazione di 18h
    long targetPos = round(frac * TOTAL_STEPS);          // passi corrispondenti

    Serial.printf("→ Target iniziale = %ld passi  (elapsed = %ld sec)\n",
                  targetPos, elapsed);

    // Debug: mostra la posizione prima del movimento
    Serial.printf("  [DEBUG] Posizione prima movimento = %ld\n", stepper.currentPosition());
    //dovrebbe essere uguale a 0

    stepper.setMaxSpeed(STEPPER_SPEED);

    // Imposto direzione e velocità
    if (targetPos < stepper.currentPosition()) {
      stepper.setSpeed(+STEPPER_SPEED);
    } else {
      stepper.setSpeed(-STEPPER_SPEED);
    }

    // Soft blink LEDs until arrival
    lastBlinkTime = millis();
    blinkState = false;
    while (-stepper.currentPosition() != targetPos) {
      stepper.runSpeed();
      long pos = -stepper.currentPosition();
      // Soft blink all LEDs
      if (millis() - lastBlinkTime >= BLINK_INTERVAL_MS) {
        lastBlinkTime = millis();
        blinkState = !blinkState;
        for (uint8_t i = 0; i < numLeds; i++) {
          //digitalWrite(ledPins[i], blinkState ? HIGH : LOW);
        }
      }
      if (pos % 5000 == 0) {
        Serial.printf("    [DEBUG] currentPosition intermedia = %ld\n", pos);
      }
    }
    // Stop blinking and clear LEDs
    turnOffLEDs();
    Serial.println("→ Arrivato al target iniziale!");
    Serial.printf("  [DEBUG] Posizione dopo movimento = %ld\n", -stepper.currentPosition());
  } else {
    // Fuori orario (23–05): fermo su START
    stepper.setSpeed(0);
    Serial.println("→ Fuori orario (23:00 05:00): motore fermo su START.");
  }
  Serial.println("=== FINE POSIZIONAMENTO ===\n");

  // Calcolo l'ora corrente come intero [0..23]
  currHour = secs / 3600;
  // Invio l'ora via swSerial e Serial
  sendHour();


  // ——— GESTIONE LED: accendo solo quello della fascia corrente se c'è attività ——————————————————
  // Spengo tutte le strisce LED (già fatto in apertura, ma nel dubbio riconfermo)
  for (uint8_t i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  if (currHour >= 5 && currHour < 23) {
    int currIdx = (currHour - 5) / 3;  // fascia corrente tra 0 e 5
    if (currIdx >= 0 && currIdx < 6) {
      if (activityWindow[currIdx]) {
        digitalWrite(ledPins[currIdx], HIGH);
        Serial.printf("→ Acceso LED idx=%d (fascia ore %d–%d) perché c'è attività.\n",
                      currIdx, 5 + currIdx*3, 5 + currIdx*3 + 2);
      } else {
        Serial.printf("→ Nessuna attività per fascia idx=%d (ore %d–%d): LED spento.\n",
                      currIdx, 5 + currIdx*3, 5 + currIdx*3 + 2);
      }
    }
  } else {
    Serial.println("→ Fuori orario 05:00–22:59: tutti i LED rimangono spenti.");
  }

  Serial.println("=== FINE SETUP ===\n");
}

void loop() {
  // Rilevo rilascio pulsante (INPUT_PULLUP)
  static bool prevState = HIGH;
  bool currState = digitalRead(BUTTON_PIN);
  bool buttonReleased = false;
  if (currState != prevState) {
    if (currState == HIGH) {
      buttonReleased = true;
    }
    prevState = currState;
  }

  // DEBUG: mostro lo stato corrente della sequenza
  Serial.printf("DEBUG: Entering loop with sequenceState=%d\n", sequenceState);

  switch (sequenceState) {
    case 0:
      Serial.println("DEBUG: State 0 - waiting for button release to send slot 3 feedback");
      // Fermo, aspetto rilascio, poi invio feedback slot 3
      if (buttonReleased) {
        sendFeedback(true, 3);
        // Ripristino LED per l'ora corrente
        {
          long secsNow = getSecondsSinceMidnight();
          if (secsNow >= 0) {
            int hourNow = secsNow / 3600;
            updateLEDsForHour(hourNow);
          }
        }
        sequenceState = 1;
      }
      break;
    case 1: {
      Serial.println("DEBUG: State 1 - moving by +2 hours");
      // Calcolo l'ora corrente e aggiungo due ore
      long secsNow = getSecondsSinceMidnight();
      if (secsNow < 0) {
        Serial.println("Errore nel calcolo dell'ora corrente.");
        break;
      }
      int currHourNow = secsNow / 3600;
      int targetHour = (currHourNow + 2) % 24;
      // Calcolo la posizione target in base a targetHour
      long relSec = targetHour * 3600L - ACTIVE_START_SEC;
      long targetPos = round(float(relSec) / float(ACTIVE_WINDOW) * TOTAL_STEPS);
      // Muovo stepper verso targetPos
      stepper.setMaxSpeed(STEPPER_SPEED);
      // Store direction for reuse in case3 and case4
      case1Dir = (targetPos < stepper.currentPosition()) ? +1 : -1;
      stepper.setSpeed(case1Dir * STEPPER_SPEED);
      while (-stepper.currentPosition() != targetPos) {
        stepper.runSpeed();
      }
      stepper.setSpeed(0);
      // Invio ora e aggiorno LED
      sendHour(targetHour);
      updateLEDsForHour(targetHour);
      sequenceState = 2;
      break;
    }
    case 2:
      Serial.println("DEBUG: State 2 - waiting for button release to send slot 5 feedback");
      // Fermo, aspetto rilascio, poi invio feedback slot 5
      if (buttonReleased) {
        sendFeedback(true, 5);
        // Salvo lo stato iniziale del finecorsa B per case 3
        prevSwitchBState = digitalRead(SWITCH_B_PIN);
        sequenceState = 3;
      }
      break;
    case 3:
      Serial.println("DEBUG: State 3 - moving towards switch B");
      // Muovo verso finecorsa B e rilevo cambiamento di stato (qualsiasi livello)
      stepper.setSpeed(case1Dir * STEPPER_SPEED);
      while (true) {
        stepper.runSpeed();
        bool currB = digitalRead(SWITCH_B_PIN);
        if (currB != prevSwitchBState && debounceCheck(SWITCH_B_PIN, currB)) {
          prevSwitchBState = currB;
          break;
        }
      }
      stepper.setSpeed(0);
      stepper.setCurrentPosition(0);
      sequenceState = 4;
      break;
    case 4:
      Serial.println("DEBUG: State 4 - returning to switch A");
      // Torno a finecorsa A
      stepper.setSpeed(-case1Dir * STEPPER_SPEED);
      while (true) {
        stepper.runSpeed();
        bool currA = digitalRead(SWITCH_A_PIN);
        if (currA != prevSwitchAState) {
          // cambio di stato sul finecorsa A
          if (currA == LOW && debounceCheck(SWITCH_A_PIN, LOW)) {
            prevSwitchAState = currA;
            break;
          }
          prevSwitchAState = currA;
        }
      }
      stepper.setSpeed(0);
      // Fine sequenza
      sequenceState = 5;
      break;
    default:
      Serial.println("DEBUG: State default - sequence completed");
      // Sequenza conclusa: fermo
      stepper.setSpeed(0);
      break;
  }

  delay(50);
}