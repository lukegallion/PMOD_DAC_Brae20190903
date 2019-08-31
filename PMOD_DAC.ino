#include <SPI.h>

// Serial Communication Settings
long BAUD = 9600;

// SPI Communication Settings
long SPI_FREQ = 500000;
int SPI_MODE = SPI_MODE0;
int SPI_ENDIAN = MSBFIRST;

// Byte variables for SPI communication
long WRITE = B0000;
long UPDATE = B0001;
long LDAC = B0010;
long WRITE_UPDATE = B0011;
long POWERDOWN = B0100;
long LOAD_CLEAR_CODE = B0101;
long LOAD_LDAC = B0110;
long RESET = B0111;
long SET_INTERNAL_REF = B1000;

//Address bytes for AD5628
byte DAC_A = B0000;
byte DAC_B = B0001;
byte DAC_C = B0010;
byte DAC_D = B0011;
byte DAC_E = B0100;
byte DAC_F = B0101;
byte DAC_G = B0110;
byte DAC_H = B0111;
byte ALL_DAC = B1111;



class Channel {
  /*
   *  Channel for a Analog devices AD5628 DAC from Digilent
   *  12 bit Digital to analog converter with a 2.5 V reference voltage
   *  Vout = 2x Vref x D / 2^12
   *  
   *  SPI protocol will send 32 bits of information
   *  xxx C3 C2 C1 C0 A3 A2 A1 A0 D11 ... D0 xxxxxxx I0
   * Commands are referenced at the start of the file as macros
   * 
   */
  private:    
    unsigned long _command=0;
  public:
    byte dac_pin;
    long voltage_to_write;
    unsigned int voltage_pin = 255;
    unsigned int current_pin = 255;
    Channel();
    float set_voltage(float voltage);
    float get_voltage();
    float get_current();  
};


class DAC {
  private:
    unsigned long _command=0;
    unsigned int _transfer[2];
    void _convert_command();
  public:
    Channel chans[8];
    float voltage_out[8];
    float current_out[8];
    DAC();
    void init_channels();
    void set_ref();
    void reset();
    
    
};

class Oracle {
  // This class will handle reading in the messages and then calling the appropriate function associated to it
  private:
  // Our messages back and forth should be shorter than 32 characters (including terminator '\n')
   char _rx_msg[32];
   char _tx_msg[32];
   bool _str_complete;
   int _msg_idx = 0;
   
  public: 
    Oracle();
    DAC dc;
    long baud;
    void interpret();
    void serialCheck();
};
Oracle::Oracle(){
  
}

void Oracle::interpret(){
  int chnl;
  long somevar = 0;
  // The first letter of the message is used to determine what function to call
  // Second char corresponds to the channel (0-7)
  // The remaining chars are the inputs for that command (if required)
  switch(_rx_msg[0]){

    case 'S':
      Serial.println("Set Voltage");
      break;
    case 'R':
      Serial.println("Read Data");
      break;
    case 'L':
      Serial.println("Load DAC");
      
      somevar |= B001;
      Serial.println(somevar);
      somevar = 0;
      somevar |= UPDATE << 8;
      Serial.println(somevar);
      break;
    case 'X':
      Serial.println("Shutdown");
      break;
    case 'V':
    // Set voltage ADC pin for channel (V02 -> Channel 0, ADC pin 2)
      Serial.println("Set Voltage Pin");
      chnl = _rx_msg[1]-'0';
      dc.chans[chnl].voltage_pin = _rx_msg[2]-'0';
      break;
    case 'C':
    // Set curernt ADC pin for channel ('C31' -> Channel 3, ADC pin 1)
      Serial.println("Set Current Pin");
      // convert char to int
      chnl = _rx_msg[1]-'0';
      dc.chans[chnl].current_pin = _rx_msg[2]-'0';
      break;
      
  }
  Serial.println(_rx_msg);  
}

void Oracle::serialCheck() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    // add it to the _rcx_msg:
    // if the incoming character is a newline,call the interpret function
    if (inChar == '\n') {
       _msg_idx = 0;
      this -> interpret();
      }
    else{
      _rx_msg[_msg_idx]= inChar;
      _msg_idx +=1;
      }
  }
}

Channel::Channel(){
}

float Channel::set_voltage(float voltage){
  // converts float to a digital number for a float
  // voltage is a float in Volts is 3.1 = 3.1 Volts etc...
 
  Serial.println("Set Voltage");
}


DAC::DAC(){

  // Reset
  // Set up internal register
  //Set up channels
}

void DAC::init_channels(){
  
  Serial.println("Initializing Channels...");
  for (int i = 0; i<8; i++){
    chans[i].dac_pin = i;
    Serial.print("Initialized Channel ");
    Serial.println(i); 
  }
}

void DAC::set_ref(){
   _command = 0;
   _command |= SET_INTERNAL_REF <<24;
   _command |=B1;
   SPI.transfer(_command, 4);
   Serial.print("Transfer Complete");
}

Oracle orc;
void setup() {
  // put your setup code here, to run once:
  //Set Serial Settings
  Serial.begin(BAUD);
  Serial.println("Starting PMOD...");
  // Set up SPI Settings
  SPI.beginTransaction(SPISettings(SPI_FREQ, SPI_ENDIAN, SPI_MODE));
  orc.dc.init_channels();
}

void loop() {
  // put your main code here, to run repeatedly:
  orc.serialCheck();
}
