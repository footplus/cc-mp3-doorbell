/*

 Connect the Somo-II TX/RX to pins 10 and 11.

 */
#include <SoftwareSerial.h>

SoftwareSerial somo(10, 11); // RX, TX
unsigned char play_track1[] = { 0x7e, 0x03, 0x00, 0x00, 0x01, 0xFF, 0xFC, 0xEF };
unsigned char play_track2[] = { 0x7e, 0x03, 0x00, 0x00, 0x02, 0xFF, 0xFB, 0xEF };

int counter = 0;              // how many times we have seen new value
int reading;                 // the current value read from the input pin
int button_state = HIGH;    // the debounced input value
int seq_timeout = 1000;       // The press sequence timeout (ms)
long seq_last_press = 0;     // The time of the last press in this sequence.
int seq_press_count = 0;     // The number of presses of this sequence.

// the following variable is a long because the time, measured in milliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long sample_time = 0;      // the last time the output pin was sampled
int debounce_count = 5;     // number of millis/samples to consider before declaring a debounced input


// Door bell input is 12 (active low)
int button = 12;

void setup() {
 
  // Init the sound module
  somo_init();

  // Set the volume to 30
  somo_set_sound(23);

  // set our input pin (bell button)
  pinMode(button, INPUT);

   Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
}

void makeNoise(int presses)
{
  if (presses == 3)
    //somo.write(play_track2, sizeof(play_track2));
    somo_play(2);
  else
    somo_play(1);
    //somo.write(play_track1, sizeof(play_track1));
}

// When this is run, a new button state has been achieved.
void handleNewButtonState()
{
  // Note when it changed.
  unsigned long handle_time = millis();

  if (button_state == LOW)
  {
    seq_last_press = handle_time;
    seq_press_count += 1;
  }
  else
  {
    ; // Button changed state to high.
  }
  

}

void handleSequenceEnd()
{
  unsigned long handle_time = millis();

  // Not in a sequence.
  if (seq_last_press == 0)
    return;

  // In a sequence, have we reached a timeout since last press time ?
  // Or, have we more than 3 presses ?
  if (handle_time - seq_last_press > seq_timeout || seq_press_count >= 3)
  { // The sequence has timed out

    makeNoise(seq_press_count);
    seq_last_press = 0;
    seq_press_count = 0;
  }
}

void loop()
{ // run over and over

  
  // If we have gone on to the next millisecond
  if(millis() != sample_time)
  {
    reading = digitalRead(button);

    if(reading == button_state && counter > 0)
    {
       counter--;
    }
    if(reading != button_state)
    {
       counter++; 
    }
    // If the Input has shown the same value for long enough let's switch it
    if(counter >= debounce_count)
    {
      counter = 0;
      button_state = reading;
      
      // Handle the new input state.
      handleNewButtonState();        
    }
    handleSequenceEnd();
    sample_time = millis();

  }

}

#define SMB_BEGIN 0
#define SMB_CMD 1
#define SMB_FEEDBACK 2
#define SMB_PAR1 3
#define SMB_PAR2 4
#define SMB_CS1 5
#define SMB_CS2 6
#define SMB_END 7 

#define SMC_PLAY_TRACK 0x03
#define SMC_SET_VOL 0x06

void somo_init()
{
  // set the data rate for the SoftwareSerial port
  somo.begin(9600);
}

void somo_play(unsigned char track)
{
  unsigned char buf[8];
  buf[SMB_CMD] = SMC_PLAY_TRACK;
  buf[SMB_FEEDBACK] = 0;
  buf[SMB_PAR1] = 0;
  buf[SMB_PAR2] = track;
  somo_checksum(buf);
  somo.write(buf, sizeof(buf));

}

void somo_set_sound(unsigned char vol)
{
  unsigned char buf[8];
  buf[SMB_CMD] = SMC_SET_VOL;
  buf[SMB_FEEDBACK] = 0;
  buf[SMB_PAR1] = 0;
  buf[SMB_PAR2] = vol;
  somo_checksum(buf);
  somo.write(buf, sizeof(buf));
}

// Calculate the checksum on the 8-byte buffer.
void somo_checksum(unsigned char *cmdbuf)
{
  unsigned short ck = 0xFFFF;
  ck -= cmdbuf[SMB_CMD];
  ck -= cmdbuf[SMB_FEEDBACK];
  ck -= cmdbuf[SMB_PAR1];
  ck -= cmdbuf[SMB_PAR2];
  ck++;

  cmdbuf[SMB_BEGIN] = 0x7E;
  cmdbuf[SMB_CS1] = (unsigned char)(ck / 256);
  cmdbuf[SMB_CS2] = (unsigned char)(ck % 256);
  cmdbuf[SMB_END] = 0xEF;

}

