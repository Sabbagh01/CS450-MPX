#include <mpx/comhand.h>
#include <mpx/io.h>
#include <mpx/serial.h>
#include <mpx/sys_req.h>
#include <mpx/time.h>
#include <mpx/syscalls.h>
#include <mpx/pcb.h>
#include <mpx/bufhelpers.h>
#include <mpx/loadR3.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

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

struct str_pcbprop_map {
    const char prop;
    const char* str;
    const char* str_out;
};

const struct str_pcbprop_map avail_pcb_class[] = {
    { KERNEL,   "kernel",   "Kernel" }, 
    { USER,     "user",     "User"   },
};

const struct str_pcbprop_map avail_pcb_execstate[] = {
    { BLOCKED,      "blocked",      "Blocked"   },
    { READY,        "ready",        "Ready"     },
    { RUNNING,      "running",      "Running"   },
};

const struct str_pcbprop_map avail_pcb_dpatchstate[] = {
    { SUSPENDED,    "suspended",    "Suspended" },
    { ACTIVE,       "active",       "Active"    },
};

const char* class_str(enum ProcClassState state) {
    return avail_pcb_class[state].str_out;
}

const char* execstate_str(enum ProcExecState state) {
    return avail_pcb_execstate[state].str_out;
}

const char* dpatchstate_str(enum ProcDpatchState state) {
    return avail_pcb_dpatchstate[state].str_out;
}

int helpCommand();
int setTimeCommand();
int getTimeCommand();
int setDateCommand();
int getDateCommand();
int createPcbCommand();
int setPcbPriorityCommand();
int showPcbCommand();
int deletePcbCommand();
int resumePcbCommand();
int suspendPcbCommand();
int unblockPcbCommand();
int blockPcbCommand();
int showPcbReadyCommand();
int showPcbBlockedCommand();
int showPcbAllCommand();
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
            "\tday - the number to set the system day to.\r\n"
            "\t      date range depends on month 1 is always the minimum while the maximum could be 28, 29, 30, 31\r\n"
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
    { STR_BUF("6"), STR_BUF("Set PCB Priority"), setPcbPriorityCommand,
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
    { STR_BUF("7"), STR_BUF("Show PCB"), showPcbCommand,
        STR_BUF(
        "Show PCB\r\n"
            "\tInput:\r\n"
            "\tName of Process\r\n"
            "\tOutput:\r\n"
            "\tInformation on found process\r\n"
            "\tDescription:\r\n"
            "\tPrints the name, class, state, suspended status, and priority of the given process\r\n"
        )
    },
    { STR_BUF("8"), STR_BUF("Show Ready PCBs"), showPcbReadyCommand,
        STR_BUF(
        "Show Ready PCBs\r\n"
            "\tInput:\r\n"
            "\tNone\n"
            "\tOutput:\r\n"
            "\tInformation on all ready PCBs\r\n"
            "\tDescription:\r\n"
            "\tPrints the name, class, state, suspended status, and priority of all ready PCBs\r\n"
        )
    },
    { STR_BUF("9"), STR_BUF("Show Blocked PCBs"), showPcbBlockedCommand,
        STR_BUF(
        "Show Blocked PCBs\r\n"
            "\tInput:\r\n"
            "\tNone\r\n"
            "\tOutput:\r\n"
            "\tInformation on all Blocked PCBs\r\n"
            "\tDescription:\r\n"
            "\tPrints the name, class, state, suspended status, and priority of all blocked PCBs\r\n"
        )
    },
    { STR_BUF("10"), STR_BUF("Show All PCBs"), showPcbAllCommand,
        STR_BUF(
        "Show All PCBs\r\n"
            "\tInput:\r\n"
            "\tNone\r\n"
            "\tOutput:\r\n"
            "\tInformation on all PCBs\r\n"
            "\tDescription:\r\n"
            "\tPrints the name, class, state, suspended status, and priority of all PCBs\r\n"
        )
    },
    { STR_BUF("11"), STR_BUF("Delete PCB"), deletePcbCommand,
        STR_BUF(
        "Delete PCB\r\n"
            "\tInput:\r\n"
            "\tName of Process\r\n"
            "\tOutput:\r\n"
            "\tNo output\r\n"
            "\tDescription:\r\n"
            "\tDeletes the PCB and frees associated memory of given PCB name if found\r\n"
        )
    },
    { STR_BUF("12"), STR_BUF("Block PCB"), blockPcbCommand,
        STR_BUF(
        "Block PCB\r\n"
            "\tInput:\r\n"
            "\tName of Process \r\n"
            "\tOutput:\r\n"
            "\tNo output\r\n"
            "\tDescription:\r\n"
            "\tSets the state of PCB to blocked if found\r\n"
        )
    },
    { STR_BUF("13"), STR_BUF("Unblock PCB"), unblockPcbCommand,
        STR_BUF(
        "Unblock PCB\r\n"
            "\tInput:\r\n"
            "\tName of Process\r\n"
            "\tOutput:\r\n"
            "\tNo output\r\n"
            "\tDescription:\r\n"
            "\tSets the state of PCB to unblocked if found\r\n"
        )
    },
    { STR_BUF("14"), STR_BUF("Suspend PCB"), suspendPcbCommand,
        STR_BUF(
        "Suspend PCB\r\n"
            "\tInput:\r\n"
            "\tName of Process \r\n"
            "\tOutput: \r\n"
            "\tNo output\r\n"
            "\tDescription:\r\n"
            "\tSuspends the PCB if found\r\n"
        )
    },
    { STR_BUF("15"), STR_BUF("Resume PCB"), resumePcbCommand,
        STR_BUF(
        "Resume PCB\r\n"
            "\tInput:\r\n"
            "\tName of Process\r\n"
            "\tOutput:\r\n"
            "\tNo output\r\n"
            "\tDescription:\r\n"
            "\tUnsuspends the PCB if found\r\n"
        )
    },
    { STR_BUF("16"), STR_BUF("Version"), versionCommand,
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
    { STR_BUF("17"), STR_BUF("Shut Down"), shutdownCommand,
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
    { STR_BUF("18"), STR_BUF("LoadR3"), loadR3,
        STR_BUF(
        "Shut Down\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\tno output value\r\n"
		    "\tDescription:\r\n"
		    "\ts 1\r\n"
        )
    },
     { STR_BUF("19"), STR_BUF("Yield"), yield,
        STR_BUF(
        "Shut Down\r\n"
		    "\tInput:\r\n"
		    "\tno input parameters\r\n"
		    "\tOutput:\r\n"
		    "\tno output value\r\n"
		    "\tDescription:\r\n"
		    "\tshuts down the machine after confirmation is given by entering 1\r\n"
        )
    }
};

char user_input[128];
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
    static const char ver_msg[] = "MPX vR2.\r\nCompiled ";
    write(COM1, STR_BUF(ver_msg));
   
    write(COM1, STR_BUF(__DATE__));
    write(COM1, STR_BUF("\r\n"));
    return 0;
}

int setPcbPriorityCommand() {
    char proc_name[MPX_PCB_PROCNAME_SZ];
    unsigned char proc_pri;

    struct pcb* pcb_findres;
    while(1) {
        static const char name_msg[] = "Enter the name of an existing process to change its priority:\r\n";
                                      
        setTerminalColor(Yellow);
        write(COM1, STR_BUF(name_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if ((user_input_len > 0) && (user_input_len <= MPX_PCB_PROCNAME_SZ))
        {
            memcpy(proc_name, user_input, user_input_len + 1);
            pcb_findres = pcb_find(proc_name);
            user_input_clear();
            if (pcb_findres == NULL)
            {
                setTerminalColor(Red);
                static const char find_error_msg[] = "Process name does not exist.\r\n";
                write(COM1, STR_BUF(find_error_msg));
                continue;
            }
            break;
        }
        user_input_clear();
        
        setTerminalColor(Red);
        static const char name_error_msg[] = "A process with the given name was not found\r\n";
        write(COM1, STR_BUF(name_error_msg));
    }
    while(1) {
        static const char pri_msg[] = "Enter process priority (0-9):\r\n";
        setTerminalColor(Yellow);
        write(COM1, STR_BUF(pri_msg));
        
        setTerminalColor(White);
        user_input_promptread();
        if (user_input_len == 1)
        {
            if (intParsable(user_input, user_input_len))
            {
                proc_pri = atoi(user_input);
                if ((proc_pri >= 0) && (proc_pri <= MPX_PCB_PROCPRI_MAX))
                {
                    pcb_remove(pcb_findres);
                    pcb_findres->state.pri = (unsigned char) proc_pri;
                    pcb_insert(pcb_findres);
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
    return 0;
}

int helpCommand() {
	setTerminalColor(Yellow);
    static const char help_msg[] = "Enter the command name same case sensitive or the number of the command\r\n"
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

int showPcbCommand(){
    setTerminalColor(Yellow);
    const char msg[] = "Enter the name of an existing process:\r\n";
    write(COM1, STR_BUF(msg));

    setTerminalColor(White);
    user_input_promptread();
    
    struct pcb* procfound = pcb_find(user_input);
    user_input_clear();
    if (procfound == NULL){
        setTerminalColor(Red);
        const char msgNotFound[] = "Specified process was not found\r\n";
        write(COM1, STR_BUF(msgNotFound));
        return 1;
    }
    
    const char msgName[] = "\r\nProcess Name: ";
    const char msgClass[] = "\r\nProcess Class: ";
    const char msgPri[] = "\r\nProcess Priority: ";
    const char msgState[] = "\r\nProcess State: ";
    const char msgStatus[] = "\r\nProcess Status: ";

    const char* charClass = class_str(procfound->state.cls);
    const char* charState = execstate_str(procfound->state.exec);
    const char* charStatus = dpatchstate_str(procfound->state.dpatch);
    char charPri[4]; 
    itoa(charPri, (int) procfound->state.pri);

    setTerminalColor(Yellow);
    write(COM1, STR_BUF(msgName));
    write(COM1, procfound->name, strlen(procfound->name));
    write(COM1, STR_BUF(msgClass));
    write(COM1, charClass, strlen(charClass));
    write(COM1, STR_BUF(msgPri));
    write(COM1, charPri, strlen(charPri));
    write(COM1, STR_BUF(msgState));
    write(COM1, charState, strlen(charState));
    write(COM1, STR_BUF(msgStatus));
    write(COM1, charStatus, strlen(charStatus));
    write(COM1, STR_BUF("\r\n"));
    return 0;
}

int deletePcbCommand() {
    setTerminalColor(Yellow);
    const char msg[] = "Enter the name of an existing process to be deleted:\r\n";
    write(COM1, STR_BUF(msg));

    setTerminalColor(White);
    user_input_promptread();

    struct pcb* procfound = pcb_find(user_input);
    user_input_clear();    
    if (procfound == NULL)
    {
        setTerminalColor(Red);
        const char msgNotFound[] = "A process with the given name was not found\r\n";
        write(COM1, STR_BUF(msgNotFound));
        return 1;
    }

    if (procfound->state.cls == KERNEL)
    {
        setTerminalColor(Red);
        const char msgKernel[] = "Process is a kernel process, cannot be removed\n";
        write(COM1, STR_BUF(msgKernel));
        return 1;
    }

    pcb_remove(procfound);
    pcb_free(procfound);
    return 0;
}

int blockPcbCommand() {
    setTerminalColor(Yellow);
    const char msg[] = "Enter the name of an existing process to block:\r\n";
    write(COM1, STR_BUF(msg));
    setTerminalColor(White);
    user_input_promptread();

    struct pcb* procfound = pcb_find(user_input);
    user_input_clear();
    if (procfound == NULL)
    {
        setTerminalColor(Red);
        const char msgNotFound[] = "A process with the given name was not found\r\n";
        write(COM1, STR_BUF(msgNotFound));
        return 1;
    }

    pcb_remove(procfound);

    procfound->state.exec = BLOCKED;
    pcb_insert(procfound);
    return 0;
}

int unblockPcbCommand() {
    setTerminalColor(Yellow);
    const char msg[] = "Enter the name of an existing process to unblock:\r\n";
    write(COM1, STR_BUF(msg));
    setTerminalColor(White);
    user_input_promptread();

    struct pcb* procfound = pcb_find(user_input);
    user_input_clear();
    if (procfound == NULL)
    {
        setTerminalColor(Red);
        const char msgNotFound[] = "A process with the given name was not found\r\n";
        write(COM1, STR_BUF(msgNotFound));
        return 1;
    }

    pcb_remove(procfound);
    
    procfound->state.exec = READY;
    pcb_insert(procfound);
    return 0;  
}

int suspendPcbCommand() {
    setTerminalColor(Yellow);
    const char msg[] = "Enter the name of an existing process to suspend:\r\n";
    write(COM1, STR_BUF(msg));
    setTerminalColor(White);
    user_input_promptread();
    
    struct pcb* procfound = pcb_find(user_input);
    user_input_clear();
    if (procfound == NULL)
    {
        setTerminalColor(Red);
        const char msgNotFound[] = "A process with the given name was not found\r\n";
        write(COM1, STR_BUF(msgNotFound));
        return 1;
    }

    if (procfound->state.cls == KERNEL)
    {
        setTerminalColor(Red);
        const char msgKernel[] = "Process is a kernel process, cannot be suspended\n";
        write(COM1, STR_BUF(msgKernel));
        return 1;
    }

    pcb_remove(procfound);
    
    procfound->state.dpatch = SUSPENDED;
    pcb_insert(procfound);
    return 0;  
}

int resumePcbCommand() {
    setTerminalColor(Yellow);
    const char msg[] = "Enter the name of an existing process to resume:\r\n";
    write(COM1, STR_BUF(msg));
    setTerminalColor(White);
    user_input_promptread();
    
    struct pcb* procfound = pcb_find(user_input);
    user_input_clear();
    if (procfound == NULL)
    {
        setTerminalColor(Red);
        const char msgNotFound[] = "A process with the given name was not found\r\n";
        write(COM1, STR_BUF(msgNotFound));
        return 1;
    }
    
    pcb_remove(procfound);
    
    procfound->state.dpatch = ACTIVE;
    pcb_insert(procfound);
    return 0;
}

int showPcbReadyCommand(){
    setTerminalColor(Yellow);
    const char msgName[] = "\r\nProcess Name: ";
    const char msgClass[] = "\r\nProcess Class: ";
    const char msgPri[] = "\r\nProcess Priority: ";
    const char msgState[] = "\r\nProcess State: ";
    const char msgStatus[] = "\r\nProcess Status: ";
    
    for (struct pcb_state i = { 0, SUSPENDED, READY, 0 }; i.exec <= ACTIVE; ++i.exec)
    {
        struct pcb_queue* queue_curr = &pcb_queues[PSTATE_QUEUE_SELECTOR(i)];
        // check that the queue is not empty (size > 0)
        if (queue_curr->head != NULL)
        {
            struct pcb_queue_node* node_temp = queue_curr->head;
            while(1)
            {
                //Get process info
                // alias
                struct pcb* proc = node_temp->pcb_elem;

                //Display information on process
                const char* charClass = class_str(proc->state.cls);
                const char* charState = execstate_str(proc->state.exec);
                const char* charStatus = dpatchstate_str(proc->state.dpatch);
                char charPri[4];
                itoa(charPri, (int) proc->state.pri);

                write(COM1, STR_BUF(msgName));
                write(COM1, proc->name, strlen(proc->name));
                write(COM1, STR_BUF(msgClass));
                write(COM1, charClass, strlen(charClass));
                write(COM1, STR_BUF(msgPri));
                write(COM1, charPri, strlen(charPri));
                write(COM1, STR_BUF(msgState));
                write(COM1, charState, strlen(charState));
                write(COM1, STR_BUF(msgStatus));
                write(COM1, charStatus, strlen(charStatus));
                write(COM1, STR_BUF("\r\n"));
                if (node_temp->p_next == NULL)
                {
                    break;
                }
                node_temp = node_temp->p_next;
            }
        }
   
    }
    return 0;
}

int showPcbBlockedCommand() {
    setTerminalColor(Yellow);
    const char msgName[] = "\r\nProcess Name: ";
    const char msgClass[] = "\r\nProcess Class: ";
    const char msgPri[] = "\r\nProcess Priority: ";
    const char msgState[] = "\r\nProcess State: ";
    const char msgStatus[] = "\r\nProcess Status: ";
    
    for (struct pcb_state i = { 0, SUSPENDED, BLOCKED, 0 }; i.exec <= ACTIVE; ++i.exec)
    {
        struct pcb_queue* queue_curr = &pcb_queues[PSTATE_QUEUE_SELECTOR(i)];
        // check that the queue is not empty (size > 0)
        if (queue_curr->head != NULL)
        {
            struct pcb_queue_node* node_temp = queue_curr->head;
            while(1)
            {
                //Get process info
                // alias
                struct pcb* proc = node_temp->pcb_elem;
                
                //Display information on process
                const char* charClass = class_str(proc->state.cls);
                const char* charState = execstate_str(proc->state.exec);
                const char* charStatus = dpatchstate_str(proc->state.dpatch);
                char charPri[4];
                itoa(charPri, (int) proc->state.pri);

                write(COM1, STR_BUF(msgName));
                write(COM1, proc->name, strlen(proc->name));
                write(COM1, STR_BUF(msgClass));
                write(COM1, charClass, strlen(charClass));
                write(COM1, STR_BUF(msgPri));
                write(COM1, charPri, strlen(charPri));
                write(COM1, STR_BUF(msgState));
                write(COM1, charState, strlen(charState));
                write(COM1, STR_BUF(msgStatus));
                write(COM1, charStatus, strlen(charStatus));
                write(COM1, STR_BUF("\r\n"));
                if (node_temp->p_next == NULL)
                {
                    break;
                }
                node_temp = node_temp->p_next;
            }
        }
   
    }
    return 0;
}

int showPcbAllCommand() {
    setTerminalColor(Yellow);
    const char msgName[] = "\r\nProcess Name: ";
    const char msgClass[] = "\r\nProcess Class: ";
    const char msgPri[] = "\r\nProcess Priority: ";
    const char msgState[] = "\r\nProcess State: ";
    const char msgStatus[] = "\r\nProcess Status: ";
    
    for (size_t i = 0; i < sizeof(pcb_queues) / sizeof(struct pcb_queue); ++i)
    {
        struct pcb_queue* queue_curr = &pcb_queues[i];
        // check that the queue is not empty (size > 0)
        if (queue_curr->head != NULL)
        {
            struct pcb_queue_node* node_temp = queue_curr->head;
            while(1)
            {
                //Get process info
                // alias
                struct pcb* proc = node_temp->pcb_elem;
                
                //Display information on process
                const char* charClass = class_str(proc->state.cls);
                const char* charState = execstate_str(proc->state.exec);
                const char* charStatus = dpatchstate_str(proc->state.dpatch);
                char charPri[4];
                itoa(charPri, (int) proc->state.pri);

                write(COM1, STR_BUF(msgName));
                write(COM1, proc->name, strlen(proc->name));
                write(COM1, STR_BUF(msgClass));
                write(COM1, charClass, strlen(charClass));
                write(COM1, STR_BUF(msgPri));
                write(COM1, charPri, strlen(charPri));
                write(COM1, STR_BUF(msgState));
                write(COM1, charState, strlen(charState));
                write(COM1, STR_BUF(msgStatus));
                write(COM1, charStatus, strlen(charStatus));
                write(COM1, STR_BUF("\r\n"));
                if (node_temp->p_next == NULL)
                {
                    break;
                }
                node_temp = node_temp->p_next;
            }
        }
   
    }
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
                                       "1 ) Help              2 ) Set Time          3 ) Get Time       4 ) Set Date\r\n"
                                       "5 ) Get Date          6 ) Change PCB Pri    7 ) Show PCB       8 ) Show Ready PCB\r\n"
                                       "9 ) Show Blocked PCB  10) Show All PCB      11) Delete PCB     12) Block PCB\r\n"
                                       "13) Unblock PCB       14) Suspend PCB       15) Resume PCB     16) Version\r\n"
                                       "17) Shutdown          18) loadR3            19) yield\r\n"
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

