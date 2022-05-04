
#define RTC_IRQ  8 // RTC goes to IRQ line 8 in cascaded PICs
#define RTC_PORT 0x70 // used to specify an index and to disable NMI
#define RTC_DATA 0x71 // used to read / write from bytes
#define REG_A   0x8A
#define REG_B   0x8B
#define REG_C   0x8C
#define RATE    0x0F  // frequency of 2 interrupts/sec == rate of 15
#define BIT6 0x40
#define LOW4 0xF0
#define LOWEST_RATE 3
#define HIGHEST_RATE 15
#define LOWEST_FREQ 2
#define HIGHEST_FREQ 1024
#define MAX_PID 6

/* RTC global vars */
int rtc_scales[MAX_PID];
int rtc_count;
// int process_flags[6];

/* void init_rtc()
 * Inputs: none
 * Return Value: none
 * Function: initializes the RTC */
void init_rtc();

/* void keyboard_handler()
 * Inputs: none
 * Return Value: none
 * Function: manages interrupts sent from RTC */
void rtc_handler();

/* int32_t rtc_open(const uint8_t* filename, int32_t current_pcb)
 * Inputs: none
 * Return Value: returns 0
 * Function: initializes RTC frequency to 2 Hz */
int32_t rtc_open(const uint8_t* filename);

/* int32_t rtc_close(int32_t fd, int32_t current_pcb)
 * Inputs: none
 * Return Value: returns 0
 * Function: none */
int32_t rtc_close(int32_t fd);

/* int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: none
 * Return Value: returns 0
 * Function: blocks RTC until OS generates a new interrupt */
int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes);

/* int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: buf -- buffer containing frequency value
 * Return Value: returns 0 upon success, -1 upon failure
 * Function: changes RTC frequency */
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes);
