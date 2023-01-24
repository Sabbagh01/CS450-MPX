#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <comhand.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>




void version();
void setDate1();
void help();

void comhand(){
    int x = 1;
    
    while(x==1){

        //menu display Write all messages
        const char *Message1 = "Welcome to 5x5 MPX. Please select an option by choosing a number.\n";
        size_t lenMes1 = strlen(Message1);
        sys_req(WRITE, COM1, Message1, lenMes1);

        const char *Message2 = "1) Help.        2) Set Time.    3) Get Time.    4) Set Date \n";
        size_t lenMes2 = strlen(Message2);
        sys_req(WRITE, COM1,Message2, lenMes2);

        const char *Message3 = "5) Get Date.    6) Version.     7) ShutDown.\n";
        size_t lenMes3 = strlen(Message3);
        sys_req(WRITE, COM1, Message3, lenMes3);

        const char *Message4 = "Enter number of choice:\n";
        size_t lenMes4 = strlen(Message4);
        sys_req(WRITE, COM1, Message4, lenMes4);

        // Get the input and call corresponding function

        char mainChoice[100];
        sys_req(READ, COM1, mainChoice, 100);

        if (strcmp(mainChoice, "1") == 0){
            help();
        }
        
        if (strcmp(mainChoice, "4") == 0){
            setDate1();
        }
        x = 0;
    }
}

void setDate1(){
    const char *dayMsg = "\nEnter the day as TWO numbers only: \n";
    size_t lengthMsg1 = strlen(dayMsg);
    sys_req(WRITE, COM1, dayMsg, lengthMsg1);
    char dayGet[100];
    sys_req(READ, COM1, dayGet, 100);

    const char *monthMsg = "Enter the month as TWO numbers only: \n";
    size_t lengthMsg2 = strlen(dayMsg);
    sys_req(WRITE, COM1, monthMsg, lengthMsg2);
    char monthGet[100];
    sys_req(READ, COM1, monthGet, 100);

    const char *yearMsg = "Enter the year as FOUR numbers only: \n";
    size_t lengthMsg3 = strlen(yearMsg);
    sys_req(WRITE, COM1, yearMsg, lengthMsg3);
    char yearGet[100];
    sys_req(READ, COM1, yearGet, 100);

    setDate(atoi(dayGet), atoi(monthGet), atoi(yearGet));

}

void version(){
    const char* Version1 = "Version Data\n";
    size_t lenVer1 = strlen(Version1);
    sys_req(WRITE, COM1, Version1 ,lenVer1);

    const char* Version2 = "The current version of MPX is R1.\nDate: \n";
    size_t lenVer2 = strlen(Version2);
    sys_req(WRITE, COM1, Version2, lenVer2);
    // GET DATE!

}

void help(){
    sys_req(WRITE, COM1, "1) Help - Provides usage instructions for all commands\n", 100);
    sys_req(WRITE, COM1, "2) Set Time - Sets the time\n", 100);
    sys_req(WRITE, COM1, "3) Get Time - Returns the time\n",100);
    sys_req(WRITE, COM1, "2) Set Time - Sets the time\n",100);
    sys_req(WRITE, COM1, "2) Set Time - Sets the time\n",100);
    sys_req(WRITE, COM1, "2) Set Time - Sets the time\n",100);
}