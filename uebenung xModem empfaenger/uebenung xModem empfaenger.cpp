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
        checkSum += block[i];
    }

    checkSum = checkSum % 256;

    if (checkSum != block[blockSize])
    {
        return false;
    }

    return true;

}


int main()
{
    Serial* com = new Serial("COM2", 9600, 8, ONESTOPBIT, NOPARITY);
    char c = ' ';

    string buffer;
   



    if (!com->open())
    {
        cout << "Fehler beim Öffnen" << endl;
        return -1;
    }
    else
    {
        com->write(0x15);

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

            if (!checkValidBlock(block))
            {
                cout << "Kein Valider Block, sende NAK" << endl;
                com->write(0x15);
            }
            else
            {
                cout << "Block richtig empfangen" << endl;
                cout << "Es wurde: ";
                com->write(0x06);
                for (int i = 3; i < dataSize; i++)
                {
                    buffer += block[i];
                    cout << block[i];
                }

                cout << " gesendet" << endl;
            }



        }

        cout << "Es wurde: " << buffer << " gesendet" << endl;


        

        return 0;
    }


}
