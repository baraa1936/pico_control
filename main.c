/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "pico/stdlib.h"
 
 #include "bsp/board_api.h"
 #include "tusb.h"
 #include "CustomFunctions.h"
 #include "usb_descriptors.h"
 #define UART_ID uart0
 
 #define UART_TX_PIN 0
 #define UART_RX_PIN 1
 
 //--------------------------------------------------------------------+
 // MACRO CONSTANT TYPEDEF PROTYPES
 //--------------------------------------------------------------------+
 
 /* Blink pattern
  * - 250 ms  : device not mounted
  * - 1000 ms : device mounted
  * - 2500 ms : device is suspended
  */
 enum  {
   BLINK_NOT_MOUNTED = 250,
   BLINK_MOUNTED = 1000,
   BLINK_SUSPENDED = 2500,
 };
 
 
 int switchData = 0;
 int KeyCodeEnabled = 0;
 int IntStr = 0;
 int IntStrKey = 0;
 uint8_t Keyboard_Position = 0;
 int16_t MousePosition[6];
 char Position[64] = {'\0'};
 char KeybaordInput[128] = {'\0'};
 
 static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
 static int chars_rxed = 0;
 static bool has_keyboard_key = true;
 
 
 
 void led_blinking_task(void);
 void hid_task(void);
 
 
 /*--------------URAT INTERPET-----------*/
 void on_uart_rx() { // this function will only read 4 bytes in the loop at a time 
   while (uart_is_readable(UART_ID)) { 
       uint8_t ch = uart_getc(UART_ID);
       // Can we send it back?
       if(ch == '$') {
           switchData = !switchData;
           IntStr = 0;
           IntStrKey = 0;
           uart_putc(UART_ID, '0' + switchData);
           return;
       }
       if (uart_is_writable(UART_ID)) {
           // Change it slightly first!
           // ch++;
           if(switchData != 1) {
             
             Position[IntStr] = ch;
             uart_putc(UART_ID, ch);
             if((Position[IntStr] == '\n')) {
               Position[IntStr] = '\0';
                 IntStr = 0;
                 return;
             }
             /*
             if (switchData == 0) 
                (the keyboard)
             */
           } else { 
 
             KeybaordInput[IntStrKey] = ch;
             uart_putc(UART_ID, ch);
             if((KeybaordInput[IntStrKey] == '\n')) {

              int* Command = ReadCommands(KeybaordInput); // Reads if there any commands like (Enter, Backspace, etc...)
              if(Command != 0) { // No Command
               
              if(Command[2] == 1) {
                KeyCodeEnabled = 1;
                KeybaordInput[IntStrKey] = '\0';
                IntStrKey = 0;
                Command = 0;
                has_keyboard_key = false;  
                return;
              }
               KeyCodeEnabled = 0;
               RemoveCommandString(KeybaordInput, Command[1]);
               
               KeybaordInput[Command[1]] = Command[0]; // Command[0] = The ascii value, Command[1] = the position of the prefix
               KeybaordInput[Command[1] + 1] = '\0';   //
               
               IntStrKey = 0;
               has_keyboard_key = false;  
              //  KeybaordInput[IntStr] = '\0';

              //  printf("\nCommandExec : = %d\n",Command[0]);
               Command = 0;
               return;
              }            

               KeybaordInput[IntStrKey] = '\0';
               has_keyboard_key = false;  
                 IntStrKey = 0;
                 return;
             }
 
           }  
                   
       }
       IntStr++;
       chars_rxed++;
       IntStrKey++;
    }
   // uart_puts(UART_ID, Postion);
 }
 
 /* ---------- Data Formating ------------ */
 void ReFormatingString(char* StringPosition) {
   int i = 0;
   char* StringSpliter = strtok(StringPosition, ","); // Split String into Tokens
   char* endptr;
   while(StringSpliter != NULL) {
     MousePosition[i] = strtol(StringSpliter, &endptr, 10); //  String to Integer
     i++;
     // printf("%s", StringSpliter);
     StringSpliter = strtok(NULL, ",");
   }
   // printf("\nX%d, Y%d\n", MousePosition[0], MousePosition[1]);
 } 
 
 
 
 /*------------- MAIN -------------*/
 int main(void)
 {
   board_init();
   stdio_init_all();
   // init device stack on configured roothub port
   tusb_rhport_init_t dev_init = {
     .role = TUSB_ROLE_DEVICE,
     .speed = TUSB_SPEED_AUTO
   };
   tusb_init(BOARD_TUD_RHPORT, &dev_init);
 
   if (board_init_after_tusb)
   {
     board_init_after_tusb();
   }
 
   int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
 
   // And set up and enable the interrupt handlers
   irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
   irq_set_enabled(UART_IRQ, true);
 
   // Now enable the UART to send interrupts - RX only
   uart_set_irq_enables(UART_ID, true, false);
 
   // OK, all set up.
   // Lets send a basic string out, and then run a loop and wait for RX interrupts
   // The handler will count them, but also reflect the incoming data back with a slight change!
   uart_puts(UART_ID, "\nHello, uart interrupts\n");
 
   while (1)
   {
     tud_task(); // tinyusb device task
     led_blinking_task();
     
     hid_task();
   }
 }
 
 //--------------------------------------------------------------------+
 // Device callbacks
 //--------------------------------------------------------------------+
 
 // Invoked when device is mounted
 void tud_mount_cb(void)
 {
   blink_interval_ms = BLINK_MOUNTED;
 }
 
 // Invoked when device is unmounted
 void tud_umount_cb(void)
 {
   blink_interval_ms = BLINK_NOT_MOUNTED;
 }
 
 // Invoked when usb bus is suspended
 // remote_wakeup_en : if host allow us  to perform remote wakeup
 // Within 7ms, device must draw an average of current less than 2.5 mA from bus
 void tud_suspend_cb(bool remote_wakeup_en)
 {
   (void) remote_wakeup_en;
   blink_interval_ms = BLINK_SUSPENDED;
 }
 
 // Invoked when usb bus is resumed
 void tud_resume_cb(void)
 {
   blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
 }
 
 //--------------------------------------------------------------------+
 // USB HID
 //--------------------------------------------------------------------+
 int rand() {
   
 }
 
 static void send_hid_report(uint8_t report_id, uint8_t Keyboard_I)
 {
   // skip if hid is not ready yet
   if ( !tud_hid_ready() ) return;
 
   switch(switchData)
   {
     case 0:
     {
       // use to avoid send multiple consecutive zero report for keyboard
       
       // printf("%d 0, %d 1 \n", MousePosition[0], MousePosition[1]);
       // printf("REPORT MOUSE HAS BEEN CALLED");
       // no button, right + down, no scroll, no pan
       ReFormatingString(Position);
       // printf("%d 0, %d 1 \n", MousePosition[0], MousePosition[1]);
       // printf("REPORT MOUSE HAS BEEN CALLED");
       // no button, right + down, no scroll, no pan
       tud_hid_abs_mouse_report(REPORT_ID_MOUSE, MousePosition[2], MousePosition[0], MousePosition[1], MousePosition[3], MousePosition[4]); // click, x, y, wheel up, wheel down
   
     }
     break;
 
     case REPORT_ID_KEYBOARD:
     {
       // use to avoid send multiple consecutive zero report for keyboard
       if(has_keyboard_key) {
        return;
      } 
      
       uint8_t const conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
        //  for(int i = 0;KeybaordInput[i] != '\0';i++) {
           uint8_t keycode[6] = { 0 };
           uint8_t modifier   = 0;
           
          if(KeyCodeEnabled == 1) {                
            if(!strncmp("!FCP", KeybaordInput, 4)) {
              modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
              keycode[0] = HID_KEY_F10;
              tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
              Keyboard_Position = 4; // WARNING : Editing Global Data
            }
            KeyCodeEnabled = 0;
            return;
          }

          if(Keyboard_I == 255) {
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            printf("\nIIX : = %c\n",KeybaordInput[Keyboard_I]);
            return; 
          }
    
          if(KeybaordInput[Keyboard_I] == '\0') {
            has_keyboard_key = true;  
            Keyboard_Position = 0;
          }  
  

           if ( conv_table[KeybaordInput[Keyboard_I]][0] ) modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
           keycode[0] = conv_table[KeybaordInput[Keyboard_I]][1];
          
         
           printf("%c",KeybaordInput[Keyboard_I]);
           tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
        //  }

     }
     break;
   }
 }
 // Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
 // tud_hid_report_complete_cb() is used to send the next report after previous one is complete
 void hid_task(void)
 {
   // Poll every 10ms
   const uint32_t interval_ms = 300;
   static uint32_t start_ms = 0;
   static uint32_t touch_ms = 0;
   static bool touch_state = false;
   if ( board_millis() - start_ms < interval_ms) return; // not enough time
   start_ms += interval_ms;
   if ( tud_suspended() )
   {
     // Wake up host if we are in suspend mode
     // and REMOTE_WAKEUP feature is enabled by host
     tud_remote_wakeup();
   }
 
     // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
     send_hid_report(REPORT_ID_KEYBOARD, 0);

 }
 
 // Invoked when sent REPORT successfully to host
 // Application can use this to send the next report
 // Note: For composite reports, report[0] is report ID
 void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
 {
   (void) instance;
   (void) len;
   static uint8_t SendNULLKey = 0;
   uint8_t next_report_id = report[0] + 1u;
  

   if(has_keyboard_key == false) {
    if(SendNULLKey) {
      send_hid_report(REPORT_ID_KEYBOARD, 255);
      SendNULLKey = 0;
      return; 
     }  
     
    Keyboard_Position++;
    send_hid_report(REPORT_ID_KEYBOARD, Keyboard_Position);
    SendNULLKey = 1;
    return;  
  }
 }
 
 // Invoked when received GET_REPORT control request
 // Application must fill buffer report's content and return its length.
 // Return zero will cause the stack to STALL request
 uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
 {
   // TODO not Implemented
   (void) instance;
   (void) report_id;
   (void) report_type;
   (void) buffer;
   (void) reqlen;
 
   return 0;
 }
 
 // Invoked when received SET_REPORT control request or
 // received data on OUT endpoint ( Report ID = 0, Type = 0 )
 void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
 {
   (void) instance;
 
   if (report_type == HID_REPORT_TYPE_OUTPUT)
   {
     // Set keyboard LED e.g Capslock, Numlock etc...
     if (report_id == REPORT_ID_KEYBOARD)
     {
       // bufsize should be (at least) 1
       if ( bufsize < 1 ) return;
 
       uint8_t const kbd_leds = buffer[0];
 
       if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
       {
         // Capslock On: disable blink, turn led on
         blink_interval_ms = 0;
         board_led_write(true);
       }else
       {
         // Caplocks Off: back to normal blink
         board_led_write(false);
         blink_interval_ms = BLINK_MOUNTED;
       }
     }
   }
 }
 
 //--------------------------------------------------------------------+
 // BLINKING TASK
 //--------------------------------------------------------------------+
 void led_blinking_task(void)
 {
   static uint32_t start_ms = 0;
   static bool led_state = false;
 
   // blink is disabled
   if (!blink_interval_ms) return;
 
   // Blink every interval ms
   if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
   start_ms += blink_interval_ms;
 
   board_led_write(led_state);
   led_state = 1 - led_state; // toggle
 }
 