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

void buildBlock(string& input, int blockNumber, char* block)
{
    int checkSum = 0;

    block[0] = 0x01;
    block[1] = blockNumber;
    block[2] = 255 - blockNumber;

    for (int i = 0; i < dataSize; i++) {
        char b;

        if (i < input.size()) 
        {
            // Noch Daten im String 
            b = input[i];

        }
        else 
        {
            // String ist zu Ende auffüllen mit ETX
            b = 0x03;

        }

        block[i + 3] = b;


        checkSum += b;
    }

    block[blockSize - 1] = static_cast<char>(checkSum % 256);
    cout << "Check Sum bei verpacken: " << checkSum % 256 << endl;
    cout << "Check Sum bei verpacken: " << static_cast<char>(checkSum % 256) << endl;
}


int main()
{
    Serial* com = new Serial("COM1", 9600, 8, ONESTOPBIT, NOPARITY);
    char c = ' ';

    string input = "Hallo Welt!. Das ist ein neuer Test";
    


    if (!com->open())
    {
        cout << "Fehler beim Öffnen" << endl;
        return -1;
    }
    else
    {
        cout << "Warten auf NAK von Empfaenger fuer start" << endl;

        c = com->read();
        


        if (c != 0x15) 
        {
            cout << "Kein NAK erhalten, Abbruch" << endl;
            return -2;
        }



        cout << "NAK empfangen -> senden" << endl;




        int numBlocks = (static_cast<int>(input.size()) + dataSize - 1) / dataSize; //Anzahl der Benötigten Blöcke

        for (int n = 0; n < numBlocks; n++) {
            // Teilstring herausziehen
            string partOfInput = input.substr(n * dataSize, dataSize);

            // Block bauen
            char block[blockSize];
            buildBlock(partOfInput, n + 1, block);

            // Block senden
            com->write(block, blockSize);
            cout << "Block " << (n + 1) << " gesendet" << endl;

            // Auf Antwort warten
            char response = com->read();
            if (response == 0x06) 
            { 
                // ACK
                cout << "ACK empfangen" << endl;
            }
            else if (response == 0x15) 
            { 
                // NAK
                cout << "NAK empfangen, Block wiederholen" << endl;
                n--; // Schleifenindex zurücksetzen → Block nochmal senden
            }
            else 
            {
                com->write(0x18);
                cout << "Unerwartetes Zeichen, Abbruch" << endl;
                return -3;
            }
        }

        // Ende der Übertragung
       
        com->write(0x04);
        cout << "EOT gesendet" << endl;

        // ACK auf EOT erwarten
        c = com->read();
        if (c == 0x06)
        {
            cout << "Übertragung abgeschlossen." << endl;
        }

        return 0;





        
    }


}
