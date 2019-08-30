long BAUD = 9600;

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
    long baud;
    void interpret();
    void serialCheck();
};

Oracle::Oracle(){
  
}

void Oracle::interpret(){
  // The first letter of the message is used to determine what function to call
  switch(_rx_msg[0]){

    case 'S':
      Serial.println("Set Voltage");
      break;
    case 'R':
      Serial.println("Read Data");
      break;
    case 'L':
      Serial.println("Load DAC");
      break;
    case 'X':
      Serial.println("Shutdown");
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

class Channel(){

  public:
    char address;
    long voltage_to_write;
    long voltage;
    long current;

  
}

Oracle orc;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD);
  Serial.println("Starting PMOD...");
}

void loop() {
  // put your main code here, to run repeatedly:
  orc.serialCheck();
}
