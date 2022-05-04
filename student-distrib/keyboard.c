#include "keyboard.h"
#include "terminal.h"
#include "system_execute.h"

// keyboard buffer
// char keybuf[KEYBUFSIZE];
// unsigned int buflen = 0;

// // flags
// volatile int enter_pressed = 1;
// volatile int shift = 0; // if 1, shift is pressed
// volatile int caps_lock = 0; //if 1, caps_lock is on
// volatile int caps_lock_count = 0;
// volatile int capital = 0; //final flag to determine whether characters should be printed in capital
// volatile int ctrl = 0; //if 1, ctrl is pressed

// arrays for numbers and characters
uint8_t numbers[10] = {'1','2','3','4','5','6','7','8','9','0'};
uint8_t characters_1[10] = {'q','w','e','r','t','y','u','i','o','p'};
uint8_t characters_2[9] = {'a','s','d','f','g','h','j','k','l'};
uint8_t characters_3[7] = {'z','x','c','v','b','n','m'};
uint8_t symbols[10] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};

/* void init_keyboard()
 * Inputs: none
 * Return Value: none
 * Function: initializes the IRQ line for the keyboard */
void init_keyboard(){
    enable_irq(KB_IRQ);
}

/* void keyboard_handler()
 * Inputs: none
 * Return Value: none
 * Function: manages interrupts sent from keyboard using switch table */
void keyboard_handler() {
    // cli();          // use cli/sti rather than disabling irq completely

    // read off the scancode from the keyboard
    uint32_t scancode = inb(KEYBOARD_PORT);

    char outchar;

    termdata_t* termdata_ptr =  &(terminals[curr_term]);

    if (scancode == EZERO) {
        termdata_ptr->e_zero = 1;
    } 

    // check flags -- shift, ctrl, capslock
    if (scancode == PRESS_L_SHIFT || scancode == PRESS_R_SHIFT) {           
        termdata_ptr->shift = 1;
    }
    if (scancode == RELEASE_L_SHIFT || scancode == RELEASE_R_SHIFT) {
        termdata_ptr->shift = 0;
    }
    if (scancode == PRESS_L_CTRL || ((scancode == PRESS_R_CTRL) && (termdata_ptr->e_zero))) {
        termdata_ptr->ctrl = 1;
    }
    if (scancode == RELEASE_L_CTRL || ((scancode == RELEASE_R_CTRL) && (termdata_ptr->e_zero))) {
        termdata_ptr->ctrl = 0;
    }
    if (((scancode == PRESS_R_ALT) && (termdata_ptr->e_zero)) || scancode == PRESS_L_ALT) {
        termdata_ptr->alt = 1;
    }
    if (((scancode == RELEASE_R_ALT) && (termdata_ptr->e_zero)) || scancode == RELEASE_L_ALT) {
        termdata_ptr->alt = 0;
    }
    if (scancode == PRESS_CAPS_LOCK) {
        if (termdata_ptr->caps_lock_count % 2 == 0) {
            termdata_ptr->caps_lock = 1;
        } else {
            termdata_ptr->caps_lock = 0;
        }
        termdata_ptr->caps_lock_count++;
    }
    if ((termdata_ptr->shift && !(termdata_ptr->caps_lock)) || (!(termdata_ptr->shift) && (termdata_ptr->caps_lock))) {
        termdata_ptr->capital = 1;
    }

    // translate scancode into ASCII char
    if (scancode == L && termdata_ptr->ctrl) {    // clear screen
        clear();
        update_cursor();
    }  else if (scancode == F1 && termdata_ptr->alt == 1) {
        // printf("Switching to 1\n");
        term_switch(0);
        // return; 
    }
    else if (scancode == F2 && termdata_ptr->alt == 1) {
        // printf("Switching to 2\n");
        term_switch(1);
        // return;
    }
    else if (scancode == F3 && termdata_ptr->alt == 1) {
        // printf("Switching to 3\n");
        term_switch(2);
        // return;
    } else if (scancode >= ONE && scancode <= ZERO) {
        if (termdata_ptr->shift) {
            outchar = symbols[scancode - ONE];
        } else {
            outchar = numbers[scancode - ONE];
        }
    } else if (scancode >= Q && scancode <= P) {
        if (termdata_ptr->capital) {
            outchar = characters_1[scancode - Q] - CAPS;
        } else {
            outchar = characters_1[scancode - Q];
        }
    } else if (scancode >= A && scancode <= L) {
        if (termdata_ptr->capital) {
            outchar = characters_2[scancode - A] - CAPS;
        } else {
            outchar = characters_2[scancode - A];
        }
    } else if (scancode >= Z && scancode <= M) {
        if (termdata_ptr->capital) {
            outchar = characters_3[scancode - Z] - CAPS;
        } else {
            outchar = characters_3[scancode - Z];
        }
    } else if (scancode == DASH) {
        if (!(termdata_ptr->shift)) {
            outchar = '-';
        } else {
            outchar = '_';
        }
    } else if (scancode == EQUAL) {
        if (!(termdata_ptr->shift)) {
            outchar = '=';
        } else {
            outchar = '+';
        }
    } else if (scancode == LBRACK) {
        if (!(termdata_ptr->shift)) {
            outchar = '[';
        } else {
            outchar = '{';
        }
    } else if (scancode == RBRACK) {
        if (!(termdata_ptr->shift)) {
            outchar = ']';
        } else {
            outchar = '}';
        }
    } else if (scancode == SEMI) {
        if (!(termdata_ptr->shift)) {
            outchar = ';';
        } else {
            outchar = ':';
        }
    } else if (scancode == QUOTE) {
        if (!(termdata_ptr->shift)) {
            outchar = QUOTE_ASCII;
        } else {
            outchar = '"';
        }
    } else if (scancode == TICK) {
        if (!(termdata_ptr->shift)) {
            outchar = '`';
        } else {
            outchar = '~';
        }
    } else if (scancode == BSLASH) {
        if (!(termdata_ptr->shift)) {
            outchar = BSLASH_ASCII;
        } else {
            outchar = '|';
        }
    } else if (scancode == COMMA) {
        if (!(termdata_ptr->shift)) {
            outchar = ',';
        } else {
            outchar = '<';
        }
    } else if (scancode == PERIOD) {
        if (!(termdata_ptr->shift)) {
            outchar = '.';
        } else {
            outchar = '>';
        }
    } else if (scancode == SLASH) {
        if (!(termdata_ptr->shift)) {
            outchar = '/';
        } else {
            outchar = '?';
        }
    } else if (scancode == BACK) { // backspace pressed
        if (termdata_ptr->buflen >= 1) {
            outchar = '\b';
            termdata_ptr->buflen -= 1;
            if (termdata_ptr->buflen < 0) {
                termdata_ptr->buflen = 0;
            }
            echo(outchar);
        }
    } else if (scancode == TAB) { // tab pressed
        outchar = '\t';
    } else if (scancode == SPACEBAR) {  // spacebar pressed
        outchar = SPACE_ASCII;
    } else if (scancode == ENTER) {  // enter pressed
        outchar = '\n';
        termdata_ptr->enter_pressed = 0;
    } else {
        outchar = 0;    // handles edge case of non alphanumeric key pressed
    }

    if (outchar && outchar != '\b' && outchar != '\e') {  // checks whether outchar is actually a char
        echo(outchar);  
        // add chars to buffer
        if (termdata_ptr->buflen < KEYBUFSIZE){
            termdata_ptr->keybuf[termdata_ptr->buflen] = outchar;
            termdata_ptr->buflen++;    
        }
        else {
            termdata_ptr->keybuf[KEYBUFSIZE-1] = '\e';
        }
    }

    termdata_ptr->capital = 0;   // reset capital flag
    if (scancode != EZERO) {
        termdata_ptr->e_zero = 0;
    } 
    
    send_eoi(KB_IRQ);
    // sti();
}
