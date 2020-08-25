#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
// define wiringPi pins for LEDs
#define LED0 0
#define LED1 2
#define LED2 3
#define LED3 4
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
int num;
double offset = 0;
int den;
int bpm;
int avg = 0;
int pianoBoolean = 0;
int seqBoolean = 0;

typedef struct{
		int nums;
		int dens;
		int bars;
}sequence;

sequence *seqs[10];





int power(int base, unsigned int exp) {
    int i, result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
 }


char *readFile(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;
	int i = 0;
	int tempFlag = 0;
	int sigFlag = 0;
    if (file == NULL)
        return NULL; //could not open file

    code = malloc(1000);

    while ((c = fgetc(file)) != EOF)
    {
		if (c == 0xff) {
			c = fgetc(file);
			if (c == 0x58 && sigFlag == 0) {
				c = fgetc(file);
				num = fgetc(file);
				den = power(2,fgetc(file));
				sigFlag = 1;
			} else if (c == 0x51 && tempFlag == 0){
				fgetc(file);
				int temp = 0;
				temp = (fgetc(file) << 16) & 0xff0000;
				temp += (fgetc(file) << 8) & 0xffff00;
				temp += (fgetc(file) << 0) & 0xffffff;
				//printf("%x ", temp);
				//printf("%x ", fgetc(file));
				//printf("%x ", fgetc(file));
				//printf("%x ", fgetc(file));
				bpm = (int)(60000000/temp);
				offset = 60000/bpm*4/den;
				tempFlag = 1;
			}
		}
        
        code[n++] = (char) c;
    }

    // don't forget to terminate with the null character
    code[n] = '\0';        

    return code;
}



/* sleep until a key is pressed and return value. echo = 0 disables key echo. */
int keypress(unsigned char echo) {
    struct termios savedState, newState;
    int c;
    if (-1 == tcgetattr(STDIN_FILENO, &savedState)){
        return EOF;     /* error on tcgetattr */
    }
    newState = savedState;
    if ((echo = !echo)) /* yes i'm doing an assignment in an if clause */{
        echo = ECHO;    /* echo bit to disable echo */
    }
    /* disable canonical input and disable echo.  set minimal input to 1. */
    newState.c_lflag &= ~(echo | ICANON);
    newState.c_cc[VMIN] = 1;
    if (-1 == tcsetattr(STDIN_FILENO, TCSANOW, &newState)){
        return EOF;     /* error on tcsetattr */
    }
    c = getchar();      /* block (withot spinning) until we get a keypress */
    /* restore the saved state */
    if (-1 == tcsetattr(STDIN_FILENO, TCSANOW, &savedState)){
        return EOF;     /* error on tcsetattr */
    }
    return c;
}

void *piano(void *str){
	pianoBoolean = 1;
	struct timeval stop, start;
	int type = 0;
	int i = 0;
	int sumBpm[6] = {0,0,0,0,0,0};
	while(1){
		gettimeofday(&start, NULL);
		char testChar = keypress(0);
		if (testChar == 'q'){
			system("mpg321 c.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'w'){
			system("mpg321 d.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'e'){
			system("mpg321 e.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'r'){
			system("mpg321 f.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 't'){
			system("mpg321 g.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'y'){
			system("mpg321 a.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'u'){
			system("mpg321 b.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'i'){
			system("mpg321 c2.mp3  > trash.txt 2>&1 &");
		} else if (testChar == '2'){
			system("mpg321 c#.mp3  > trash.txt 2>&1 &");
		} else if (testChar == '3'){
			system("mpg321 d#.mp3  > trash.txt 2>&1 &");
		} else if (testChar == '5'){
			system("mpg321 f#.mp3  > trash.txt 2>&1 &");
		} else if (testChar == '6'){
			system("mpg321 g#.mp3  > trash.txt 2>&1 &");
		} else if (testChar == '7'){
			system("mpg321 a#.mp3  > trash.txt 2>&1 &");
		} else if (testChar == 'n'){
			exit(1);
			return NULL;
		}
		gettimeofday(&stop, NULL);
		unsigned long sec = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
		if (sec > ((offset*4*1000)-(offset*4*1000*0.22))){
			type = 1;
		} else if (sec > ((offset*2*1000)-(offset*2*1000*0.3))){
			type = 2;
		} else if (sec > ((offset*1*1000)-(offset*1*1000*0.3))){
			type = 4;
		} else if (sec > ((offset*0.5*1000)-(offset*0.5*1000*0.3))){
			type = 8;
		} else if (sec > ((offset*0.25*1000)-(offset*0.25*1000*0.3))){
			type = 16;
		} else if (sec > ((offset*0.128*1000)-(offset*0.128*1000*0.3))){
			type = 32;
		} else { 
			type = 64;
		}
		sumBpm[i%6] = (int)(60000000.0/sec*4/type);
		avg = 0;
		i++;
		avg = (sumBpm[0] + sumBpm[1] + sumBpm[2] + sumBpm[3] + sumBpm[4] + sumBpm[5])/6*4/den;
		//printf("avg: %d", avg);

		
	}
}


int bit_test(int value, int bit_pos) {
    return CHECK_BIT(value, bit_pos);
}

void tapBPM(){  
	struct timeval stop, start;
	int sumBpm = 0;
	int i = 0;
	int avgl = 0;
	keypress(0);
	printf("Use Spacebar to tap tempo:\n");
	//for (int i = 8; i > 0; i--){
	while (1){
		i++;
		gettimeofday(&start, NULL);
		char testChar = keypress(0);
		if (testChar == 'n'){
			printf("Tempo set to: %d\n", avgl);
			bpm = avgl;
			return;
		}
		gettimeofday(&stop, NULL);
		unsigned long sec = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
		sumBpm += (int)(60000000.0/sec);
		avgl = sumBpm/i;
		printf("%-3d BPM\n", avgl);
	}

	return;
}


void *metronome(void *vargp) {
	int beat = 1;
	int bi = 0;
	int i = 0;
	printf("*1\n");
	while(1){
		beat = i%num;
		bi = power(2, beat);
		int led0val = (bit_test(bi, 0) || bit_test(bi, 4) || bit_test(bi, 8) || bit_test(bi, 12));
		int led1val = (bit_test(bi, 1) || bit_test(bi, 5) || bit_test(bi, 9) || bit_test(bi, 13));
		int led2val = (bit_test(bi, 2) || bit_test(bi, 6) || bit_test(bi, 10) || bit_test(bi, 14));
		int led3val = (bit_test(bi, 3) || bit_test(bi, 7) || bit_test(bi, 11) || bit_test(bi, 15));
		digitalWrite(LED0, led0val);
		digitalWrite(LED1, led1val);
		digitalWrite(LED2, led2val);
		digitalWrite(LED3, led3val);
		if (i%num == 0){
			system("mpg321 bar.mp3  > trash.txt 2>&1 &");
		} else {
			system("mpg321 beat.mp3 > trash.txt 2>&1 &");
		}
		delay(5*offset/6);
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 0);
		delay(offset/6);
		i++;
		if (beat == num-1){
			printf("*%d\t", ((i/num)+1));
			if (pianoBoolean == 1){
				if (avg > (bpm+4)){
					printf("Slow Down");
				} else if (avg < (bpm-4)){
					printf("Speed Up");
				}
			}
			printf("\n");
		}
	} 

}


void setBPM(){
    printf("Enter BPM: \n");
    scanf("%d", &bpm);

}

void options(){
	printf("BPM:      %d\n", bpm);
	printf("Time Sig: %d/%d\n", num, den);
	printf("Offset: %f\n", offset);
}

void setSig(){
	if (offset == 0){
		printf("Set BPM first\n");
	}
	printf("Enter time signature numerator: \n");
	scanf("%d", &num);
	printf("Enter time signature denominator: \n");
	scanf("%d", &den);
	offset = 60000/bpm*4/den;	
}

void sequencer(){
	for (int i = 0; i < 10; i++){
		seqs[i] = NULL;
	}
	for (int i = 0; i < 10; i++){
		sequence *s = malloc(sizeof(sequence));
		printf("Enter time signature numerator:\n");
		int num1;
		scanf("%d",&num1);
		printf("Enter time signature denominator:\n");
		int den1;
		scanf("%d",&den1);
		printf("Enter number of bars:\n");
		int bars;
		scanf("%d",&bars);
		s->nums = num1;
		s->dens = den1;
		s->bars = bars;
		seqs[i] = s;
		printf("Create another sequence? [y/n]\n");
		char ans[1];
		scanf("%s", ans);
		if (strcmp(ans, "n") == 0){
			return;
		}
	}
}

void showSeq(){
	if (seqs[0] == NULL){
		printf("NULL\n");
		return;
	}
	for (int i = 0; i < 10; i++){
		if (seqs[i] != NULL){
			printf("%d/%d:%d bars\n", seqs[i]->nums, seqs[i]->dens, seqs[i]->bars);
		}
	}
	return;
}

void *playSeq(void *str){
	printf("\n");
	seqBoolean = 1;
	int bar = 0;
	printf("\n");
	for (int k = 0; k < 10; k++){
		if (seqs[k] == NULL){
			printf("Done\n");
			return NULL;
		}
		printf("%d/%d:%d bars\n", seqs[k]->nums, seqs[k]->dens, seqs[k]->bars);
		bar = 0;
			num = seqs[k]->nums;
			den = seqs[k]->dens;
			offset = offset = 60000/bpm*4/den;
			int beat = 1;
			int bi = 0;
			int i = 0;
			printf("*1\n");
			while(bar != seqs[k]->bars){
				beat = i%num;
				bi = power(2, beat);
				int led0val = (bit_test(bi, 0) || bit_test(bi, 4) || bit_test(bi, 8) || bit_test(bi, 12));
				int led1val = (bit_test(bi, 1) || bit_test(bi, 5) || bit_test(bi, 9) || bit_test(bi, 13));
				int led2val = (bit_test(bi, 2) || bit_test(bi, 6) || bit_test(bi, 10) || bit_test(bi, 14));
				int led3val = (bit_test(bi, 3) || bit_test(bi, 7) || bit_test(bi, 11) || bit_test(bi, 15));
				digitalWrite(LED0, led0val);
				digitalWrite(LED1, led1val);
				digitalWrite(LED2, led2val);
				digitalWrite(LED3, led3val);
				if (i%num == 0){
					system("mpg321 bar.mp3  > trash.txt 2>&1 &");
				} else {
					system("mpg321 beat.mp3 > trash.txt 2>&1 &");
				}
				delay(5*offset/6);
				digitalWrite(LED0, 0);
				digitalWrite(LED1, 0);
				digitalWrite(LED2, 0);
				digitalWrite(LED3, 0);
				delay(offset/6);
				i++;
				if (beat == num-1){
					bar++;
					printf("*%d\t", ((i/num)+1));
					if (pianoBoolean == 1){
						if (avg > (bpm+4)){
							printf("Slow Down");
						} else if (avg < (bpm-4)){
							printf("Speed Up");
						}
					}
					printf("\n");
				}
			
		}
	} 
}

int main(void) {
		

    // Setup WiringPi
    if (wiringPiSetup() == -1)
      return 1;

    // Set output pins
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
   
	//num = 4;
	//den = 4;
	//bpm = 120;
	//offset = 60000/bpm*4/den;		

	//readFile("test.mid");
	//exit(1);

	while (1) {
		char cmd[100], file_name[100];
		printf("Enter Cmd:\n");
		scanf("%s", cmd);
		if (strcmp(cmd, "set") == 0) {
			printf("BPM or Sig?\n");
			scanf("%s", cmd);
			if (strcmp(cmd, "bpm") == 0) {
				setBPM();
			} else if (strcmp(cmd, "sig") == 0) {
				setSig();
			} else {
				printf("invalid\n");
			}
		} else if (strcmp(cmd, "tapBpm") == 0) {
					tapBPM();
		} else if (strcmp(cmd, "options") == 0) {
			options();
		} else if (strcmp(cmd, "load") == 0) {
			printf("Enter file name: \n");
			scanf("%s", file_name);
			readFile(file_name);
		} else if (strcmp(cmd, "playSong") == 0) {
			printf("Enter file name: \n");
			scanf("%s", file_name);
			//sprintf(file_name, "mpg321 .mp3  > trash.txt 2>&1 &", a);
			//system(buf);
			system("mpg321 wii.mp3  > trash.txt 2>&1 &");
			metronome(NULL);
		} else if (strcmp(cmd, "play") == 0) {
			metronome(NULL);
		} else if (strcmp(cmd, "piano") == 0) {
			pthread_t thread1, thread2; 
			pthread_create(&thread1, NULL, metronome, NULL);
			pthread_create(&thread2, NULL, piano, NULL); 
			pthread_join(thread1, NULL); 
			pthread_join(thread2, NULL); 
		} else if (strcmp(cmd, "sequencer") == 0){ 
			sequencer();
		} else if (strcmp(cmd, "showSeq") == 0){ 
			showSeq();
		} else if (strcmp(cmd, "playSeq") == 0){
			playSeq(NULL);
		} else if (strcmp(cmd, "pianoSeq") == 0){
			pthread_t thread1, thread2; 
			pthread_create(&thread1, NULL, playSeq, NULL);
			pthread_create(&thread2, NULL, piano, NULL); 
			pthread_join(thread1, NULL); 
			pthread_join(thread2, NULL); 
		} else if (strcmp(cmd, "help") == 0){
			printf("-set\n\t-bpm\n\t-sig\n-tapBpm\n-options\n-load\n-play\n-piano\n-sequencer\n-showSeq\n-playSeq\n"); 
		} else {
			printf("invalid cmd\n");
		}
	}
    return 0;
  }
