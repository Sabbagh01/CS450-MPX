#include <comhand.h>

#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>


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
    { resetColor, sizeof (resetColor) - 1 },
    { yellowColor, sizeof (yellowColor) - 1 },
    { whiteColor, sizeof (whiteColor) - 1 },
    { redColor, sizeof (redColor) - 1 },
    { purpleColor, sizeof (purpleColor) - 1 },
    { blueColor, sizeof (blueColor) - 1 }
};

enum Color {
    Reset = 0x00,
    Yellow,
    White,
    Red,
    Purple,
    Blue
};

void setTerminalColor(enum Color color) {
    sys_req (WRITE, COM1, serial_text_colors[color].colorbytes, serial_text_colors[color].sz);
}

void setTimeCommand() {
    size_t input_len;
    char input_buffer[30] = { 0 };
    int hour, minute, second;

    static const char error_msg[] = "\r\nCould not parse, please re-enter time:";
    
    while(1) {
        setTerminalColor(Yellow);

        static const char hour_msg[] = "\r\nEnter the hour (0-23):\r\n";
        sys_req(WRITE, COM1, hour_msg, sizeof(hour_msg));

        setTerminalColor(White);
        input_len = sys_req(READ, COM1, input_buffer, sizeof(input_buffer));
        hour = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
        
        if ( (hour < 24) && (hour >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));
    }
    while(1) {
        setTerminalColor(Yellow);

        static const char minute_msg[] = "\r\nEnter the minute (0-59):\r\n";
        sys_req(WRITE, COM1, minute_msg, sizeof(minute_msg));

        setTerminalColor(White);
        input_len = sys_req(READ, COM1, input_buffer, sizeof(input_buffer));
        minute = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
        
        if ( (minute < 60) && (minute >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));
    }
    while (1) {
        setTerminalColor(Yellow);

        static const char second_msg[] = "\r\nEnter the second (0-59):\r\n";
        sys_req(WRITE, COM1, second_msg, sizeof(second_msg));

        setTerminalColor(White);
        input_len = sys_req(READ, COM1, input_buffer, sizeof(input_buffer));
        second = atoi(input_buffer);
        
        if ( (second < 60) && (second >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));
    }
    sys_req(WRITE, COM1, "\r\n", 2);

    setTime(hour, minute, second);
}

void getTimeCommand() {
    setTerminalColor(Yellow);
    static const char day_msg[] = "\r\nThe time is:\r\n";
    sys_req(WRITE, COM1, day_msg, sizeof (day_msg));
    
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
        sys_req(WRITE, COM1, month_msg, sizeof (month_msg));
        
        setTerminalColor(White);       
        input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
        month = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( (month <= 12) && (month >= 1) ) {
            break;
        }

        setTerminalColor(Red);       
        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));    
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char year_msg[] = "\r\nEnter the last two digits of the year (0-99):\r\n";
        sys_req(WRITE, COM1, year_msg, sizeof (year_msg));
        
        setTerminalColor(White);       
        input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
        year = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( (year < 100) && (year >= 0) ) {
            break;
        }
        
        setTerminalColor(Red);
        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char day_msg[] = "\r\nEnter the day of the month:\r\n";
        sys_req(WRITE, COM1, day_msg, sizeof (day_msg));
        
        setTerminalColor(White);
        input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
        day = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( ((day <= month_info[month - 1].lastday) || 
             ((month == 2) && (year % 4 == 0) && (day <= 29))) && (day >= 1) ) {
            break;
        }
        
        setTerminalColor(Red); 
        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));
    }
    sys_req(WRITE, COM1, "\r\n", 2);
    
    setDate (day, month, year);
}

void getDateCommand() {
    setTerminalColor(Yellow);
    static const char day_msg[] = "\r\nThe date is:\r\n";
    sys_req(WRITE, COM1, day_msg, sizeof (day_msg));
    
    getDate();
}

void versionCommand() {
    setTerminalColor(White);
    static const char ver_msg[] = "\r\nMPX vR1.\r\nCompiled ";
    sys_req(WRITE, COM1, ver_msg, sizeof (ver_msg));
   
    sys_req(WRITE, COM1, __DATE__, sizeof (__DATE__));
    sys_req(WRITE, COM1, "\r\n", 2);
}

//helper command to print messages onto the device
static void log(device dev, const char *msg)
{
	sys_req(WRITE, dev, msg, strlen(msg));
}

/**
* @ param char command - the string that is used to determine which command information to display
* @ returns - none
* @ brief - Displays information for a certain user command if the string given matches the number id of a command or the exact name of the command unless all is specified in which all commands are displayed. Otherwise will display that a command was not found.
*
*/
void helpCommand() {
	//gets a message from the user to use or not use if blank as command parameter
	//yellow color
	setTerminalColor(Yellow);
        const char *helpMsg = "\r\nEnter the command name or id to display a specific command\r\nEnter [all] to display all commands\r\n";
        size_t lenHelpMsg = strlen(helpMsg);
	//enter optional parameter
	sys_req(WRITE, COM1, helpMsg, lenHelpMsg);
	//white color
    	setTerminalColor(White);
	char command[120] = "";
	sys_req(READ, COM1, command, 120);
	//flag to display error message or not
	int recognized = 0;
	//id 6
	//version command
	if (strcmp("version", command) == 0 || strcmp("6", command) == 0 || strcmp("all", command) == 0) {
		//version command
		log(COM1, "\nversion\r\n\0");
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\tno input parameters\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\tno output\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\tprints the current version of MPX and the compilation date\n\r\n\0");
		recognized = 1;
	}
	//id 1
	//help command
	if (strcmp("Help", command) == 0 || strcmp("1", command) == 0 || strcmp("all", command) == 0) {
		log(COM1, "\r\nHelp\r\n\0");git
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\tcommand - the command name or its corrosponding number or all\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\tno return value\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\tprints all available user commands as well as details about the commands if\n\tall is used as the parameter\r\n\tprints the user command with details if a command or its id is specified\n\r\n\0");
		recognized = 1;
	}
	//id 7
	//shutdown command
	if (strcmp("Shut Down", command) == 0 || strcmp("7", command) == 0 || strcmp("all", command) == 0) {
		log(COM1, "\r\nShut Down\r\n\0");
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\tno input parameters\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\tno output value\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\tshuts down the machine after confirmation is given by entering 1\n\r\n\0");
		recognized = 1;
	}
	//id 5
	//get date command
	if (strcmp("Get Date", command) == 0 || strcmp("5", command) == 0 || strcmp("all", command) == 0) {
		//get date command
		log(COM1, "\r\nGet Date\r\n\0");
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\tno input parameters\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\toutputs the system date\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\toutputs the date stored on the system data registers in day, month, year\n\tformat\n\r\n\0");
		recognized = 1;
	}
	//id 4
	//set date command
	if (strcmp("Set Date", command) == 0 || strcmp("4", command) == 0 || strcmp("all", command) == 0) {
		log(COM1, "\r\nSet Date\r\n\0");
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\tday - the number to set the system day to\r\n\0");
		log(COM1, "\tdate range depends on month 1 is always the minimum while the maximum could be 28, 29, 30, 31\r\n\0");
		log(COM1, "\tmonth - the number to set the system month to (1-12)\r\n\0");
		log(COM1, "\tyear - the number to set the system year to (2 digits)\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\tno output\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\tsets the system date by writing to the system data register\r\n\0");
		log(COM1, "\tit will prompt the user to enter the month XX, year 20XX, then day 20XX\n\r\n\0");
		recognized = 1;
	}
	//id 3
	//get time command
	if (strcmp("Get Time", command) == 0 || strcmp("3", command) == 0 || strcmp("all", command) == 0) {
		log(COM1, "\r\nGet Time\r\n\0");
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\tno inputs\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\toutputs the system time in day, month, year format\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\toutputs the time stored on the system data registers\n\r\n\0");
		recognized = 1;
	}
	//id 2
	if (strcmp("Set Time", command) == 0 || strcmp("2", command) == 0 || strcmp("all", command) == 0) {
		//set time command
		log(COM1, "\r\nSet Time\r\n\0");
		log(COM1, "\tInput:\r\n\0");
		log(COM1, "\thour - the hour to be written into data (0-23)\r\n\0");
		log(COM1, "\tminute - the minute to be written into data (0-59)\r\n\0");
		log(COM1, "\tsecond - the second to be written into data (0-59)\r\n\0");
		log(COM1, "\tOutput:\r\n\0");
		log(COM1, "\tno output\r\n\0");
		log(COM1, "\tDescription:\r\n\0");
		log(COM1, "\tsets the system time by writing to the system data register,\n\tthe time is in UTC\r\n\0");
		log(COM1, "\tit will prompt the user to enter the hour, minute, then second specifications\n\r\n\0");
		recognized = 1;
	}
	if (recognized == 0) {
		log(COM1, "\r\ncommand name or id not recognized\r\n\0");
	}
}

int shutdownCommand() {
    setTerminalColor(White);
    static const char shutdownmsg[] = "\r\nAre you sure you would like to shut down?\r\n"
                                      "Enter 1 to confirm, enter another key to go back to menu:\r\n";
    sys_req(WRITE, COM1, shutdownmsg, sizeof(shutdownmsg));

    char shutdownRead[30] = { 0 };
    sys_req(READ, COM1, shutdownRead, sizeof(shutdownRead));
    
    setTerminalColor(White);
    if (strcmp(shutdownRead, "1") == 0){
        static const char shuttingdown[] = "\r\nShutting down now.\r\n";
        sys_req(WRITE, COM1, shuttingdown, sizeof(shuttingdown));
        return 0;
    }
    static const char sdcancel[] = "\r\nShut down cancelled.\r\n";
    sys_req(WRITE, COM1, sdcancel, sizeof(sdcancel));
    return 1;
}

void comhand() { 
    static const char menu_welcome_msg[] = "Welcome to 5x5 MPX.\r\n";
    static const char menu_options[] = "Please select an option by choosing a number.\r\n"
                                       "1) Help        2) Set Time    3) Get Time    4) Set Date\r\n"
                                       "5) Get Date    6) Version     7) Shut Down\r\n"
                                       "Enter number of choice:\r\n";
    
    setTerminalColor(Blue);
    sys_req(WRITE, COM1, menu_welcome_msg, sizeof (menu_welcome_msg));
    
    while (1) {
        setTerminalColor(Purple);
        //display the menu
        sys_req(WRITE, COM1, menu_options, sizeof (menu_options));
        
        setTerminalColor(White);
        // Get the input and call the corresponding function
        char user_input_buffer[100] = { 0 };
        sys_req(READ, COM1, user_input_buffer, sizeof (user_input_buffer));
        
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
            sys_req(WRITE, COM1, invalidOption, sizeof(invalidOption));
        }
    }
}

