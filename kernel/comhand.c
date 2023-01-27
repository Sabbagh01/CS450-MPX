#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <comhand.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


void setDateCommand() {
    size_t input_len;
    char input_buffer[30] = { 0 };
    
    const char day_msg[] = "\r\nEnter the day as TWO numbers only:\r\n";
    sys_req(WRITE, COM1, day_msg, sizeof (day_msg));
    input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
    int day = atoi (input_buffer);
    memset (input_buffer, 0, input_len);
    
    const char month_msg[] = "\r\nEnter the month as TWO numbers only:\r\n";
    sys_req(WRITE, COM1, month_msg, sizeof (month_msg));
    input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
    int month = atoi (input_buffer);
    memset (input_buffer, 0, input_len);
    
    const char year_msg[] = "\r\nEnter the year as FOUR numbers only:\r\n";
    sys_req(WRITE, COM1, year_msg, sizeof (year_msg));
    input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
    int year = atoi (input_buffer);
    sys_req (WRITE, COM1, "\r\n", 2);
    
    setDate (day, month, year);
}

void getDateCommand() {
    const char day_msg[] = "\r\nThe date is:\r\n";
    sys_req(WRITE, COM1, day_msg, sizeof (day_msg));
    
    getDate();
}

void versionCommand() {
    const char ver_msg[] = "\r\nMPX vR1.\r\nDate:\r\n";
    sys_req(WRITE, COM1, ver_msg, sizeof (ver_msg));
    // print compilation date, will probably require some scripting and
    // the passing of a preprocessor define to the compiler in the makefile
}

void helpCommand() {
    const char help_msg[] = "\r\n"
                            "1) Help - Provides usage instructions for all commands\r\n"
                            "2) Set Time - Sets the time\r\n"
                            "3) Get Time - Returns the current time\r\n";
    sys_req(WRITE, COM1, help_msg, sizeof (help_msg));
}

void comhand(){
    const char menu_welcome_msg[] = "Welcome to 5x5 MPX.\r\n";
    const char menu_options[] = "Please select an option by choosing a number.\r\n"
                                "1) Help.        2) Set Time.    3) Get Time.    4) Set Date.\r\n"
                                "5) Get Date.    6) Version.     7) Shut Down.\r\n"
                                "Enter number of choice:\r\n";
    
    sys_req(WRITE, COM1, menu_welcome_msg, sizeof (menu_welcome_msg));
    while (1) {
        // display the menu
        sys_req(WRITE, COM1, menu_options, sizeof (menu_options));
        
        // Get the input and call corresponding function
        char user_input_buffer[100];
        sys_req(READ, COM1, user_input_buffer, sizeof (user_input_buffer));

        if (user_input_buffer[0] == '1') {
            helpCommand();
        } else if (user_input_buffer[0] == '4') {
            setDateCommand();
        } else if (user_input_buffer[0] == '5') {
            getDateCommand();
        } else if (user_input_buffer[0] == '6') {
            versionCommand();
        }
    }
}

