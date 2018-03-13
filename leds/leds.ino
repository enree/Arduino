#include <time.h>
#include <stdlib.h>

const char LEDS_COUNT = 5;
const int DELAY = 200;
const int FIRST_BUTTON = 14;
const int WAIT_INPUT = 50;

const int SHUFFLE_CYCLES = 100;

int buttonState[LEDS_COUNT];

class LedsPanel
{
public:
  /**
   * Create LedsPanel object with @a first pin and @a count
   */
  LedsPanel(int first, int count)
    : m_first(first)
    , m_count(count)
    , m_current(0)
    , m_leds(new int[m_count])
    {
    }

  ~LedsPanel()
  {
    delete[] m_leds;
  }
  /**
   * Shuffle leds panel
   */
  void shuffle()
  {
    for (int i = 0; i < m_count; ++i)
    {
      m_leds[i] = i;
    }    

    for (int i = 0; i < SHUFFLE_CYCLES; ++i)
    {
      swap(rand() % m_count, rand() % m_count);
    }
    m_current = 0;
  }

  bool next()
  {
    ++m_current;
    return m_current < m_count;
  }

  /**
   * Blink 
   */
  void blink(int on, int off)
  {
    trigger(true);
    delay(on);
    trigger(false);
    delay(off);
  }
  
  /**
   * Initialize leds
   */
  void init()
  {
    for (int i = m_first; i < m_first + m_count; ++i)
    {
      pinMode(i, OUTPUT);
    }
    shuffle();
  }

  int current() const 
  {
    return m_leds[m_current];
  }
private:
  void swap(int i, int j)
  {
    if (i != j)
    {
      int tmp = m_leds[i];
      m_leds[i] = m_leds[j];
      m_leds[j] = tmp;
    }
  }

  void trigger(bool isOn)
  {
    auto level = isOn ? HIGH : LOW; 
    for (int i = 0; i <= m_current; ++i)
    {
      digitalWrite(m_first + m_leds[i], level);
    }
  }
  
private:
  int m_first;
  int m_count;    
  int m_current;
  int *m_leds;
};

LedsPanel panel(9, 5);

// the setup function runs once when you press reset or power the board
void setup() 
{
  srand (time(NULL));
  panel.init();
  for (int i = 0; i < LEDS_COUNT; ++i)
  {
    pinMode(FIRST_BUTTON + i, INPUT);
    buttonState[i] = LOW;
  }
  // initialize serial communication:
  Serial.begin(9600);
}


bool readButton(int button)
{
  int buttonPin = button + FIRST_BUTTON;
  bool shouldSwitch = false;
  int reading = digitalRead(buttonPin);
  if (reading != buttonState[button])
  {
    delay(WAIT_INPUT);
    reading = digitalRead(buttonPin);
    if (reading != buttonState[button] && reading == HIGH)
    {
      Serial.println("Triggered");
      shouldSwitch = true;  
    }
  }

  buttonState[button] = reading;
  return shouldSwitch;
}

enum PHASE
{
  WAIT_FOR_START,
  WAIT_FOR_BUTTON, 
  BLINK  
} current_phase = BLINK;

unsigned long lastBlinkTime = 0;
const int WAIT_TIMEOUT = 2000;

// the loop function runs over and over again forever
void loop() 
{
  if (current_phase == WAIT_FOR_BUTTON)
  {
    // read buttons
    auto button = readButton(panel.current());
    // check if button good
    if (button)
    {
      Serial.println("Next");
      delay(300);
      if (!panel.next())
      {
        Serial.println("WIN");
        current_phase = WAIT_FOR_START;
      }
      else
      {
        current_phase = BLINK;
      }
    }

    // check if timeout
    long newEventTime = millis();

    if (newEventTime - lastBlinkTime > WAIT_TIMEOUT)
    {
      current_phase = WAIT_FOR_START;
    }
  }

  if (current_phase == BLINK)
  {
    panel.blink(500, 100);
    lastBlinkTime = millis();
    current_phase = WAIT_FOR_BUTTON;
    Serial.println("Wait for button");
  }

  if (current_phase == WAIT_FOR_START && readButton(0))
  {
      panel.shuffle();
      current_phase = BLINK;
  }
//  for (int i = 0; i < LEDS_COUNT; ++i)
//  {
//    bool clicked = readButton(i);
//    if (clicked) 
//    {
//      digitalWrite(FIRST_LED + i, ledState[i]);
//    }
//  }

  
}
