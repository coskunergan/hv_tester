#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <bits/stdc++.h>

char CheckCommPacket[] = {0x7b, 0x06, 0x00, 0x00, 0x06, 0x7d};
char TestStopPacket[] = {0x7b, 0x07, 0x00, 0x02, 0x00, 0x09, 0x7d};
char FileSetPacket[] = {0x7b, 0x08, 0x00, 0x0b, 0x00, 0x00, 0x13, 0x7d};
char SuccesPacket[] = {0x7b, 0x06, 0x00, 0x01, 0x07, 0x7d};

HANDLE hComm;                          // Handle to the Serial port
char   ComPortName[] = "\\\\.\\COM10"; // Name of the Serial port(May Change) to be opened,
BOOL   Status;
DWORD NoBytesRead;                     // Bytes read by ReadFile()
DWORD dwEventMask;                     // Event mask to trigger
COMMTIMEOUTS timeouts = { 0 };
char  TempChar;                        // Temperory Character
char  SerialBuffer[256];               // Buffer Containing Rxe
int i = 0;
int file_no = 0;

/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
int ComSendPacket(char *lpBuffer)
{
    DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
    DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port

    dNoOFBytestoWrite = sizeof(lpBuffer); // Calculating the no of bytes to write into the port

    Status = WriteFile(hComm,               // Handle to the Serialport
                       lpBuffer,            // Data to be written to the port
                       dNoOFBytestoWrite,   // No of bytes to write into the port
                       &dNoOfBytesWritten,  // No of bytes written to the port
                       NULL);

    if(Status == FALSE)
    {
        printf("\n\n   Error %d in Writing to Serial Port", GetLastError());
        return 13;
    }

    Status = SetCommMask(hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

    if(Status == FALSE)
    {
        printf("\n\n    Error! in Setting CommMask");
        return 17;
    }
    else
    {
        printf("\n\n    Setting CommMask successfull");
    }

    printf("\n\n    Waiting for Data Reception");

    SerialBuffer[0] = 0;
    i = 0;

    do
    {
        Status = ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
        SerialBuffer[i] = TempChar;
        i++;
    }
    while(NoBytesRead > 0);

    if(memcmp(SerialBuffer, SuccesPacket, sizeof(SuccesPacket)) == 0)
    {
        return 0;
    }
    return 19;
}
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
int ComInit(void)
{
    /*----------------------------------- Opening the Serial Port --------------------------------------------*/
    hComm = CreateFile(ComPortName,                        // Name of the Port to be Opened
                       GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
                       0,                                 // No Sharing, ports cant be shared
                       NULL,                              // No Security
                       OPEN_EXISTING,                     // Open existing port only
                       0,                                 // Non Overlapped I/O
                       NULL);                             // Null for Comm Devices

    if(hComm == INVALID_HANDLE_VALUE)
    {
        printf("\n   Error! - Port %s can't be opened", ComPortName);
        return 3;
    }
    else
    {
        printf("\n   Port %s Opened\n ", ComPortName);
    }

    DCB dcbSerialParams = { 0 };                        // Initializing DCB structure
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    Status = GetCommState(hComm, &dcbSerialParams);     //retreives  the current settings

    if(Status == FALSE)
    {
        printf("\n   Error! in GetCommState()");
        return 5;
    }

    dcbSerialParams.BaudRate = CBR_9600;      // Setting BaudRate = 9600
    dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
    dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
    dcbSerialParams.Parity   = NOPARITY;      // Setting Parity = None

    Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB

    if(Status == FALSE)
    {
        printf("\n   Error! in Setting DCB Structure");
        return 7;
    }
    else
    {
        printf("\n   Setting DCB Structure Successfull\n");
        printf("\n       Baudrate = %d", dcbSerialParams.BaudRate);
        printf("\n       ByteSize = %d", dcbSerialParams.ByteSize);
        printf("\n       StopBits = %d", dcbSerialParams.StopBits);
        printf("\n       Parity   = %d", dcbSerialParams.Parity);
    }

    timeouts.ReadIntervalTimeout         = 300;
    timeouts.ReadTotalTimeoutConstant    = 300;
    timeouts.ReadTotalTimeoutMultiplier  = 100;
    timeouts.WriteTotalTimeoutConstant   = 300;
    timeouts.WriteTotalTimeoutMultiplier = 100;

    if(SetCommTimeouts(hComm, &timeouts) == FALSE)
    {
        printf("\n   Error! in Setting Time Outs");
        return 11;
    }
    else
    {
        printf("\n\n   Setting Serial Port Timeouts Successfull");
    }
    return 0;
}
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
void ComDeInit(void)
{
    CloseHandle(hComm);
}
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
int main(int argc, char *argv[])
{
    printf("\r\n GTM Sender V1.0 \r\n\r\n Coskun ERGAN \r\n");

    FileSetPacket[4] = atoi(argv[1]);
    if(FileSetPacket[4] == 0)
    {
        return 1;
    }
    FileSetPacket[4]--;
    FileSetPacket[6] += FileSetPacket[4];
    
    int res = ComInit();

    res += ComSendPacket(TestStopPacket);

    res += ComSendPacket(TestStopPacket);

    res += ComSendPacket(FileSetPacket);

    ComDeInit();

    if(res == 0)
    {
        printf("\r\nALL Done.\r\n");
    }    

    printf("\r\nExit.\r\n");

    return res;
}
/**********************************************************************************************/
/**********************************************************************************************/
/**********************************************************************************************/
