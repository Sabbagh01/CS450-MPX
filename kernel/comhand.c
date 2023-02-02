#include <comhand.h>

#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include "syscalls.h"


// colors, note that how a terminal represents colors will not be constant
// i.e, terminals may be configured with different color palettes
// color strings below thus represent colors on most terminals using their default palette
#define resetColor  "\033[0m"
#define yellowColor "\033[0;33m"
#define whiteColor  "\033[0;37m"
#define redColor    "\033[0;31m"
#define purpleColor "\033[0;36m"
#define blueColor   "\033[0;34m"

static const struct { 
    char* colorbytes; 
    size_t sz;
} serial_text_colors[] = {
    { STR_BUF(resetColor) },
    { STR_BUF(yellowColor) },
    { STR_BUF(whiteColor) },
    { STR_BUF(redColor) },
    { STR_BUF(purpleColor) },
    { STR_BUF(blueColor) }
};

enum Color {
    Reset = 0x00,
    Yellow,
    White,
    Red,
    Purple,
    Blue
};

// simplification for serial terminal color setting.
void setTerminalColor(enum Color color) {
    write(COM1, serial_text_colors[color].colorbytes, serial_text_colors[color].sz);
}

void setTimeCommand() {
    size_t input_len;
    char input_buffer[30] = { 0 };
    int hour, minute, second;

    static const char error_msg[] = "\r\nCould not parse, please re-enter time:";
    
    while(1) {
        setTerminalColor(Yellow);

        static const char hour_msg[] = "\r\nEnter the hour (0-23):\r\n";
        write(COM1, STR_BUF(hour_msg));

        setTerminalColor(White);
        input_len = read(COM1, BUF(input_buffer));
        hour = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
        
        if ( (hour < 24) && (hour >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    while(1) {
        setTerminalColor(Yellow);

        static const char minute_msg[] = "\r\nEnter the minute (0-59):\r\n";
        write(COM1, STR_BUF(minute_msg));

        setTerminalColor(White);
        input_len = read(COM1, BUF(input_buffer));
        minute = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
         
        if ( (minute < 60) && (minute >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    while (1) {
        setTerminalColor(Yellow);

        static const char second_msg[] = "\r\nEnter the second (0-59):\r\n";
        write(COM1, STR_BUF(second_msg));

        setTerminalColor(White);
        input_len = read(COM1, BUF(input_buffer));
        second = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
        
        if ( (second < 60) && (second >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    write(COM1, STR_BUF("\r\n"));

    setTime(hour, minute, second);
}

void getTimeCommand() {
    setTerminalColor(Yellow);
    static const char day_msg[] = "\r\nThe time is:\r\n";
    write(COM1, STR_BUF(day_msg));
    
    getTime();
}

void setDateCommand() {
    size_t input_len;
    char input_buffer[30] = { 0 };
    int day, month, year;
      
    static const char error_msg[] = "\r\nCould not parse, please re-enter time:";
    
    while(1) {
        setTerminalColor(Yellow);
        static const char month_msg[] = "\r\nEnter the month (1-12):\r\n";
        write(COM1, STR_BUF(month_msg));
        
        setTerminalColor(White);       
        input_len = read(COM1, BUF(input_buffer));
        month = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( (month <= 12) && (month >= 1) ) {
            break;
        }

        setTerminalColor(Red);       
        write(COM1, STR_BUF(error_msg));    
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char year_msg[] = "\r\nEnter the last two digits of the year (0-99):\r\n";
        write(COM1, STR_BUF(year_msg));
        
        setTerminalColor(White);       
        input_len = read(COM1, BUF(input_buffer));
        year = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( (year < 100) && (year >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char day_msg[] = "\r\nEnter the day of the month:\r\n";
        write(COM1, STR_BUF(day_msg));
        
        setTerminalColor(White);
        input_len = read(COM1, BUF(input_buffer));
        day = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( ((day <= month_info[month - 1].lastday) || 
             ((month == 2) && (year % 4 == 0) && (day <= 29))) && (day >= 1) ) {
            break;
        }
        
        setTerminalColor(Red); 
        write(COM1, STR_BUF(error_msg));
    }
    write(COM1, STR_BUF("\r\n"));
    
    setDate (day, month, year);
}

void getDateCommand() {
    setTerminalColor(Yellow);
    static const char day_msg[] = "\r\nThe date is:\r\n";
    write(COM1, STR_BUF(day_msg));
    
    getDate();
}

void versionCommand() {
    setTerminalColor(White);
    static const char ver_msg[] = "\r\nMPX vR1.\r\nCompiled ";
    write(COM1, STR_BUF(ver_msg));
   
    write(COM1, STR_BUF(__DATE__));
    write(COM1, STR_BUF("\r\n"));
}

/**
* @ param char command - the string that is used to determine which command information to display
* @ returns - none
* @ brief - Displays information for a certain user command if the string given matches the number id of a command or the exact name of the command unless all is specified in which all commands are displayed. Otherwise will display that a command was not found.
*/
void helpCommand() {
	//gets a message from the user to use or not use if blank as command parameter
	setTerminalColor(Yellow);
    static const char help_msg[] = "\r\nEnter the command name or id to display a specific command\r\n"
                                  "Enter [all] to display all commands\r\n";
	write(COM1, STR_BUF(help_msg));
    setTerminalColor(White);
	char command[120] = { 0 };
	read(COM1, BUF(command));
	//flag to display error message or not
	char recognized = 0;
    // cache 'all' comparison
    char printall = (strcmp("all", command) == 0);
	
    //version command (ID 6)
	if (strcmp("Version", command) == 0 || strcmp("6", command) == 0 || printall) {
		//version command
		write(COM1, STR_BUF(
            "\r\nVersion\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\tno output\r\n"
		    "\tDescription:\r\n"
		    "\tprints the current version of MPX and the compilation date")
        );
		recognized = 1;
	}
	// help command (ID 1)
	if (strcmp("Help", command) == 0 || strcmp("1", command) == 0 || printall) {
		write(COM1, STR_BUF(
            "\r\nHelp\r\n"
		    "\tInput:\r\n"
		    "\tcommand - the command name or its corrosponding number or all\r\n"
		    "\tOutput:\r\n"
		    "\tno return value\r\n"
		    "\tDescription:\r\n"
		    "\tprints all available user commands as well as details about the commands if\r\n"
            "\tall is used as the parameter\r\n"
            "\tprints the user command with details if a command or its id is specified")
        );
		recognized = 1;
	}
	//shutdown command (ID 7)
	if (strcmp("Shut Down", command) == 0 || strcmp("7", command) == 0 || printall) {
		write(COM1, STR_BUF(
            "\r\nShut Down\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\tno output value\r\n"
		    "\tDescription:\r\n"
		    "\tshuts down the machine after confirmation is given by entering 1")
        );
		recognized = 1;
	}
	//get date command (ID 5)
    else if (strcmp("Get Date", command) == 0 || strcmp("5", command) == 0 || printall) {
		write(COM1, STR_BUF(
            "\r\nGet Date\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\toutputs the system date\r\n"
		    "\tDescription:\r\n"
		    "\toutputs the date stored on the system data registers in month, day, year\r\n"
            "\tformat")
        );
		recognized = 1;
	}
	//set date command (ID 4)
	if (strcmp("Set Date", command) == 0 || strcmp("4", command) == 0 || printall) {
		write(COM1, STR_BUF(
            "\r\nSet Date\r\n"
		    "\tInput:\r\n"
		    "\tday - the number to set the system day to\r\n"
		    "\tdate range depends on month 1 is always the minimum while the maximum could be 28, 29, 30, 31\r\n"
		    "\tmonth - the number to set the system month to (1-12)\r\n"
		    "\tyear - the number to set the system year to (2 digits)\r\n"
		    "\tOutput:\r\n"
		    "\tno output\r\n"
	        "\tDescription:\r\n"
		    "\tsets the system date by writing to the system data register\r\n"
		    "\tit will prompt the user to enter the month XX, then day XX, year 20XX")
        );
		recognized = 1;
	}
	//get time command (ID 3)
	if (strcmp("Get Time", command) == 0 || strcmp("3", command) == 0 || printall) {
		write(COM1, STR_BUF(
            "\r\nGet Time\r\n"
		    "\tInput:\r\n"
		    "\tno inputs\r\n"
		    "\tOutput:\r\n"
		    "\toutputs the system time in month, day, year format\r\n"
		    "\tDescription:\r\n"
		    "\tOutputs the time stored on the system data registers")
        );
		recognized = 1;
	}
	//set time command (ID 2)
	if (strcmp("Set Time", command) == 0 || strcmp("2", command) == 0 || printall) {
		write(COM1, STR_BUF(
            "\r\nSet Time\r\n"
		    "\tInput:\r\n"
		    "\thour - the hour to be written into data (0-23)\r\n"
	        "\tminute - the minute to be written into data (0-59)\r\n"
		    "\tsecond - the second to be written into data (0-59)\r\n"
		    "\tOutput:\r\n"
		    "\tno output\r\n"
		    "\tDescription:\r\n"
		    "\tSets the system time.\r\n"
		    "\tThe user will be prompted to enter the hour, minute, then second specifications")
        );
		recognized = 1;
	}
	if (recognized == 0) {
		write(COM1, STR_BUF("\r\nCommand name or id not recognized"));
	}
    write(COM1, STR_BUF("\r\n"));
    
    return;
}

int shutdownCommand() {
    setTerminalColor(Red);
    static const char shutdown_msg[] = "\r\nAre you sure you would like to shut down?\r\n"
                                      "Enter 1 to confirm, enter another key to go back to menu:\r\n";
    write(COM1, STR_BUF(shutdown_msg));

    char shutdownRead[30] = { 0 };
    read(COM1, BUF(shutdownRead));
    
    setTerminalColor(White);
    if (strcmp(shutdownRead, "1") == 0){
        static const char sdexec_msg[] = "\r\nShutting down now.\r\n";
        write(COM1, STR_BUF(sdexec_msg));
        return 0;
    }
    static const char sdcancel_msg[] = "\r\nShut down cancelled.\r\n";
    write(COM1, STR_BUF(sdcancel_msg));
    return 1;
}

void comhand() { 
    static const char menu_welcome_msg[] = "Welcome to 5x5 MPX.\r\n";
    static const char menu_options[] = "Please select an option by choosing a number.\r\n"
                                       "1) Help        2) Set Time    3) Get Time    4) Set Date\r\n"
                                       "5) Get Date    6) Version     7) Shut Down\r\n"
                                       "Enter number of choice:\r\n";
    
    setTerminalColor(Blue);
    write(COM1, STR_BUF(menu_welcome_msg));
    
    while (1) {
        setTerminalColor(Purple);
        //display the menu
        write(COM1, STR_BUF(menu_options));
        
        setTerminalColor(White);
        // Get the input and call the corresponding function
        char user_input_buffer[100] = { 0 };
        read(COM1, BUF(user_input_buffer));
        
        // these could be substituted for a switch case block because comparisons
        // for a single char is simple, but in case of further changes, strcmp
        // works fine.
        if (strcmp(user_input_buffer,"1") == 0) {
            helpCommand();
        } else if (strcmp(user_input_buffer,"2") == 0) {
            setTimeCommand();
        } else if (strcmp(user_input_buffer,"3") == 0) {
            getTimeCommand();
        } else if (strcmp(user_input_buffer,"4") == 0) {
            setDateCommand();
        } else if (strcmp(user_input_buffer,"5") == 0) {
            getDateCommand();
        } else if (strcmp(user_input_buffer,"6") == 0) {
            versionCommand();
        } else if (strcmp(user_input_buffer,"7") == 0) {
            if (!shutdownCommand()) {
                break;
            }
        } else {
            setTerminalColor(Red);
            static const char invalidOption[] = "\r\nPlease select a valid option.\r\n";
            write(COM1, STR_BUF(invalidOption));
        }
    }
}

