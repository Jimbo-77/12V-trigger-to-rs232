

/*
  The purpose of this program is to power on and select the right source of a Cambridge Audio 851A amplifier
  I use the information from a 12V triggerOut. In my case from a Denon 4308.
  And the Power On order goes trough the RS232 port of a Cambridge 851A.
*/


#include <Wire.h>
#include "rgb_lcd.h"


rgb_lcd lcd;

// set the debug mode 
const bool DEBUG_MODE = false;
const bool LOOP_DEBUG_MODE = false;

// Set the analog input for the triggerOut
const int triggerInput = A0;

// Set divider voltage gain. 3 or 10
const int dividerGain = 3;

// Set trigger value limit
const float triggerLimit = 11 / dividerGain;

// Set const Power State
const bool OFF = false;
const bool ON = true;

// set const about the cycle
const bool DONE = true;
const bool UNDONE = false;

// Set const source selected
const int LINE1 = 1;
const int LINE2 = 2;
const int LINE3 = 3;
const int LINE4 = 4;
const int LINE5 = 5;
const int LINE6 = 6;
const int LINE7 = 7;

// Set const source for the HC
const int HCinput = LINE1;

// Set const RS232 commands
const String powerON = "#1,11,1\r";
const String powerOFF = "#1,11,0\r";
const String selectHCSource = "#1,01,\r";

// Set const RS232 replies
const String IAmPowerON = "#4,11,1";
const String IAmPowerOFF = "#4,11,0";

const String IAmOnSource1 = "#4,01,";
const String IAmOnSource2 = "#4,02,";
const String IAmOnSource3 = "#4,03,";
const String IAmOnSource4 = "#4,04,";
const String IAmOnSource5 = "#4,05,";
const String IAmOnSource6 = "#4,06,";
const String IAmOnSource7 = "#4,07,";
const String IAmOnSourceX = "#4,0";

const String SpeakerSelectX ="#4,21,";

const char SpeakerAselected = '0';
const char SpeakerABselected = '1';
const char SpeakerBselected = '2';

const String newVolumeSet = "#4,13,";

const String newLCDBrightnessSet = "#4,20,";

const char LCDBrightnessOFF = '0';
const char LCDBrightnessDIM = '1';
const char LCDBrightnessBRIGHT = '2';

const String newBalanceSet = "#4,17,0";

// Set const error messages replies
const String errorMsgX = "#5,0";
const String overloadError = "#5,01,";
const String DCOffsetError = "#5,02,";
const String overTempError = "#5,03,";
const String clippingError = "#5,04,";
const String mainFail = "#5,05,";
const String speakerFail = "#5,06,";
const String cmdGroupUnknown = "#5,07,";
const String cmdNumberUnknown = "#5,08,";
const String cmdDataError = "#5,09,";

const char overloadErrorNb = '1';
const char DCOffsetErrorNb = '2';
const char overTempErrorNb = '3';
const char clippingErrorNb = '4';
const char mainFailNb = '5';
const char speakerFailNb = '6';
const char cmdGroupUnknownNb = '7';
const char cmdNumberUnknownNb = '8';
const char cmdDataErrorNb = '9';

// Set const no reply message
const String noReply = "No reply";

// Set HC Power status
bool Ampli_powerState = OFF;

// Set the selected source
int sourceSelected = 0;
int sourceSelectedTmp;

// Set and initialize trigger value
float triggerValue = -1;
float oldTriggerValue;

// Set and initialize ampliReply
String ampliReply ="";

// Set and initialize the power cycle
bool cycle = UNDONE;

// Set and Initialize timestamp variable
unsigned long timestamp = 0;

// Initialize and set timestamp display variabel
unsigned long timestampLCD = 0;

void setup() {

  // Set the serial port for the computer communication
  Serial.begin(9600);
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB
  }
  
  // Set the serial port for the 851A communication. Connexion to uart port 3
  /*
  From RS232 851A documentation 
  All data consists of ASCII characters.
  Settings for RS232 com port are 9600 bps, N, 8, 1 with no handshake CR is carriage return, value 0x0D (13 decimal)
  Command data consists of 10 characters maximum
  */
  Serial3.begin(9600);
  while (!Serial3) {
    delay(10); // wait for serial port to connect. Needed for native USB
  }

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.print("Setup OK");

  delay(1000);
  lcd.clear();
  lcd.print("Let's go !");
  delay(1000);

}

void loop() {
  
  // See if we have to turn off the LCD
  if(millis() - timestampLCD > 30000)
  {
    lcd.noDisplay();
  }
  
  if(DEBUG_MODE)
    {
      // Delay to avoid log spam
      delay(500);
      Serial.println("");
      Serial.println("== loop == Begin the loop");
    }
  // See if the ampli has something new to say and update status
  ampliReply = readAmpliReply();
  ProcessReply(ampliReply, noReply);
  int readValue = analogRead(triggerInput);

  triggerValue = (float)readValue * ((float)5 / (float)1023);
  
  String triggerValueStr = String(triggerValue, 7);
  
  // Verify is the ampli is ON with the triggerValue
  if(triggerValue > triggerLimit && (!cycle) && (millis() - timestamp > 10000)) 
  {
    if(LOOP_DEBUG_MODE)
    {
      Serial.println("== loop == trigger value reached & cycle undone");
      Serial.println(String(cycle));
    }
    // Send Power On to the 851A if needed
    
    if(!Ampli_powerState)
    {
      if(LOOP_DEBUG_MODE)
      {
        Serial.println("== loop == power up ampl ");
      }
      
      timestamp = millis();
      Serial3.print(powerON);
      
    }
    
    
    // We let the programp looping for 3s and then we change the source input
    if(millis() - timestamp > 2000)
    {
      timestamp = millis();
      // Send change input to HC source if needed
      if(sourceSelected != HCinput)
      {
        Serial.println(selectHCSource);
        Serial3.print(selectHCSource);
      }
      
      cycle = DONE;
    }
  }
  else
  {
    // Verify if we have to power off the ampli (trigger out at 0 + ampli ON + HCinput selected + cycle done)
    if(triggerValue < triggerLimit && Ampli_powerState && sourceSelected == HCinput && cycle)
    {
      Serial3.print(powerOFF);
      
      cycle = UNDONE;
    }
    else
    {
      if(triggerValue < triggerLimit && Ampli_powerState && sourceSelected != HCinput && cycle)
      {
        cycle = UNDONE;
      }
    }
  }

}


// Function to process the reply and update the LCD
void ProcessReply(String reply, String expectedReply)
{
   if(DEBUG_MODE)
    {
      Serial.println("= ProcessReply = Begin ProcessReply function");
      Serial.println("= ProcessReply = Reply to process : "+ reply);
    }

  if(reply.indexOf(newVolumeSet) == 0)
  {
    if(DEBUG_MODE)
    {
      Serial.println("= ProcessReply = Volume set at ");
    }
    //lcd.print("Volume set at ");
    return;
  }
  else
  {
    if(DEBUG_MODE)
    {
      Serial.println("= ProcessReply = Not LCD changed match");
    }
    if(reply.indexOf(noReply) == 0)
    {
      if(DEBUG_MODE)
      {
        Serial.println("= ProcessReply = No Reply match");
      }
      lcd.setCursor(14, 1);
      lcd.print(":)");
      return;
    }
    else
    {
      lcd.display();
      timestampLCD = millis();
      
      if (reply == IAmPowerOFF)
      {
        if(DEBUG_MODE)
        {
          Serial.println("= ProcessReply = power Off match");
        }
        lcd.clear();
        lcd.print("Ampli pwrd off   ");
      }
      else
      {
        if (reply == IAmPowerON)
        {
          if(DEBUG_MODE)
          {
            Serial.println("= ProcessReply = power On match");
          }
          lcd.clear();
          lcd.print("Ampli pwrd on   ");
        }
        else
        { 
          if(DEBUG_MODE)
          {
            Serial.println("= ProcessReply = Not power On match");
          }
          if (reply.indexOf(IAmOnSourceX) == 0)
          {
            if(DEBUG_MODE)
            {
              Serial.println("= ProcessReply = Source changed match");
            }
            char sourceNbChar = reply.charAt(4);
            if(DEBUG_MODE)
            {
              Serial.println("= ProcessReply = sourceNbChar variable : " + String(sourceNbChar));
            }
            
            int sourceNum;
            
            int returnScan = sscanf(&sourceNbChar, "%d", &sourceNum);
            if(DEBUG_MODE)
              {
                Serial.println("= ProcessReply = Scanf return = " + String(returnScan));
                Serial.println("= ProcessReply = SourceNum variable = " + String(sourceNum));
              }
            if(sourceNum < 8)
            {
              if(DEBUG_MODE)
              {
                Serial.println("= ProcessReply = Source number < 8");
              }
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Src ");
              lcd.print(sourceNum);
              lcd.print(" selected");
              //delay(5000);
              return;
            }
            else
            {
              if(DEBUG_MODE)
              {
                Serial.println("= ProcessReply = Tape monitor match");
              }
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.println("Src Tape ");
              lcd.print("Monitor selected");
              
              delay(5000);
              return;
            }
          }
          else
          {
            if(DEBUG_MODE)
            {
              Serial.println("= ProcessReply = Not power On match");
            }
            if(reply.indexOf(SpeakerSelectX) == 0)
            {
              if(DEBUG_MODE)
              {
                Serial.println("= ProcessReply = Speaker select match");
              }
              lcd.setCursor(0,1);
              lcd.print("Speaker ");
              char speakerSelected = reply.charAt(6);
              
              switch(speakerSelected)
              {
                case SpeakerAselected :
                  lcd.print("A       ");
                  break;
                case SpeakerABselected :
                  lcd.print("AB      ");
                  break;
                case SpeakerBselected :
                  lcd.print("B       ");
                  break;
                default :
                  lcd.print("unknown ");
                  break;
              }
            }
            else
            {
              if(DEBUG_MODE)
              {
                Serial.println("= ProcessReply = Not speaker select match");
              }
              if(reply.indexOf(newLCDBrightnessSet) == 0)
              {
                if(DEBUG_MODE)
                {
                  Serial.println("= ProcessReply = new LCD Brightness");
                }
                char brghtLevel = reply.charAt(6);
                
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("LCD Brightness ");
                lcd.setCursor(0,1);
                lcd.print("changed to ");
                
                switch(brghtLevel)
                    {
                      case LCDBrightnessOFF :
                        lcd.print("off");
                        break;
                        
                      case LCDBrightnessDIM :
                        lcd.print("dim");
                        break;
                        
                      case LCDBrightnessBRIGHT :
                        lcd.print("bright");
                        break;
                        
                      default:
                        lcd.print("unknown");
                        break;
                    }
                // delay(2000);
                return;
              }
              else
              {
                if(DEBUG_MODE)
                {
                  Serial.println("= ProcessReply = Not reply match");
                }
                lcd.setCursor(0, 0);
                if (reply.indexOf(errorMsgX) == 0)
                {
                  char errorNb = reply.charAt(4);
                  switch(errorNb)
                  {
                    case overloadErrorNb :
                      lcd.print("Overload error");
                      break;
                    
                    case DCOffsetErrorNb :
                      lcd.print("DC offset error");
                      break;
                      
                    case overTempErrorNb :
                      lcd.print("Over temperature");
                      lcd.setCursor(0, 1);
                      lcd.print("error");
                      break;
                      
                    case clippingErrorNb :
                      lcd.print("Clipping error");
                      break;
                      
                    case mainFailNb :
                      lcd.print("Mains fail");
                      break;
                      
                    case speakerFailNb :
                      lcd.print("Speaker fail");
                      break;
                      
                    case cmdGroupUnknownNb :
                      lcd.print("Command group");
                      lcd.setCursor(0, 1);
                      lcd.print("unknown");
                      break;
                    
                    case cmdNumberUnknownNb :
                      lcd.print("Command number");
                      lcd.setCursor(0, 1);
                      lcd.print("unknown in grp");
                      break;
                      
                    case cmdDataErrorNb :
                      lcd.print("Command data");
                      lcd.setCursor(0, 1);
                      lcd.print("error");
                      break;
                    
                    default :
                      lcd.print("Ampli reply:");
                      lcd.setCursor(0, 1);
                      lcd.print(reply);
                      break;
                  }
                  delay(10000);
                  return;
                }
                else
                {
                  if(DEBUG_MODE)
                  {
                    Serial.println("= ProcessReply = Not error match");
                  }
                  lcd.setCursor(0, 1);
                  lcd.print(reply);
                  //delay(1000);
                  return;
                } 
              }
            }
          }
        }
      }
    }
  }
  return;
}

// Read the ampli's reply and update the status variable
String readAmpliReply()
{
  if(DEBUG_MODE)
    {
      Serial.println("= readAmpliReply = Begin ReadAmpliReply function");
    }
  
  if(Serial3.available() > 0)
  {
    if(DEBUG_MODE)
    {
      Serial.println("= readAmpliReply = Data available");
    }
    
    ampliReply = Serial3.readStringUntil('\r');
    
    if(DEBUG_MODE)
    {
      Serial.println("= readAmpliReply = Reply received : " + ampliReply);
    }
    if(ampliReply.indexOf(newVolumeSet) == 0)
    {
      Serial.println(ampliReply);
      return ampliReply;
    }
    else
    {
      if(ampliReply == IAmPowerON)
      {
        if(DEBUG_MODE)
        {
          Serial.println("= readAmpliReply = Power On Match");
        }
        else
        {
          Serial.println(ampliReply);
        }
        Ampli_powerState = ON;
        Serial.print("pwrState : ");
        Serial.println(Ampli_powerState);
        return IAmPowerON;
      }
      else
      {
        if(ampliReply == IAmPowerOFF)
        {
          if(DEBUG_MODE)
          {
            Serial.println("= readAmpliReply = Power Off Match");
          }
          else
          {
            Serial.println(ampliReply);
          }
          Ampli_powerState = OFF;
          Serial.print("pwrState : ");
          Serial.println(Ampli_powerState);
          // Time delay to wait a moment in case if the HC is still on and to avoid to power on the 851 right now
          delay(10000);
          return IAmPowerOFF;
        }
        else
        {
          if(ampliReply.indexOf(IAmOnSourceX) == 0)
          {
            if(DEBUG_MODE)
            {
              Serial.println("= readAmpliReply = Source changed Match");
              
            }
            char sourceNb = ampliReply.charAt(4);
            if(DEBUG_MODE)
            {
              Serial.println("= readAmpliReply = sourceNb variable = " + String(sourceNb));
            }
            sscanf(&sourceNb, "%d", &sourceSelected);
            
            if(sourceSelected < 8)
            {
              
              if(DEBUG_MODE)
              {
                Serial.println("= readAmpliReply = Source number < 8");
                Serial.println("= readAmpliReply = sourceSelected variable = " + String(sourceSelected));
              }
              return ampliReply;
            }
          }
          
          // Return the reply's ampli if there was no match
          else
          {
            
            if(DEBUG_MODE)
            {
              Serial.println("= readAmpliReply = No Match -> " + ampliReply);
            }
            else
            {
              Serial.println(ampliReply);
            }
            
            return ampliReply;
          }
        }
      }
    }
  }
  else
  {
    if(DEBUG_MODE)
          {
            Serial.println("= readAmpliReply = No data available");
          }
    return noReply;
  }
  
}


