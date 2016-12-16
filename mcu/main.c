/*******************************************************************************
   main.c: Snake game
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
              Karel Slany    <slany AT fit.vutbr.cz>
              Tibor Dudlák   <xdudla00 AT fit.vutbr.cz>

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$

*******************************************************************************/

#include <fitkitlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thermometer/thermometer.h>
#include <lcd/display.h>
#include "keyboard.h"
#include "snake.h"

unsigned int flags_kbd = 0; // prinak smeru z klavesnice
unsigned short flag_int = 0; // priznak preruseni z casovace

unsigned char actual_x; // aktualni pozice
unsigned char actual_y;
unsigned char actual_d; // a smer

// nasledujici pozice a smer
unsigned char next_x, next_y, next_d;

#define CCR_INIT 0x1200         // pocatecni perioda casovani
#define CCR_STEP 0x0020         // o kolik se zkrati perioda
#define CCR_MIN  0x0500         // minimalni perioda (musi byt > CCR_STEP)

unsigned int ccr0_delay = CCR_INIT; // perioda

unsigned short started = 0;

/*******************************************************************************
 * Nastavenie a Vysvietenie LED
*******************************************************************************/
unsigned char row[8]={0};

void setLEDxy(unsigned char x, unsigned char y, int set) {
    if (set) {
        row[y] |= (1 << x);
    } else {
        row[y] &= ~(1 << x);
    }
}

void lightLED() {
    unsigned char i = 0;
    for (i = 0; i < 8; i++) {
        P4OUT = row[i] & 0x3f;
        P1OUT = row[i] & 0xc0;
        P6OUT = ~(1<<i);
        delay_ms(1);
    }
}

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
    term_send_str_crlf(" VGA CLEAR ...... smazani videoRAM");
    term_send_str_crlf(" VGA 0/1/2/3 .... zapis bytu do videoRAM");
}


/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return (CMD_UNKNOWN);
}

/*******************************************************************************
 * Inicializace struktur a hada
 *******************************************************************************/
void snake_init_structs(void)
{
  int i;

  if (snake != NULL)
     return;

  // inicializace poc. souradnic, smeru
  actual_x = BEGIN_X;
  actual_y = BEGIN_Y;
  actual_d = BEGIN_D;
  next_d   = actual_d;

  //inicializace indexu do struktury
  snake_struct_length = 0;
  snake_head_i = 0;
  snake_tail_i = 0;
  snake_extend = 0;

  // alokace struktury
  snake = malloc(sizeof(snake_t)*MAX_SNAKE_STRUCT_LENGTH);
  if (snake != NULL)
  {
     for (i = 0; i < INIT_SNAKE_LENGTH; i++)
     {
         snake_add_head(actual_x - INIT_SNAKE_LENGTH + i + 1, actual_y, actual_d);
         // vykresleni hada (pocatecni pozice)
        //  VGA_SetPixelXY(actual_x - INIT_SNAKE_LENGTH + i + 1, actual_y, 2);
         setLEDxy(actual_x - INIT_SNAKE_LENGTH + i + 1, actual_y, 2);
     }
  }
  else
  {
     term_send_str_crlf("Chyba alokace pameti");
  }
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized(void)
{
  LCD_init();
  LCD_write_string("Snake ...");

  // VRAM_Init(); //incializace VGA
  snake_init_structs(); //inicializace struktur
}

/*******************************************************************************
 * Obsluha preruseni casovace timer Timer A0
*******************************************************************************/
interrupt (TIMERA0_VECTOR) Timer_A (void) {
  //po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k dalsimu preruseni
  CCR0 += ccr0_delay;
  //nastalo preruseni - nastavit vlajku
  flag_int++;
}


/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  unsigned char ax, ay, ad; // pomocne
  unsigned int score = 0;
  unsigned short yummy_present = 0; // existuje nesnezene papani
  char result[10]; // textovy retezec zobrazeny na displeji

  snake = NULL;

  initialize_hardware();      //inicializace HW
  thermometer_init_rand();    //inicializace generatoru nah. cisel

  set_led_d6(1);              // rozsviceni D6
  set_led_d5(1);              // rozsviceni D5

  WDG_stop();                // zastav watchdog

  P6DIR = 0xff;
  P4DIR |= 0x3f;
  P1DIR |= 0xc0;

  while (1)
  {
    terminal_idle(); // obsluha terminalu
    lightLED();

    // preklad z prikazu klavesnice na novy smer
    switch (kbstate())
    {
      case CMD_UP :
        if (actual_d != D_DOWN)
           next_d = D_UP;
        break;

      case CMD_RIGHT :
        if (actual_d != D_LEFT)
           next_d = D_RIGHT;
        break;

      case CMD_DOWN :
        if (actual_d != D_UP)
           next_d = D_DOWN;
        break;

      case CMD_LEFT :
        if (actual_d != D_RIGHT)
           next_d = D_LEFT;
        break;

      case CMD_START :
        if ((started == 0) && (snake != NULL)) {
           CCTL0 = CCIE;             // povol preruseni pro casovac (rezim vystupni komparace)
           CCR0 = ccr0_delay;        // nastav po kolika ticich (16384 = 0x4000, tj. za 1/2 s) ma dojit k preruseni
           TACTL = TASSEL_1 + MC_2;  // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim
           started = 1;
        }
        break;

      default :
        break;
    }

    //rizeni hada
    if ((started) && (flag_int > 0))
    {
       flag_int--;

       // vypocet nove pozice z aktualniho smeru a pozice
       switch (next_d)
       {
         case D_UP :
           next_x = actual_x;
           next_y = Y_UP(actual_y);
           break;
         case D_RIGHT :
           next_x = X_RIGHT(actual_x);
           next_y = actual_y;
           break;
         case D_DOWN :
           next_x = actual_x;
           next_y = Y_DOWN(actual_y);
           break;
         case D_LEFT :
           next_x = X_LEFT(actual_x);
           next_y = actual_y;
           break;
         default :
           break;
       }

       //vygenerovani jidla
       if (!yummy_present)
       {
           unsigned char ax[4], ay[4];
           gen_yummy(ax, ay);
           int i = 0;
           for (i = 0; i < 4; i++) {
               setLEDxy(ax[i],ay[i],3);
               yummy_present++;
           }
       }

       //posun hada
       if (is_snake_body(next_x, next_y))
       {
          //had narazil - konec hry
          CCTL0 = 0; //zastavit preruseni
          started = 0;
          sprintf(result, "SCORE: %d", score);
          LCD_write_string(result); // zobrazeni skore
       }
       else
       {
          if (is_yummy(next_x, next_y))
          {
             // bude se prodluzovat
             snake_extend += SNAKE_EXTEND_STEP;
             // bylo spapano
             yummy_present--;
             // vypise se nove skore
             score++;
             sprintf(result, "Score: %d", score);
             LCD_write_string(result); // zobrazeni skore
             // zrychlime
             ccr0_delay -= CCR_STEP;
             if (ccr0_delay < CCR_MIN)
               ccr0_delay = CCR_MIN;
          }

          if (!snake_extend)
          {
             // zrusi konec hada
             snake_get_tail(&ax, &ay, &ad);
             snake_cut_tail();
             setLEDxy(ax, ay, 0);
          }
          else
          {
             snake_extend--;
          }

          // vykresli dalsi pixel
          snake_add_head(next_x, next_y, next_d);
          setLEDxy(actual_x, actual_y, 2);
          setLEDxy(next_x, next_y, 3);
          actual_x = next_x;
          actual_y = next_y;
          actual_d = next_d;
      }
    }
  }
}
