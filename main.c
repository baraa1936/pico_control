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

#include "usb_descriptors.h"
#define UART_ID uart0

#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

int16_t MousePosition[4];
char Position[64] = {'\0'};
int IntStr = 0;
int ModeSwitch = REPORT_ID_MOUSE;
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

void on_uart_rx() { // this function will only read 4 bytes in the loop at a time 
  while (uart_is_readable(UART_ID)) { 
      uint8_t ch = uart_getc(UART_ID);
      // Can we send it back?
      if(ch == 's') {
          ModeSwitch = REPORT_ID_KEYBOARD + !(ModeSwitch - 1);
          uart_putc(UART_ID, '0' + ModeSwitch);
          return;
      }
      if (uart_is_writable(UART_ID)) {
          // Change it slightly first!
          // ch++;
          Position[IntStr] = ch;
          uart_putc(UART_ID, ch);
          if((Position[IntStr] == '\n')) {
            Position[IntStr] = '\0';
              IntStr = 0;
              return;
          }        
      }
      IntStr++;
      chars_rxed++;
  }
  // uart_puts(UART_ID, Postion);
}

char checkp(char str[]) {
  int d = strlen(str);
  for(int i = 0;str[i] != '\0';i++) {
    // printf("\nchar %d == %c",str[i] ,str[i]);
  }
  printf("\nlen %d", d);
}

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  stdio_init_all();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

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
  (void)remote_wakeup_en;
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

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if (!tud_hid_ready())
  {
    printf("Function Has been called");
    return;
  };

  switch (report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;
      
        if(ModeSwitch == 1) {
          for(int i = 0; i < 9;i++) {
          uint8_t keycode[6] = { 0 };
          keycode[0] = HID_KEY_ENTER + i;
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
          has_keyboard_key = true;
          }// send empty key report if previously has key pressed  
        } else if(ModeSwitch == 2) {

          if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
          has_keyboard_key = false;

        }
        
      }
  
    break;

  case REPORT_ID_MOUSE:
  {
    int8_t const delta = 5;
    ReFormatingString(Position);
    // printf("%d 0, %d 1 \n", MousePosition[0], MousePosition[1]);
    // printf("REPORT MOUSE HAS BEEN CALLED");
    // no button, right + down, no scroll, no pan
    tud_hid_abs_mouse_report(REPORT_ID_MOUSE, MousePosition[2], MousePosition[0], MousePosition[1], 0, 0);
}
  break;
  default:
    break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete

void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 50;
  static uint32_t start_ms = 0;
  int TUD_HID_CHECK = tud_hid_ready();

  if (board_millis() - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  volatile uint32_t const btn = 1;

  // Remote wakeup

  // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
  send_hid_report(ModeSwitch, btn);
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
  (void)instance;
  (void)len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if (bufsize < 1)
        return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }
      else
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
  if (!blink_interval_ms)
    return;

  // Blink every interval ms
  if (board_millis() - start_ms < blink_interval_ms)
    return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
