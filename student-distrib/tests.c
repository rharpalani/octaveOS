#include "tests.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* ****************** */
/* CHECKPOINT 1 TESTS */
/* ****************** */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	if (result==FAIL){
		return result;
	}
	
	return result;
}

/* Divide by Zero Test
 * 
 * Asserts that a division by zero error occurs correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: divides integers by zero
 * Files: idt.c/h
 */
void div_zero_test(){
	TEST_HEADER;
	int i = 10;
	int j = 0;
	int result;
	
	result = i / j;
}

/* Paging Test
 * 
 * Asserts that pagefault does not occur
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: puts something into video memory
 * Files: paging.c/h
 */
int paging_tests() {

	clear(); 

	// uint8_t* video = (uint8_t*)(VIDEO + 1024*4); // should pagefault
	uint8_t* video = (uint8_t*)(VIDEO); // should not pagefault
	putc(video[2]);
	video[2] = 'a';
	putc(video[2]);

	return PASS;
}

/* ****************** */
/* CHECKPOINT 2 TESTS */
/* ****************** */

/* RTC Test
 * 
 * Asserts that RTC frequency can be read and written
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: prints "1" for various RTC frequencies
 * Files: rtc.c/h
 */
int rtc_test() {
	TEST_HEADER;

	clear();
	int openval = 0;
	int writeval = 0;
	int readval = 0;

	// pcb_t temp_pcb;
	// int32_t pcb_ptr = (int32_t)(&temp_pcb);

	openval = rtc_open(NULL);

	int i;
	int j;
	for (i = 2; i <= 1024; i *= 2) {	// iterate through all frequencies
		writeval += rtc_write(1, &i, 1);
		for (j = 0; j < i; j++) {
			putc(49);	// 49 is ASCII 1
			readval += rtc_read(NULL, NULL, NULL);
			
		}
		clear();
	}

	// check return values of each function call -- should be 0 for all
	if (openval == 0 && writeval == 0 && readval == 0) {
		return PASS;
	} else {
		return FAIL;
	}
}

/* Terminal Test
 * 
 * Asserts the keyboard buffer can successfully be copied into the arg buffer upon pressing enter
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: copies keyboard buffer into arg buffer and echoes to screen
 * Files: terminal.c/h
 */
int terminal_test() {
	TEST_HEADER;
	clear();
	unsigned char testbuf[127];
	while(1) {
		term_read(0, testbuf, 127);
	}
	term_write(0, testbuf, 127);
	return PASS;
}

/* Terminal Read Test
 * 
 * Asserts the keyboard buffer can successfully be copied into the arg buffer upon pressing enter
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: copies keyboard buffer into arg buffer
 * Files: terminal.c/h
 */
int term_read_test() {
	TEST_HEADER;
	clear();
	int val = 0;
	unsigned char testbuf[128];
	int i;
	for (i = 0; i < 128; i++) {
		testbuf[i] = 0;
	}
	val = val + term_read(0, testbuf, 128) - 128;
	for (i = 0; i < 128; i++) {
		putc(testbuf[i]);
	}
	val += term_read(NULL, NULL, NULL);
	printf("\n");
	if (val == 0) {
		return PASS;
	} else {
		return FAIL;
	}
}


/* Terminal Write Test
 * 
 * Asserts that the contents of the arg buffer can be printed to the screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: uses putc to echo contents of arg buffer to screen
 * Files: terminal.c/h
 */
int term_write_test() {
	TEST_HEADER;
	clear();
	int val = 0;
	unsigned char testbuf[12] = "Hello world!";
	val = val + term_write(0, testbuf, 12) - 12;	// check terminal write
	printf("\n");
	val += term_write(NULL, NULL, NULL);			// NULL check

	if (val == 0) {
		return PASS;
	} else {
		return FAIL;
	}
}

/* Terminal Buffer Test
 * 
 * Asserts that the terminal can handle buffer overflow
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: puts a value larger than 128 into the read buffer
 * Files: terminal.c/h
 */
int term_buffer_test() {
	
	int val = term_read(0, NULL, 130);

	if (val == -1) {
		return PASS;
	} else {
		return FAIL;
	}
}

/* Directory Test
*  DESCRIPTION: Asserts that all file names are able to get accessed and displays onto screen
*  INPUTS: none
*  OUTPUTS: PASS
*  Side Effects: None
*  COVERAGE: Outputs names of all files in fsdir
*  FILES: filesys.c/h
*/
// int directory_tests (){
// 	TEST_HEADER;
// 	int i;
// 	unsigned int j; 
// 	char buf[33];

// 	// int space[2];

// 	pcb_t temp_pcb;
// 	int32_t pcb_ptr = (int32_t)(&temp_pcb);

// 	clear();

// 	int32_t fd = dopen(pcb_ptr); 
// 	// printf("fd in directory open test is %d\n", fd);
	

// 	// printf("Starting file name prints\n");
// 	for (i = 0; i < 20; ++i){
// 		memset(buf, 0, 32);
// 		dread(fd, buf, 32, pcb_ptr);
// 		buf[32] = 0;
// 		printf(buf);
// 		printf("\n");
		
// 	}

// 	dclose(fd, pcb_ptr);

// 	for (j = 0; j < 1000000000; ++j);
	

// 	return PASS; 

// }

// THESE TESTS are no longer valid, because they would use up the kernel pcb I think
/* Read File By Name Test
*  DESCRIPTION: Checks if can access file contents given name
*  INPUTS: none
*  OUTPUTS: PASS
*  Side Effects: None
*  COVERAGE: Displays contents of frame1.txt then verylargetextwithverylongname.txt
*  FILES: filesys.c/h
// */
// int read_file_by_name(){
// 	TEST_HEADER;
// 	// int space[3]; 
// 	pcb_t temp_pcb;
// 	int32_t pcb_ptr = (int32_t)(&temp_pcb);

// 	char buf[5310];
// 	int x;
// 	unsigned int j; 
// 	memset(buf, 0, 5310);
// 	clear();

// 	int32_t fd = fopen((uint8_t*)"frame1.txt");
// 	// printf("fd in frame1.txt open test is %d\n", fd);
// 	// while (1);
// 	x = fread(fd,buf,175, pcb_ptr);
// 	// read a few lines fromm it into a buffer

// 	printf(buf);
// 	printf("\n\n%d",x);
// 	fclose(fd, pcb_ptr);

// 	for (j = 0; j < 1000000000; ++j);

// 	fd = fopen((uint8_t*)"verylargetextwithverylongname.txt");
// 	// printf("fd in longfile open test is %d", fd);
// 	x = fread(fd,buf,5310, pcb_ptr);
// 	// read a few lines fromm it into a buffer
// 	clear();
// 	printf(buf);
// 	// printf("\n\n%d",x);
// 	fclose(fd, pcb_ptr);

// 	return PASS;
// }

// /* Checkpoint 3 tests */
// int open_many_dirs(){
// 	TEST_HEADER;
// 	int i;
// 	int32_t fd;
// 	int opened_count = 0;
	
// 	pcb_t temp_pcb;
// 	int32_t pcb_ptr = (int32_t)(&temp_pcb);

// 	clear();
	
// 	for (i = 0; i < 12; i++){
// 		fd = dopen(pcb_ptr);
// 		if (fd >= 2) {
// 			opened_count++;
// 		}
// 		printf("On %dth iteration, fd returned %d\n", i, fd);
// 	}
	
// 	if (opened_count==6){
// 		return PASS;
// 	}

// 	return FAIL;

// }


// int open_and_close_many_dirs(){
// 	TEST_HEADER;
// 	int i;
// 	int32_t fd;
// 	int opened_count = 0;
	
// 	pcb_t temp_pcb;
// 	int32_t pcb_ptr = (int32_t)(&temp_pcb);

// 	clear();
	
// 	for (i = 0; i < 12; i++){
// 		if (i+1>=6){
// 			dclose(2 + (i%6),pcb_ptr);
// 			printf("Then closed fd %d\n",2 + (i%6));
// 		}
// 		fd = dopen(pcb_ptr);
// 		if (fd >= 2) {
// 			opened_count++;
// 		}
// 		printf("On %dth iteration, fd returned %d\n", i+1, fd);
// 	}


// 	fd = dopen(pcb_ptr);
// 	printf("On final iteration (should be -1), fd returned %d\n", fd);
	

// 	if (opened_count==12){
// 		return PASS;
// 	}

// 	return FAIL;

// }


/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests(){
	/* CHECKPOINT 1 TESTS */
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("paging_tests", paging_tests());
	// TEST_OUTPUT("div_zero_test", div_zero_test());

	/* CHECKPOINT 2 TESTS */
	// TEST_OUTPUT("rtc_test", rtc_test());
	// TEST_OUTPUT("terminal_test", terminal_test());
	// TEST_OUTPUT("term_buffer_test", term_buffer_test());
	// TEST_OUTPUT("term_read_test", term_read_test());
	// TEST_OUTPUT("term_write_test", term_write_test());
	// TEST_OUTPUT("directory_tests", directory_tests());
	// TEST_OUTPUT("read_file_by_name", read_file_by_name());
	// TEST_OUTPUT("open_many_dirs", open_many_dirs());
	// TEST_OUTPUT("open_and_close_many_dirs", open_and_close_many_dirs());
}
