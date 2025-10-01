/*
  SENDER (Transmitter)

  Voraussetzungen:
    - Ein virtuelles COM-Port-Paar (z. B. mit HHD Virtual Serial Port Tool).
      Beispiel: COM1 <-> COM2, wobei der SENDER auf COM1, der EMPFÄNGER auf COM2 liegt.

  Ablauf:
    1) SENDER öffnet seinen Port.
    2) SENDER wartet, bis EMPFÄNGER DTR=true setzt (als DSR sichtbar).
    3) SENDER sendet einzelne Zeichen, Strings und einen Puffer.

  Hinweise:
    - DSR wird per Polling abgefragt; das ist nicht blockierend.
    - Write-Aufrufe können kurz blockieren (Timeout z. B. 100 ms in unserer Serial.cpp).
*/

#include "Serial.h"  // Serial-Klasse liegt im Ordner Serial/Serial  (siehe VS-Projektordner)
#include <iostream>
#include <algorithm>
#include <string>
#include <windows.h>

using namespace std;

// Übertragungs-Steuerzeichen (für spätere Protokoll-Erweiterungen nützlich)
char ETX = 0x03; // End of Text (Ende eines Datenblocks)
char ACK = 0x06; // Acknowledge (fehlerfreie Übertragung)
char NAK = 0x15; // No Acknowledge (fehlerhafte Übertragung)
char SOH = 0x01; // Start of Heading (Start eines Blockes)
char EOT = 0x04; // End of Transmission (Ende der Übertragung)
char CAN = 0x18; // Cancel (Abbruch der Übertragung)


const int dataSize = 5;
const int blockSize = 3 + dataSize + 1;



bool checkValidBlock(char* block)
{
    int checkSum = 0;

    for (int i = 0; i < dataSize; i++)
    {
        checkSum += block[3 + i];
    }

    checkSum = checkSum % 256;

    cout << "CheckSum errechnet: " << checkSum << endl;
    cout << "CheckSum empfangen: " << block[blockSize - 1] << endl;
    if (static_cast<char>(checkSum) == block[blockSize - 1] && block[0] == 0x01)    //static_cast<int>(block[blockSize])
    { 
        cout << "CheckSum ist valid" << endl;
        return true;
        
    }


    cout << "check sum ist nicht valide" << endl;
    Sleep(10000);
    return false;

}


int main()
{
    Serial* com = new Serial("COM2", 9600, 8, ONESTOPBIT, NOPARITY);
    char c = ' ';

    string buffer;
   



    if (!com->open())
    {
        cout << "Fehler beim oeffnen" << endl;
        return -1;
    }
    else
    {
        com->write(0x15); 
        cout << "NAK gesendet" << endl;

        while (true)
        {
            char block[blockSize];
            int numberOfBytes = com->read(block, blockSize); //Wie viele Bytes wurden tatsächlcih vom Empfaenger gelesebn


            if (numberOfBytes == 1 && block[0] == 0x04)
            {
                cout << "Ende der übertragung" << endl;
                com->write(0x06);

                break;
            }

            if (block[0] != SOH)
            {
                cout << "Start ist nicht SOH" << endl;
                com->write(0x015);
                continue;
            }

            


            if (!checkValidBlock(block))
            {
                cout << "Kein Valider Block, sende NAK" << endl;
                com->write(0x15);
                continue,
            }
            else
            {
                cout << "Block richtig empfangen" << endl;
                cout << "Es wurde: ";
                com->write(0x06);
                for (int i = 0; i < dataSize; i++)
                {
                    buffer += block[3 + i];
                    cout << block[3 + i];
                }

                cout << " gesendet" << endl;
            }



        }

        cout << "Es wurde: " << buffer << " gesendet" << endl;


        

        return 0;
    }


}
