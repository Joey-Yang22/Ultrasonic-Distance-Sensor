// C++ code
//    


/*       Name: Joey Yang | Email: jyang474@ucr.edu:

*          Discussion Section: 022

 *         Assignment: Lab #5, Exercise #3

 *          
            Exercise Description:
              This exercise introduces the joystick and displays multiple numbers using the 4D7S display. 

              Task 5: Joystick - Period: 200ms

              When the joystick button is pressed, toggle the 4D7S display to show the distance in cm and inches.
              The RGB LED should still report the colors based on cm values (i.e., no changes to Tasks 3 and 4).
              Only the 4D7S display changes in response to the button being pressed. 
              Moving the joystick UP/DOWN adjusts the thresholds for changing the colors on the RGB LED.
              Close (5 cm, 10 cm)
              Moderate (10 cm, 20 cm) -- This is the default threshold when the system is initialized(The thresholds used for exercise 2)
              Far (15 cm, 30 cm)
              The default threshold is Moderate 
              Moving the joystick UP increases the threshold: Close → Moderate, Moderate → Far, Far → Far
              Moving the joystick DOWN decreases the threshold Far → Moderate, Moderate → Close, Close → Close

 *        

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: <https://www.youtube.com/watch?v=wqfMFw7WNSU&ab_channel=JoeyYang>

 */



#include "timerISR.h"
#include "helper.h"
#include "periph.h"


//TODO: declare variables for cross-task communication
int distance = 0;
int count = 0;
signed int change = 0;
int temp = 0;

int i = 0;

int inch = 0;

int count2 = 0;


//double temp = read_sonar() * 0.393701;


int pwmPeriod = 10;

int redH = 0;
int redL = 0;

int greenH = 0;
int greenL = 0;

int rDutyCycle = 0;
int gDutyCycle = 0;





/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 5


//Task struct for concurrent synchSMs implementations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsigned long TASK1_PERIOD = <PERIOD>
const unsigned long TASK1_PERIOD = 1000;
const unsigned long TASK2_PERIOD = 1;
const unsigned long TASK3_PERIOD = 1;
const unsigned long TASK4_PERIOD = 1;
const unsigned long TASK5_PERIOD = 200;

const unsigned long GCD_PERIOD = findGCD(TASK2_PERIOD, TASK1_PERIOD); /* TODO: Calulate GCD of tasks */ 


task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions

enum sonar_states {SO_start, detect};
int Tick_Sonar(int state);


int Tick_Sonar(int state)
{
    switch(state) { //state transitions
        case SO_start:
            state = detect;
            
        break;

        case detect:
            state = detect;
        break;

        default:
         state = SO_start;
    }

    switch(state) { //state actions

        case detect:
           
              distance = static_cast<int>(read_sonar());

            break;
            
           default:
            break;
    }
    return state;
}


enum display_states {DP_start, SHOW, BUTTON};
int Tick_Display(int state);

int Tick_Display(int state)
{
    inch = (distance * 2) / 5;
    int ones = (distance / 1) % 10;
    int tens = (distance / 10) % 10;
    int hundreds = (distance / 100) % 10;
    int thousands = (distance / 1000) % 10;

    int ones1 = (inch / 1) % 10;
    int tens1 = (inch / 10) % 10;
    int hundreds1 = (inch / 100) % 10;
    int thousands1 = (inch / 1000) % 10;

    switch(state) {

        case DP_start:
            state = SHOW;
        break;

        case SHOW:
        if(ADC_read(0) >= 0 && ADC_read(0) < 200)
        {
          state = BUTTON;
          count2++;
          if(count2 == 2 || count2 == 4 || count2 == 6 || count2 == 8)
            {
                PORTB = 0x01;
            }
        }
        else{
            state = SHOW;
            count++;
            if(count == 4 || count == 8 || count == 12 || count == 16)
            {
                PORTB = 0x01;
            }
        }
        break;

        default:
            state = DP_start;
    }

    switch(state) {

        case SHOW:
        if(count > 0 && count <= 4)
        {
                PORTB = PORTB | 0b11111011; //first digit
                outNum(ones);
        }
        else if(count > 4 && count <= 8)
        {
                PORTB = PORTB | 0b11110111; //second digit
                outNum(tens);
        }
        else if(count > 8 && count <= 12)
        {
                PORTB = PORTB | 0b11101111; //third digit
                outNum(hundreds);
        }
        else if(count > 12 && count <= 16)
        {
                PORTB = PORTB | 0b11011111; //fourth digit
                outNum(thousands);
        }
        if(count > 16)
        {
            count = 0;
        }
        break;


        case BUTTON:
        if(count2 > 0 && count2 <= 2)
        {
                PORTB = PORTB | 0b11111011; //first digit
                outNum(ones1);
        }
        else if(count2 > 2 && count2 <= 4)
        {
                PORTB = PORTB | 0b11110111; //second digit
                outNum(tens1);
        }
        else if(count2 > 4 && count2 <= 6)
        {
                PORTB = PORTB | 0b11101111; //third digit
                outNum(hundreds1);
        }
        else if(count2 > 6 && count2 <= 8)
        {
                PORTB = PORTB | 0b11011111; //fourth digit
                outNum(thousands1);
        }
        if(count2 > 8)
        {
            count2 = 0;
        }
        break;

        default:
            break;
    }

    return state;
}

enum green {g_start, GLow, GHigh};
int Green_Tick(int state);

int Green_Tick(int state) {


  if(distance < (10 + change))
          {
            gDutyCycle = 0;

          }
          else if (distance >= (10 + change) && distance <= (20 + change))
          {
            gDutyCycle = 3;
            
          }
          else{
            gDutyCycle = 10;
          }
          
  greenH = gDutyCycle;
  greenL = pwmPeriod - (gDutyCycle); 

    switch(state) { //state transitions
        case g_start:
            state = GHigh;
        break;

        case GHigh:
        if(i < greenH){
            state = GHigh;
        }
        else if(i >= greenH){
            state = GLow;
            i = 0;
          }
        break;

        case GLow:
        if(i < greenL || gDutyCycle == 0)
        {
            state = GLow;
        }
        else if(i >= greenL)
        {
            state = GHigh;
            i = 0;
        }
        break;

        

        default:
         state = g_start;
    }

    switch(state) { //state actions

        case GLow:
            PORTC = SetBit(PORTC, 4, 0);
            i++;
          break;  

        case GHigh:
            PORTC = SetBit(PORTC, 4, 1);
            i++;
            break;
            
           default:
            break;
    }
    return state;
}

enum red {r_start, RLow, RHigh};
int Red_Tick(int state);

int Red_Tick(int state)
{
   if(distance < (10 + change))
          {
            rDutyCycle = 10;

          }
          else if (distance >= (10 + change) && distance <= (20 + change))
          {
            rDutyCycle = 9;
            
          }
          else{
            rDutyCycle = 0;
          }

  redH = rDutyCycle;
  redL = pwmPeriod - (rDutyCycle); 


    switch(state) { //state transitions
        case r_start:
            state = RHigh;
        break;

        case RHigh:
        if(i < redH){
            state = RHigh;
        }
        else if(i >= redH){
            state = RLow;
            i = 0;
          }
        break;

        case RLow:
        if(i < redL)
        {
            state = RLow;
        }
        else if(i >= redL)
        {
            state = RHigh;
            i = 0;
        }
        break;

        

        default:
         state = r_start;
    }

    switch(state) { //state actions

        case RHigh:
          PORTC = SetBit(PORTC, 5, 1);
          i++;
        break;

        case RLow:
          PORTC = SetBit(PORTC, 5, 0);
          i++;
          break;


           default:
            break;
    }
    return state;
} 


enum joystick {j_start, WAIT, UP, DOWN};
int joystick_tick(int state);


int joystick_tick(int state)
{
    switch(state) { //state transitions
      case j_start:
        state = WAIT;
      break;

      case WAIT:
        
        if(ADC_read(1) <= 1023 && ADC_read(1) > 800)
        {
          state = UP;
        }
        else if(ADC_read(1) >= 0 && ADC_read(1) < 200)
        {
          state = DOWN;
        }
        break;

      case UP:
      if(ADC_read(1) > 500 && ADC_read(1) < 600)
      {
        change = change + 10;
        state = WAIT;
      }
       break;

      case DOWN:
      if(ADC_read(1) > 500 && ADC_read(1) < 600)
      {
        change = change - 10;
        state = WAIT;
      }
        break;


        default:
         state = j_start;
    }

    switch(state) { //state actions

            
           default:
            break;
    }
    return state;
}



void TimerISR() {
    
    //TODO: sample inputs here
    

	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int main(void) {
    //TODO: initialize all your inputs and ouputs

    //inputs = B0, C0, C1
    //echo = input, trigger = output
    //B0 = echo
    
    
      DDRD = 0xFF; PORTD = 0x00;
      DDRB = 0xFE; PORTB = 0x01;
      DDRC = 0xFC; PORTC = 0x03;

      


    ADC_init();   // initializes ADC
    init_sonar(); // initializes sonar

    //TODO: Initialize tasks here
    // e.g. tasks[0].period = TASK1_PERIOD
    // tasks[0].state = ...
    // tasks[0].timeElapsed = ...
    // tasks[0].TickFct = &task1_tick_function;
    tasks[0].period = TASK1_PERIOD;
    tasks[0].state = SO_start;
    tasks[0].elapsedTime = TASK1_PERIOD;
    tasks[0].TickFct = &Tick_Sonar;

    tasks[1].period = TASK2_PERIOD;
    tasks[1].state = DP_start;
    tasks[1].elapsedTime = TASK2_PERIOD;
    tasks[1].TickFct = &Tick_Display;

    tasks[2].period = TASK3_PERIOD;
    tasks[2].state = g_start;
    tasks[2].elapsedTime = TASK3_PERIOD;
    tasks[2].TickFct = &Green_Tick;

    tasks[3].period = TASK4_PERIOD;
    tasks[3].state = r_start;
    tasks[3].elapsedTime = TASK4_PERIOD;
    tasks[3].TickFct = &Red_Tick;

    tasks[4].period = TASK5_PERIOD;
    tasks[4].state = j_start;
    tasks[4].elapsedTime = TASK5_PERIOD;
    tasks[4].TickFct = &joystick_tick;

  
    

    TimerSet(GCD_PERIOD);
    TimerOn();


    while (1) {}

    return 0;
}
