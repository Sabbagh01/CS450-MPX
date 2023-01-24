#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <comhand.h>
#include <mpx/interrupts.h>



int binToDec(int binNum){
        int dec = 0;
        int tempVal;
        int remain;
        int starter = 1;
        tempVal = binNum;

        while(tempVal > 0){
            remain = tempVal%10;
            dec = dec +(remain * starter);
            tempVal = tempVal / 10;
            starter = starter * 2;
        }
        return dec;
    }
    int decToBin(int decNum){
        int bin = 0;
        int remain;
        int tempVal;
        int starter = 1;
        tempVal = decNum;

        while( tempVal != 0){
            remain = tempVal%2;
            bin = bin + starter * remain;
            starter = starter*10;
            tempVal = tempVal/2;
        }
        return bin;
    }
    int year1(int number){
        while(number > 0){
            number = number / 100;
        }
        return number;
}
    int year2(int number){
         while(number > 0){
            number = number % 100;
        }
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
        
        
        outb(0x70, 0x04);

        //get hours in binary
        int binHours = inb(0x71);

        //hours convert to decimal 
        int decHours = binToDec(binHours);

        // //change hours to be 4 hours behind
        // decHours -= 4;
        // if(decHours < 0 ){
        //     decHours += 12;
        // }

        //increase4 a day
        if(decHours < 4 &&  decHours > 0){
            if(month == 1){
                if(day == 31){
                    month = 2;
                    day = 1;
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
            }
            
           }
           else if(month == 3){
                 if(day == 31){
                    month = 4;
                    day = 1;
                }
            }
            else if(month == 4){
                 if(day == 30){
                    month = 5;
                    day = 1;
                }
            }
            else if(month == 5){
                 if(day == 31){
                    month = 6;
                    day = 1;
                }
            }
           else if(month == 6){
                 if(day == 30){
                    month = 7;
                    day = 1;
                }
            }
            else if(month == 7){
                 if(day == 31){
                    month = 8;
                    day = 1;
                }
            }
            else if(month == 8){
                 if(day == 30){
                    month = 9;
                    day = 1;
                }
            }
            else if(month == 9){
                 if(day == 31){
                    month = 10;
                    day = 1;
                }
            }
            else if(month == 10){
                 if(day == 30){
                    month = 11;
                    day = 1;
                }
            }
            else if(month == 11){
                 if(day == 31){
                    month = 12;
                    day = 1;
                }
            }
            else if(month == 12){
                 if(day == 30){
                    month = 1;
                    day = 1;
                    year = year+1;
                }
            }
        }

        int half1Year = year1(year);
        int half2Year = year2(year);
       
        

        int binYearHalf1 = decToBin(half1Year);
        int binYearHalf2 = decToBin(half2Year);
        int binMonth = decToBin(month);
        int binDay = decToBin(day);

        cli();

        //first half of years setup
        outb(0x70, 0x09);
        outb(0x71, binYearHalf1);

        //first half of years setup
        outb(0x70, 0x32);
        outb(0x71, binYearHalf2);

        //months setup 
        outb(0x70, 0x08);
        outb(0x71, binMonth);

        //Days setup
        outb(0x70, 0x07);
        outb(0x71, binDay);

        sti();

    }
    // void getDate( int day, int month, int year) {

    //     //getting the day
    //     outb(0x70, 0x07);
    //     int binDay = inb(0x71);

    //     //getting the month
    //     outb(0x70, 0x08);
    //     int binMonth = inb(0x71);

    //     //getting first half of year
    //     outb(0x70, 0x09);
    //     int binYear1 = inb(0x71);

    //     //getting second half of the year
    //     outb(0x70, 0x32);
    //     int binYear2 = inb(0x71);

    //     //convert the date to decimal
    //     // int decDay = binToDec(binDay);
    //     // int decMonth = binToDec(binMonth);
    //     // int decYear1 = binToDec(binYear1);
    //     // int decYear2 = binToDec(binYear2);

    //     //itoa

    // }


 
