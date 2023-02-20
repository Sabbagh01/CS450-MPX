#include <mpx/comhand.h>

#include <mpx/io.h>
#include <mpx/serial.h>
#include <mpx/sys_req.h>
#include <mpx/time.h>
#include <mpx/syscalls.h>
#include <mpx/pcb.h>
#include <string.h>
#include <stdlib.h>

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

char intParsable(const char* string, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if ( (string[i] > '9') || (string[i] < '0') ) {
            return 0;
        }
    }
    return 1;
}

int helpCommand();
int setTimeCommand();
int getTimeCommand();
int setDateCommand();
int getDateCommand();
int createPcbCommand();
int setPcbPriorityCommand();
int versionCommand();
int shutdownCommand();

const struct cmd_entry {
    const char* key;
    const size_t key_len;
    const char* key_alt;
    const size_t key_alt_len;
    int (*const command_func)();
    const char* help_msg;
    const size_t help_msg_len;
}
cmd_entries[] = 
{
    { STR_BUF("1"), STR_BUF("Help"), helpCommand,
        STR_BUF(
        "Help\r\n"
            "\tInput:\r\n"
            "\tcommand - the command name or its corresponding number or all\r\n"
            "\tOutput:\r\n"
            "\tno return value\r\n"
            "\tDescription:\r\n"
            "\tprints all available user commands as well as details about the commands if\r\n"
            "\tall is used as the parameter\r\n"
            "\tprints the user command with details if a command or its id is specified\r\n"
        )
    },
    { STR_BUF("2"), STR_BUF("Set Time"), setTimeCommand,
        STR_BUF(
        "Set Time\r\n"
            "\tInput:\r\n"
            "\thour - the hour to be written into data (0-23)\r\n"
            "\tminute - the minute to be written into data (0-59)\r\n"
            "\tsecond - the second to be written into data (0-59)\r\n"
            "\tOutput:\r\n"
            "\tno output\r\n"
            "\tDescription:\r\n"
            "\tSets the system time.\r\n"
            "\tThe user will be prompted to enter the hour, minute, then second specifications\r\n"
        )
    },
    { STR_BUF("3"), STR_BUF("Get Time"), getTimeCommand,
        STR_BUF(
        "Get Time\r\n"
            "\tInput:\r\n"
            "\tno inputs\r\n"
            "\tOutput:\r\n"
            "\toutputs the system time in month, day, year format\r\n"
            "\tDescription:\r\n"
            "\tOutputs the time stored on the system data registers\r\n"
        )
    },
    { STR_BUF("4"), STR_BUF("Set Date"), setDateCommand,
        STR_BUF(
        "Set Date\r\n"
            "\tInput:\r\n"
            "\tday - the number to set the system day to\r\n"
            "\tdate range depends on month 1 is always the minimum while the maximum could be 28, 29, 30, 31\r\n"
            "\tmonth - the number to set the system month to (1-12)\r\n"
            "\tyear - the number to set the system year to (2 digits)\r\n"
            "\tOutput:\r\n"
            "\tno output\r\n"
            "\tDescription:\r\n"
            "\tsets the system date by writing to the system data register\r\n"
            "\tit will prompt the user to enter the month XX, then day XX, year 20XX\r\n"
        )
    },
    { STR_BUF("5"), STR_BUF("Get Date"), getDateCommand,
        STR_BUF(
        "Get Date\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\toutputs the system date\r\n"
		    "\tDescription:\r\n"
		    "\toutputs the date stored on the system data registers in month, day, year\r\n"
            "\tformat\r\n"
        )
    },
    { STR_BUF("6"), STR_BUF("Create PCB"), createPcbCommand,
        STR_BUF(
        "Create PCB\r\n"
		    "\tInput:\r\n"
		    "\tprocess name - name to provide to the new process\r\n"
            "\tprocess class - class of the new process\r\n"
            "\tprocess priority - base priority of the new process\r\n"
		    "\tOutput:\r\n"
		    "\tno output if successful, error if a pcb could not be created\r\n"
		    "\tDescription:\r\n"
		    "\tCreates a new process pcb to insert it into an active ready queue\r\n"
        )
    },
    { STR_BUF("7"), STR_BUF("Set PCB Priority"), setPcbPriorityCommand,
        STR_BUF(
        "Set PCB Priority\r\n"
		    "\tInput:\r\n"
            "\tprocess name - corresponding name of the new process\r\n"
            "\tprocess priority - base priority of the new process\r\n"
		    "\tOutput:\r\n"
		    "\tno output\r\n"
		    "\tDescription:\r\n"
		    "\tLocates a process pcb and changes its base priority\r\n"
        )
    },
    { STR_BUF("8"), STR_BUF("Version"), versionCommand,
        STR_BUF(
        "Version\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\tno output\r\n"
		    "\tDescription:\r\n"
		    "\tprints the current version of MPX and the compilation date\r\n"
        )
    },
    { STR_BUF("9"), STR_BUF("Shut Down"), shutdownCommand,
        STR_BUF(
        "Shut Down\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\tno output value\r\n"
		    "\tDescription:\r\n"
		    "\tshuts down the machine after confirmation is given by entering 1\r\n"
        )
    },
};

char user_input[100];
int user_input_len = 0;
// functions to help manage the global input buffer and debug.
void user_input_clear() {
    memset(user_input, 0, user_input_len);
#ifdef MPX_DEBUG
    user_input_len = 0;
#endif
}

void user_input_promptread() {
#ifdef MPX_DEBUG
    if (user_input_len != 0)
    {
        write(COM1, STR_BUF("DEBUG: Missed manual clear\r\n"))
    }
#endif
    user_input_len = read(COM1, user_input, sizeof(user_input));
    write(COM1, STR_BUF("\r\n"));
    return;
}

int setTimeCommand() {
    int hour, minute, second;

    static const char error_msg[] = "Could not parse, please re-enter time:\r\n";
    
    while(1) {
        setTerminalColor(Yellow);
        static const char hour_msg[] = "Enter the hour (0-23):\r\n";
        write(COM1, STR_BUF(hour_msg));

        setTerminalColor(White);
        user_input_promptread();
        if (intParsable(user_input, user_input_len)) {
            hour = atoi(user_input);
                    
            if ( (hour < 24) && (hour >= 0) ) {
                user_input_clear();
                break;
            }
        }
        user_input_clear();
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char minute_msg[] = "Enter the minute (0-59):\r\n";
        write(COM1, STR_BUF(minute_msg));

        setTerminalColor(White);
        user_input_promptread();
        if (intParsable(user_input, user_input_len)) {
            minute = atoi(user_input);
             
            if ( (minute < 60) && (minute >= 0) ) {
                user_input_clear();
                break;
            }
        }
        user_input_clear();
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    while (1) {
        setTerminalColor(Yellow);
        static const char second_msg[] = "Enter the second (0-59):\r\n";
        write(COM1, STR_BUF(second_msg));

        setTerminalColor(White);
        user_input_promptread();
        if (intParsable(user_input, user_input_len)) {
            second = atoi(user_input);
            
            if ( (second < 60) && (second >= 0) ) {
                user_input_clear();
                break;
            }
        }
        user_input_clear();
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }

    setTime(hour, minute, second);
    return 0;
}

int getTimeCommand() {
    setTerminalColor(Yellow);
    static const char day_msg[] = "The time is:\r\n";
    write(COM1, STR_BUF(day_msg));
    
    getTime();
    return 0;
}

int setDateCommand() {
    int day, month, year;
      
    static const char error_msg[] = "Could not parse, please re-enter time:\r\n";
    
    while(1) {
        setTerminalColor(Yellow);
        static const char month_msg[] = "Enter the month (1-12):\r\n";
        write(COM1, STR_BUF(month_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if (intParsable(user_input, user_input_len)) {
            month = atoi (user_input);
            
            if ( (month <= 12) && (month >= 1) ) {
                user_input_clear();
                break;
            }
        }
        user_input_clear();

        setTerminalColor(Red);       
        write(COM1, STR_BUF(error_msg));    
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char year_msg[] = "Enter the last two digits of the year (0-99):\r\n";
        write(COM1, STR_BUF(year_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if (intParsable(user_input, user_input_len)) {
            year = atoi (user_input);
            
            if ( (year < 100) && (year >= 0) ) {
                user_input_clear();
                break;
            }
        }
        user_input_clear();
        
        setTerminalColor(Red);
        write(COM1, STR_BUF(error_msg));
    }
    while(1) {
        setTerminalColor(Yellow);
        static const char day_msg[] = "Enter the day of the month:\r\n";
        write(COM1, STR_BUF(day_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if (intParsable(user_input, user_input_len)) {
            day = atoi (user_input);
            
            if (
                 (
                    (day <= month_info[month - 1].lastday) || 
                    ((month == 2) && (year % 4 == 0) && (day <= 29))
                 ) && 
                 (day >= 1) 
            ) {
                user_input_clear();
                break;
            }
        }
        user_input_clear();
        
        setTerminalColor(Red); 
        write(COM1, STR_BUF(error_msg));
    }
    write(COM1, STR_BUF("\r\n"));
    
    setDate (day, month, year);
    return 0;
}

int getDateCommand() {
    setTerminalColor(Yellow);
    static const char day_msg[] = "The date is:\r\n";
    write(COM1, STR_BUF(day_msg));
    
    getDate();
    return 0;
}

int versionCommand() {
    setTerminalColor(White);
    static const char ver_msg[] = "MPX vR1.\r\nCompiled ";
    write(COM1, STR_BUF(ver_msg));
   
    write(COM1, STR_BUF(__DATE__));
    write(COM1, STR_BUF("\r\n"));
    return 0;
}

const struct str_pcbclass_map {
    const char* str;
    const enum ProcClass class;
} avail_pcb_class[] = {
    { "user", USER },
    { "kernel", KERNEL }
};

int createPcbCommand() {
    char proc_name[MPX_PCB_PROCNAME_SZ];
    enum ProcClass proc_class;
    unsigned char proc_pri;
    
    while(1) {
        static const char name_msg[] = "Enter the name for the new process,\r\n"
                                       "it must be at least 8 and up to 64 characters long.\r\n";
        setTerminalColor(Yellow);
        write(COM1, STR_BUF(name_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if ((user_input_len <= MPX_PCB_PROCNAME_SZ) && (user_input_len >= MPX_PCB_PROCNAME_MIN))
        {
            memcpy(proc_name, user_input, user_input_len);
            user_input_clear();
            break;
        }
        user_input_clear();
        
        setTerminalColor(Red);
        static const char name_error_msg[] = "Name falls out of the range of 8 and 64\r\n";
        write(COM1, STR_BUF(name_error_msg));
    }
    while(1) {
        static const char class_msg[] = "Enter the class for the new process <user | kernel>:\r\n";
        setTerminalColor(Yellow);
        write(COM1, STR_BUF(class_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        for (size_t i = 0; i < sizeof(avail_pcb_class) / sizeof(struct str_pcbclass_map); ++i)
        {
            if (strcmp(user_input, avail_pcb_class[i].str) == 0)
            {
                proc_class = avail_pcb_class[i].class;
                user_input_clear();
                goto procbreak;
            }
        }
        user_input_clear();
        
        setTerminalColor(Red);
        static const char class_error_msg[] = "Invalid class provided.\r\n";
        write(COM1, STR_BUF(class_error_msg));
    }
    procbreak:
    while(1) {
        static const char pri_msg[] = "Enter the base priority for the new process (0-9):\r\n";
        setTerminalColor(Yellow);
        write(COM1, STR_BUF(pri_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if (user_input_len == 1)
        {
            if (intParsable(user_input, user_input_len))
            {
                proc_pri = atoi(user_input);
                if ((proc_pri >= 0) && (proc_pri <= 9))
                {
                    user_input_clear();
                    break;
                }
            }
        }
        user_input_clear();
        
        setTerminalColor(Red);
        static const char pri_error_msg[] = "Priority is not in the accepted range.\r\n";
        write(COM1, STR_BUF(pri_error_msg));
    }

    struct pcb* pcb_new = pcb_setup(proc_name, proc_class, proc_pri);
    if (pcb_new == NULL)
    {
        return -1;
    }
    pcb_insert(pcb_new);
    return 0;
}

int setPcbPriorityCommand() {
    return 0;
}

int helpCommand() {
	setTerminalColor(Yellow);
    static const char help_msg[] = "Enter the command name or id to display a specific command\r\n"
                                   "Enter [all] to display all commands\r\n";
	write(COM1, STR_BUF(help_msg));
    setTerminalColor(White);
	user_input_promptread();
    
    if (strcmp("all", user_input) == 0) {
        for (size_t i = 0; i < sizeof(cmd_entries) / sizeof(struct cmd_entry); ++i)
        {
            write(COM1, cmd_entries[i].help_msg, cmd_entries[i].help_msg_len);
        }
        user_input_clear();
        return 0;
    }
    else
    {
        for (size_t i = 0; i < sizeof(cmd_entries) / sizeof(struct cmd_entry); ++i)
        {
            if (
                (strcmp(user_input, cmd_entries[i].key) == 0) || 
                (strcmp(user_input, cmd_entries[i].key_alt) == 0)
            )
            {
                write(COM1, cmd_entries[i].help_msg, cmd_entries[i].help_msg_len);
                user_input_clear();
                return 0;
            }
        }
    }
	write(COM1, STR_BUF("Command name not recognized\r\n"));
    user_input_clear();
    return 0;
}

int shutdownCommand() {
    setTerminalColor(Red);
    static const char shutdown_msg[] = "Are you sure you would like to shut down?\r\n"
                                       "Enter 1 to confirm, enter another key to go back to menu:\r\n";
    write(COM1, STR_BUF(shutdown_msg));

    user_input_promptread();
    
    setTerminalColor(White);
    if (strcmp(user_input, "1") == 0){
        static const char sdexec_msg[] = "Shutting down now.\r\n";
        write(COM1, STR_BUF(sdexec_msg));
        user_input_clear();
        return 0;
    }
    static const char sdcancel_msg[] = "Shut down cancelled.\r\n";
    write(COM1, STR_BUF(sdcancel_msg));
    user_input_clear();
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
        commandloop_begin:
        setTerminalColor(Purple);
        //display the menu
        write(COM1, STR_BUF(menu_options));
        
        setTerminalColor(White);
        // get the input and call the corresponding function
        user_input_promptread();
        
        int retcode;
        for (size_t i = 0; i < sizeof(cmd_entries) / sizeof(struct cmd_entry); ++i)
        {
            if (
                (strcmp(user_input, cmd_entries[i].key) == 0) || 
                (strcmp(user_input, cmd_entries[i].key_alt) == 0)
            )
            {
                user_input_clear();
                retcode = cmd_entries[i].command_func();
                if (
                    (cmd_entries[i].command_func == shutdownCommand) &&
                    (retcode == 0)
                )
                {
                    return;
                }
                goto commandloop_begin;
            }
        }
        user_input_clear();
        setTerminalColor(Red);
        static const char invalidOption[] = "Please select a valid option.\r\n";
        write(COM1, STR_BUF(invalidOption));
    }
}

