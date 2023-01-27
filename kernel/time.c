#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <comhand.h>
#include <mpx/interrupts.h>
#include <string.h>
#include <time.h>

int decimalToBCD(int integer){
    int x = ((integer/10)<<4);
    int y = ((integer)%10);
   int output =  x+y;
    return output;
}

int BCDtoDecimal(int integer){
    int x = (integer&0x0F);
    int y = ((integer/16)*10);
    int output = x + y;
    return output;
}

    int year1(int number){
       
            number = number / 100;
        return number;
}
    int year2(int number){
            number = number % 100;
            // if(number < 10 && number >= 0){
            //    // number + 0
            // }
        return number;
    }

// void setTime(int hours, int minute){
    
//     //();
//     // convert to UTC
//     hours = hours + 4;

//     // if hours > 24
//     if(hours >= 24){
//         hours = hours - 24;
        
//         // increment date by 1
//         // int day = BintoDec(outb(0x70, 0x70));
//         // int month = BintoDec(outb(0x70,0x08));
//         // int year = BintoDec(outb(0x70,0x09));
        
//         //setDate(day, month, year);
//     }

//     // convert to binary
//     outb(0x70, 0x00);
//     //outb(0x71,dectoBin(seconds));
    
//     outb(0x70, 0x02);
//     //outb(0x71, dectoBin(minute));
    
//     outb(0x70, 0x04);
//    // outb(0x71, dectoBin(hours));
     
//     //sti();
// }
//void getTime(){
//}
void setDate( int day, int month, int year) {
        
        //get hours
        outb(0x70, 0x04);
        //get hours in bcd
        int binHours = inb(0x71);

        //hours convert to decimal 
        int decHours = BCDtoDecimal(binHours);


        //increase4 a day
        if(decHours < 5 &&  decHours > 0){
            if(month == 1){
                if(day == 31){
                    month = 2;
                    day = 1;
                }
                 else{
                    day = day+1;
                }
            }
           else if(month == 2){
            if(day == 28){
               if(((year % 4 ==0 )&& (year % 100 != 0)) || (year % 400 == 0)){
                day = day+1;
            }else {
                month = 3;
                day = 1;
            }
            }else if (day == 29){
                month = 3;
                day = 1;
            }else{
                    day = day+1;
                }
            
           }
           else if(month == 3){
                 if(day == 31){
                    month = 4;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 4){
                 if(day == 30){
                    month = 5;
                    day = 1;
                }
                else{
                    day = day+1;
                }
            }
            else if(month == 5){
                 if(day == 31){
                    month = 6;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
           else if(month == 6){
                 if(day == 30){
                    month = 7;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 7){
                 if(day == 31){
                    month = 8;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 8){
                 if(day == 30){
                    month = 9;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 9){
                 if(day == 31){
                    month = 10;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 10){
                 if(day == 30){
                    month = 11;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 11){
                 if(day == 31){
                    month = 12;
                    day = 1;
                }else{
                    day = day+1;
                }
            }
            else if(month == 12){
                 if(day == 30){
                    month = 1;
                    day = 1;
                    year = year+1;
                }else{
                    day = day+1;
                }
            }
        }

        int half1Year = year1(year);
        int half2Year = year2(year);
       
        

        int binYearHalf1 = decimalToBCD(half1Year);
        int binYearHalf2 = decimalToBCD(half2Year);
        int binMonth = decimalToBCD(month);
        int binDay = decimalToBCD(day);

        cli();

        //first half of years setup
        outb(0x70, 0x09);
        outb(0x71, binYearHalf1);

        //first half of years setup
        outb(0x70, 0x09);
        outb(0x71, binYearHalf2);

        //months setup 
        outb(0x70, 0x08);
        outb(0x71, binMonth);

        //Days setup
        outb(0x70, 0x07);
        outb(0x71, binDay);

        sti();
        return;

    }
        void getDate(){

        //getting the day
        outb(0x70, 0x07);
        int binDay = inb(0x71);

        //getting the month
        outb(0x70, 0x08);
        int binMonth = inb(0x71);

        //getting first half of year
        outb(0x70, 0x09);
        int binYear1 = inb(0x71);

        //getting second half of the year
        outb(0x70, 0x32);
        int binYear2 = inb(0x71);

        //convert the date to decimal
        int decDay = BCDtoDecimal(binDay);
        int decMonth = BCDtoDecimal(binMonth);
        int decYear1 = BCDtoDecimal(binYear1);
        int decYear2 = BCDtoDecimal(binYear2);

        //get hours
        outb(0x70, 0x04);
        //get hours in binary
        int binHours = inb(0x71);

        //hours convert to decimal 
        int decHours = BCDtoDecimal(binHours);

        //convert time to local time
        if(decHours < 5 &&  decHours > 0){
            if(decMonth == 1){
                if(decDay == 1){
                    decMonth = 12;
                    decDay = 31;
                    decYear2 = decYear2-1;
                    if(decYear2 < 0){
                        decYear2 = 99;
                        decYear1 = decYear1-1;
                    }
                }
                else{
                    decDay = decDay-1;
                    
                }
            }
           else if(decMonth == 2){
            if(decDay == 1){
                    decMonth = 1;
                    decDay = 31;
            }
            else{
                    decDay = decDay-1;
                }
           }
           else if(decMonth == 3){
                 if(decDay == 1){
                    decMonth = 2;
                    if((((decYear1*100 + decYear2) % 4 ==0 )&& ((decYear1*100 + decYear2) % 100 != 0)) || ((decYear1*100 + decYear2) % 400 == 0)){
                decDay = 29;
            }else {
                decDay = 28;
            }
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 4){
                 if(decDay == 1){
                    decMonth = 3;
                    decDay = 30;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 5){
                 if(decDay == 1){
                    decMonth = 4;
                    decDay = 31;
                }
                else{
                    decDay = decDay-1;
                }
            }
           else if(decMonth == 6){
                 if(decDay == 1){
                    decMonth = 5;
                    decDay = 30;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 7){
                 if(decDay == 1){
                    decMonth = 6;
                    decDay = 31;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 8){
                 if(decDay == 1){
                    decMonth = 7;
                    decDay = 30;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 9){
                 if(decDay == 1){
                    decMonth = 8;
                    decDay = 31;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 10){
                 if(decDay == 1){
                    decMonth = 9;
                    decDay = 30;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 11){
                 if(decDay == 1){
                    decMonth = 10;
                    decDay = 31;
                }
                else{
                    decDay = decDay-1;
                }
            }
            else if(decMonth == 12){
                 if(decDay == 1){
                    decMonth = 11;
                    decDay = 30;
                }
                else{
                    decDay = decDay-1;
                }
            }
        }
            
            //print year
            char calendar[20];
            //size_t lenCalendar = strlen(calendar);
            
    //  const char *dayMsg = "\nEnter the day as TWO numbers only: \n";
    // size_t lengthMsg1 = strlen(dayMsg);
    // sys_req(WRITE, COM1, dayMsg, lengthMsg1);

            // char printYear1[20];
            // size_t lenPrintYear1 = strlen(printYear1);

            // char printYear2[20];
            // size_t lenPrintYear2 = strlen(printYear2);

            // char printMonth[20];
            // size_t lenPrintMonth = strlen(printMonth);

            // char printDay[20];
            // size_t lenPrinDay = strlen(printDay);

        

        //     char test14[20];
        //     size_t lentest1 = strlen(test14);
        //     //13
        //     int num1 = 10000;

        //     itoa(test14,num1);
        //    // sys_req(WRITE, COM1, &test14[0], lentest1);
        //     //sys_req(WRITE, COM1, "\n", 2);
            
            


            itoa(calendar,decYear2);
            sys_req(WRITE, COM1, &calendar[0], 2);
            itoa(calendar,decYear1);
            sys_req(WRITE, COM1, &calendar[0], 2);
            sys_req(WRITE, COM1, "/", 2);

            itoa(calendar,decDay);
            sys_req(WRITE, COM1, &calendar[0], 2);
            sys_req(WRITE, COM1, "/", 2);
            
            itoa(calendar,decMonth);
            sys_req(WRITE, COM1, &calendar[0], 2);
            sys_req(WRITE, COM1, "\n", 2);



            //print month



            //print day
            

    }



 
