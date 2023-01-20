#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>

// int splitter(int number){
//      while(number > 0){
//             int mod = number % 10;
//             number = number / 100;
//         }
//         return number;
// }



void comhand(void ){
    while(1){

        //menu display
        int bufferSize = 50;
        sys_req(WRITE, COM1,"Welocme to 5x5 MPX. Please select an option by choosing a number.", bufferSize);
        sys_req(WRITE, COM1,"1) Help.", bufferSize);
        sys_req(WRITE, COM1,"2) Set Time.", bufferSize);
        sys_req(WRITE, COM1,"3) Get Time.", bufferSize);
        sys_req(WRITE, COM1,"Enter choice: ", bufferSize);

        //read the choice from the user.
        char buf[100] =  {0};
        int nread = sys_req(READ, COM1, buf, sizeof(buf));
        // simply echoes the input
        sys_req(WRITE, COM1, buf, nread);
        //process the command
        // if(command was shutdown and shutdown is confirmed){
        //     return;
        // }
    }
    //method to convert binary to decimal

}
