/*
 * SRAM Read/Write Test Program
 * SRAM Chip Model No.: UM61256AK-15
 * Chip Info: 32K X 8 High Speed CMOS RAM (32,768 bytes MAX)
 * 
 * This program tests the Arduino's ability to interface to older generation SRAM chips.
 * The memory testing algorithm is not meant to be a rigorous test, but rater to show that
 * the Read/Write functions of the program work.
 */

#define OE_LOW()    digitalWrite(51, 0)   //Sets OE LOW
#define CE_LOW()    digitalWrite(52, 0)   //Sets CE LOW
#define WE_LOW()    digitalWrite(53, 0)   //Sets WE LOW
#define WE_HIGH()   digitalWrite(53, 1)   //Sets WE HIGH

void setup()     
{

  Serial.begin(9600); // Initiate serial port data transmission at 9600 baud

  /*
   * Control Line Setup
   * WE = Write Enable is Pin 53
   * OE = Output Enable is Pin 52
   * CE = Chip Enable is Pin 51
   */

   DDRB = 0b00000111; //Pin 53, 52, 51 set to output


  /*
   * Address Bus Setup 
   * Maximum number of lines: 15
   */

   DDRA = 0b11111111; //Pins 22-29 Set as Output (Lower Byte of Address)
   DDRC = 0b01111111; //Pins 37-31 Set as Output (Upper Byte of Address)

}

void loop() 
{

   /*
    * Memory Testing Algorithm
    * 1.) Sequence through addresses from 0x0000 to 0x7FFF (0 to 32767)
    * 2.) Write random data to that location
    * 3.) Read back data and confirm it is the same
    * 3a.) If data read back is same, memory test is passed
    * 3b.) If data read back is different, memory test failed
    * 4.) Repeat
    */

    
    uint16_t address;                               //Declare address variable
    uint8_t random_data;                            //Declare random data variable
    randomSeed(analogRead(A0));                     //Initialize random number generator using signal from pin AO
    
    for (address = 0; address < 0x8000; ++address)  //Start at 0, sequence through all (2^15) - 1 addresses
    {
      random_data = random(0x00, 0xFF);             //set random data variable equal to random number between 0x00 and 0xFF
      write_data(address, random_data);             //write random data to address
      if(read_data(address) == random_data)         //If data read from address is equal to random data generated...
      {
        /*
         * If data read from address is equal to random data generated,
         * Print Address and Data Written in Hexadecimal as well as memory status as "PASS"
         */
        Serial.print("Address:");
        Serial.print(" ");
        Serial.print(address, HEX);
        Serial.print(" ");
        Serial.print("Data Written:");
        Serial.print(" ");
        Serial.print(random_data, HEX);
        Serial.print(" ");
        Serial.print("Status: ");
        Serial.print("PASS");
        Serial.print("\n");
      }
      else
      {
        /*
         * If data read from address is NOT equal to random data generated,
         * Print Address and Data Written in Hexadecimal as well as memory status as "FAIL"
         */
        Serial.print("Address:");
        Serial.print(" ");
        Serial.print(address, HEX);
        Serial.print(" ");
        Serial.print("Data Written:");
        Serial.print(" ");
        Serial.print(random_data, HEX);
        Serial.print(" ");
        Serial.print("Status: ");
        Serial.print("FAIL");
        Serial.print("\n");
      }
    }

    delay(500);   //Delay 500 milliseconds to ensure all data is properly displayed from Serial Monitor
    exit(0);      //Exit program
}

/*
 * Write Data Function
 * Arguments: Address and Data to be written
 */

void write_data(uint16_t address, uint8_t data)
{
  /*
   * Write operation uses a WE (Write Enabled Controlled) Write Cycle.
   */
  OE_LOW();             //OE is continuously LOW
  CE_LOW();             //CE is continuously LOW
  WE_HIGH();            //WE starts off HIGH
  delay(1);             //Delay just to ensure signals stay HIGH/LOW long enough
  
  set_addr(address);    //Address applied first
  WE_LOW();             //WE goes from HIGH to LOW
  data_op('w', data);   //Data applied to data bus
  WE_HIGH();            //WE goes from LOW to HIGH

}

/*
 * Read Data Function
 * Arguments: Address to be used to obtain data
 */

uint8_t read_data(uint16_t address)
{

  /*
   * Read Operation uses "Read Cycle 1", see PDF documentation linked
   */
  WE_HIGH();     //WE set to HIGH at all times for Type 1 Read
  OE_LOW();      //OE set to LOW at all times for Type 1 Read
  CE_LOW();      //CE set to LOW at all times for Type 1 Read
  
  set_addr(address);        //Address applied first
  return (data_op('r', NULL));  //Read data operation is committed, data obtained is returned
  
}

/*
 * Set Address Function, Outputs the address through two pin registers
 * Arguments: Address to be outputted
 */

void set_addr (uint16_t address)
{

  PORTA = address & 0xff;         //Takes the first half of the address and sets it to PORTA (Lower Address Byte)
  PORTC = (address >> 8) & 0xff;  //Takes the second half of the address and sets it to PORTC (Upper Address Byte)
  
}

/*
 * Data Operation Function, controls the data IO from the Arduino
 * Arguments: Read/Write option and data to be written
 */

uint8_t data_op (char rw, uint8_t data)
{
  if(rw == 'w') 
  {
    /*
     * If RW option is set to char 'w' for WRITE
     */
    DDRL = 0xff;   //Set Data Direction on L register pins to OUTPUT
    PORTL = data;  //Output Data through L pin data register
    return PORTL;  //Return the data outputted as confirmation
  }
  else if (rw == 'r')
  {
   /*
    * If RW option is set to char 'r' for READ, read data
    */
    PORTL = 0x00;  //Erase any data still being held from previous write operation in L pin data register
    DDRL = 0x00;   //Set L register pins to INPUT
    return PINL;   //Return values read in the pin register
  }

}


