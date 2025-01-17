#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define FONT_COUNT 11
int font[FONT_COUNT] = {31599,19812,14479,31207,23524,29411,29679,30866,31727,31719,1040};
#define FONT_ROWS 5
#define FONT_COLS 3
#define DIGITS_COUNT 8
#define DIGITS_PAD 2
#define DISPLAY_WIDTH (FONT_COLS + DIGITS_PAD)*DIGITS_COUNT
#define DISPLAY_HEIGHT FONT_ROWS

sem_t sem1, sem2;

void*timer();
void*input_f();
void up_time(struct tm* time);
void sec_to_timer(struct tm *ris, time_t t_start);
void time_printer(struct tm *time);


int main(int argc, char**argv){
  pthread_t main_thread, input_thread;
  sem_init(&sem1, 0, 1);
  sem_init(&sem2, 0, 1);
  pthread_create(&input_thread, NULL, input_f, NULL);
  pthread_create(&main_thread, NULL, timer, NULL);
  pthread_join(main_thread, NULL);
  pthread_join(input_thread, NULL);
  return 0;
}

//@ funzioni core

void *timer(){
  int sem1_value, sem2_value;
  //time_t t = time(NULL);
  //struct tm *time = localtime(&t);
  struct tm time; 
  sem_getvalue(&sem1, &sem1_value);
  sem_getvalue(&sem2, &sem2_value);
 
  while(sem2_value) {
    if (sem1_value){
      //sec_to_timer(time, t);
      time_printer(&time);
      up_time(&time);
      sleep(1);
    }
    sem_getvalue(&sem1, &sem1_value);
    sem_getvalue(&sem2, &sem2_value);
  }
  return NULL;
}

//@ funzione input

void *input_f(){
  int sem_value = 1;
  char command[2];
  while(1){
    scanf("%s", command);
    if (command[0] == 'q'){
      sem_wait(&sem2);
      return NULL;
    }
    if (sem_value) {
      sem_value = 0;
      sem_wait(&sem1);
    } else{
      sem_value = 1;
      sem_post(&sem1);
    }
  }
  return NULL;
}


//@ funzioni ausiliarie

void up_time(struct tm *time){
  int new_sec = time->tm_sec + 1;
  int new_min = time->tm_min + new_sec/60;
  int new_hour = time->tm_hour + new_min/60;

  time->tm_hour = new_hour;
  time->tm_min = new_min % 60;
  time->tm_sec = new_sec % 60;
}

void sec_to_timer(struct tm *ris, time_t t_start){
	time_t t_now = time(NULL);
	int time = (int)difftime(t_now, t_start);
	ris->tm_sec = time%60;
	time = time/60;
	ris->tm_min = time%60;
	ris->tm_hour = time/60;
}

void time_printer(struct tm *time){
  int digits[DIGITS_COUNT];
  digits[0] = time->tm_hour/10;
  digits[1] = time->tm_hour%10;
  digits[2] = 10;
  digits[3] = time->tm_min/10;
  digits[4] = time->tm_min%10;
  digits[5] = 10;
  digits[6] = time->tm_sec/10;
  digits[7] = time->tm_sec%10;
  printf("\033[H\033[J");
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      int i = x/(FONT_COLS + DIGITS_PAD);
      int dx = x%(FONT_COLS + DIGITS_PAD);
      if (dx < FONT_COLS && (font[digits[i]]>>((FONT_ROWS - y - 1)*FONT_COLS + dx))&1) {
        printf("\033[1;31m█\033[0m");
      } else {
        printf("█");
      }
    }
    printf("\n");
  }
  //printf("\033[%dA\033[%dD", DISPLAY_HEIGHT, DISPLAY_WIDTH);
}
