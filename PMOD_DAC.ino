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


// Class definitions

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
   * This class keeps the DAC and ADC channels together
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
  /*
   * This class is keeps all the DAC channels. It is responsible for group commands to the PMOD 
   *  ie load DAC, reset, shutdown etc...
   *  
   *  It also contains the voltage and current arrays that contain data from all the channels
   */
  private:
    unsigned long _command=0;
    unsigned int _transfer[2];
    void _convert_command();
  public:
    DAC();
    //Fields
    Channel chans[8];
    float voltage_out[8];
    float current_out[8];

    // Methods
    void init_channels();
    void set_ref();
    void reset();
    void get_data(); 
};

void DAC::get_data(){
  Serial.println("Getting Data...");
  for (int i = 0; i < 8; i++){
    voltage_out[i]=chans[i].get_voltage();
    current_out[i]=chans[i].get_current();
    
  }
}

class Oracle {
  /*
   * This class is responsible for communicating with the computer, interpreting and calling commands from the other classes. 
   * _rx_msg contains a character array that will hold the incoming message from the computer
   * First char is the command
   * Second char is the channel (if applicable)
   * Third char can be the pin number for the ADC,  
   * Or 
   * The 3rd-10th char are the float encoded in ascii text for the set voltage command
   * 
   */
  private:
  // Our messages back and forth should be shorter than 32 characters (including terminator '\n')
   char _rx_msg[32];
   bool _str_complete;
   int _msg_idx = 0;
   float _get_float_portion(int start, int finish);
   
  public: 
    Oracle();
    DAC dc;
    long baud;
    void interpret();
    void serialCheck();
    void send_data_array(float *arg, int buffer_size);
    void send_float(float f);
};

//***************************** Oracle ****************************
Oracle::Oracle(){
  
}

void Oracle::interpret(){
  int chnl;
  long somevar = 0;
  float in_data;
  // The first letter of the message is used to determine what function to call
  // Second char corresponds to the channel (0-7)
  // The remaining chars are the inputs for that command (if required)
  switch(_rx_msg[0]){
    case 'S':
      Serial.println("Set Voltage");
      chnl = _rx_msg[1]-'0';
      in_data = this-> _get_float_portion(2,10);
      dc.chans[chnl].set_voltage(in_data);
      break;
    case 'R':
      Serial.println("Read Data");
      chnl = _rx_msg[1]-'0';
      dc.get_data();
      Serial.print("F_START");
      this -> send_data_array( dc.voltage_out, 8);
      this -> send_data_array( dc.current_out, 8);
      
      break;
    case 'L':
      Serial.println("Load DAC");
      somevar |= LOAD_LDAC << 24;
      break;
    case 'X':
      Serial.println("PowerDown");
      somevar |= POWERDOWN << 24;
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
float Oracle::_get_float_portion(int start, int finish){
 /*
  * Returns the float portion of the recieved message specified by start and finish
  */
    float volt;
    char temp_array[finish-start];
    for (int i=start; i<finish; i++){
      temp_array[i-2]=_rx_msg[i];
    }

    volt = atof(temp_array);
    return volt;
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

void Oracle::send_data_array(float *arg, int buffer_size){
  for (int i = 0; i < buffer_size; i++){
    this-> send_float(arg[i]);
  }
  
}

void Oracle::send_float(float f){
  byte *b = (byte *) &f;
  Serial.write(b,4);  
}


// ******************************** Channel *************************************
Channel::Channel(){
}

float Channel::set_voltage(float voltage){
  // converts float to a digital number for a float
  // voltage is a float in Volts is 3.1 = 3.1 Volts etc...
 
  Serial.println("Set Voltage");
}

float Channel::get_voltage(){
  float data = 0;
  if (voltage_pin != 255){
    data = analogRead(voltage_pin);
  }
  return data;
}

float Channel::get_current(){
  float data = 0;
  if (current_pin != 255){
    data = analogRead(current_pin);
  }
  return data;
}

// **************************** DAC **********************************
DAC::DAC(){
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
   byte *cmd = (byte*)_command;
   SPI.transfer(cmd, 4);
   Serial.print("Transfer Complete");
}

// ******************************** Setup and Loop **************************
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
