/*-----------------------------------------------------------------------
Embedded Challenge Fall 2023 

Authors : Dehit Trivedi(dt2412), Dorian Yeh(dy2314), Ziyuan Liu(zl4963)

Objective : Calculate the Distance travelled and steps taken. 

/*
DataReading: Ability to successfully and continuously measure gyro values from the angular velocity sensor
Use spi, ticker to read the data.
spi, ticker -> set_flag(), set_mode();
------------------------------------------------------------------------*/


#include "mbed.h"
#include "drivers/LCD_DISCO_F429ZI.h"
using namespace std::chrono;
#define _USE_MATH_DEFINES
#include <math.h>

SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut chip_select(PC_1);  // chip select pin for Gyroscope

LCD_DISCO_F429ZI lcd;       //Create a LCD_DISCO_F429ZI object    

Ticker ticker;

Timer t;

volatile bool flag = true;


void set_flag() {
flag = true;
}


void set_mode() {          
  chip_select = 0;
  spi.write(0x20);
  spi.write(0xCF);
  chip_select = 1;
}

//Funtion to read data from gyroscope 
int16_t read_gyroscope_data(int address){

    // Low byte
    chip_select = 0;
    spi.write(address);
    uint8_t low_byte =spi.write(0x00); // Dummy byte
    chip_select = 1;

    // High byte
    chip_select = 0;
    spi.write(address+1);
    int8_t high_byte =spi.write(0x00); 
    chip_select = 1;

    // Sum 
    int16_t data = low_byte + high_byte*256;
    printf("%d",data);

    return data;
}



int main()
{
    
    chip_select = 1; // Chip must be deselected


    wait_us(1000);
    
    
    spi.format(8,3);  // Setup the spi for 8 bit data, high steady state clock,
    spi.frequency(1000000);  // second edge capture, with a 1MHz clock rate

    
    int16_t Buffer_gyro_sum=0;
    char buf[20];           //temporary string variable
    
    lcd.SetFont(&Font16);

    //SET MODE
    set_mode(); 


    //Variables 
    int16_t gyroscope_data;
    int buffer=0;
    uint8_t lenght_feet = 3; // Length of the feet in feets 
    int16_t velocity;
    float step_distance = 0;
    float total_distance = 0;
    int round_distance;
    int16_t Buffer_gyro[3]= {0,0,0};    //Array to store the results for Z from the gyro
    int16_t Buffer_veloity[20] = {0};   //Array to store linear velocity 
    uint8_t steps = 0;
    bool stepset = false;
    float time_count;
    uint8_t average_speed = 0;
    

    t.start();
    //ticker.attach(&set_flag, 1ms);

    while (1) 
    {  
        Buffer_gyro_sum=0; //setting the average to 0

        for(int j = 0;j<20;j++)
        {
            gyroscope_data = read_gyroscope_data(0xAC);
            gyroscope_data = gyroscope_data*0.00825;	//multiplying for 250 senstivity
            buffer +=gyroscope_data;
            wait_us(10000); 
        }
        buffer/=20; //Average value of the buffer 

        Buffer_gyro[2]=Buffer_gyro[1]; //shifting values for buffer Buffer_gyro
        Buffer_gyro[1]=Buffer_gyro[0];
        Buffer_gyro[0]=buffer;
        for(uint8_t k = 0; k<3;k++)
        Buffer_gyro_sum +=Buffer_gyro[k]; 
        Buffer_gyro_sum/=3; //buffer average

        buffer = (0.8)*Buffer_gyro[1] + 0.2*(buffer+Buffer_gyro[1])/2; // low pass filter
        
        
        velocity = buffer * lenght_feet * 0.0174; //degree to radian conversion lenght_feet = lenght of the tester's feet 

        step_distance = velocity * 0.14;  //Step distance 

        // filter for the idle values step_distance                                             
        if (step_distance > 0 && Buffer_gyro_sum >50)
        total_distance+=step_distance;
        round_distance = (int)total_distance;

        //Calculating the Steps
        if (buffer > 150 && stepset == false) 
        {
            steps+=1;
            stepset = true;
        }
        if(buffer < 150)
        stepset = false;

        //Calculate Time and Average Speed 
        time_count = duration_cast<milliseconds>(t.elapsed_time()).count() / 1000; //total time in seconds
        average_speed = round_distance/time_count; //calculating average speed since active
        Buffer_gyro_sum=0;

        //LCD Displays 
        lcd.DisplayStringAtLine(5, (uint8_t *) "Linear Velocity ");
        sprintf(buf, "V = %6d", velocity);
        lcd.DisplayStringAtLine(6,(uint8_t *) buf);
        lcd.DisplayStringAtLine(7, (uint8_t *) "Distance ");
        sprintf(buf, "D = %6d ft", round_distance);
        lcd.DisplayStringAtLine(8,(uint8_t *) buf);
        lcd.DisplayStringAtLine(9, (uint8_t *) "Steps ");
        sprintf(buf, "S = %6d steps", steps);
        lcd.DisplayStringAtLine(10,(uint8_t *) buf);
        wait_us(1000);
    }
}


