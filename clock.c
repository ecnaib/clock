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
//need to update input_f as well when changing display height
#define DISPLAY_HEIGHT FONT_ROWS

sem_t s_btn, q_btn;

void*timer_f();
void*clock_f();
void*input_f();
void up_time(struct tm* time);
void sec_to_timer(struct tm *ris, time_t t_start);
void time_printer(struct tm *time);


int main(int argc, char**argv){
  pthread_t main_thread, input_thread;
  sem_init(&s_btn, 0, 1);
  sem_init(&q_btn, 0, 1);
  pthread_create(&input_thread, NULL, input_f, NULL);
  pthread_create(&main_thread, NULL, timer_f, NULL);
  pthread_join(main_thread, NULL);
  pthread_join(input_thread, NULL);
  return 0;
}

//@ funzioni core

void *timer_f(){
  int s_btn_value, q_btn_value;
  time_t start_time = time(NULL);
  time_t curr_time;
  time_t displayed;
  struct tm t; 
  sem_getvalue(&s_btn, &s_btn_value);
  sem_getvalue(&q_btn, &q_btn_value);

  while(q_btn_value) {
    if (s_btn_value){
      curr_time = time(NULL);
      displayed = curr_time - start_time;
      sec_to_timer(&t, displayed);
      time_printer(&t);
      sleep(1);
    } else {
      start_time = time(NULL) - displayed;
    }
    sem_getvalue(&s_btn, &s_btn_value);
    sem_getvalue(&q_btn, &q_btn_value);
  }
  return NULL;
}

void *clock_f(){
  int q_btn_value;
  time_t curr_time;
  struct tm *curr_hour;
  sem_getvalue(&q_btn, &q_btn_value);
  while (q_btn_value){
    curr_time = time(NULL);
    curr_hour = localtime(&curr_time);
    time_printer(curr_hour);
    sleep(1);
    sem_getvalue(&q_btn, &q_btn_value);
  }
  return NULL;
}

//@ funzione input

void *input_f(){
  int s_value = 1;
  char command[2];
  while(1){
    fgets(command, sizeof(command), stdin);
    //6H needs to change when changinng display_height
    printf("\33[6H\33[2K");
    switch (command[0]){
      case 'q':
          sem_wait(&q_btn);
          return NULL;
      case 's':
        if (s_value) {
          s_value = 0;
          sem_wait(&s_btn);
        } else{
          s_value = 1;
          sem_post(&s_btn);
        }
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

void sec_to_timer(struct tm *ris, time_t time){
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
  printf("\33[H\33[J");
  for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < DISPLAY_WIDTH; ++x) {
      int i = x/(FONT_COLS + DIGITS_PAD);
      int dx = x%(FONT_COLS + DIGITS_PAD);
      if (dx < FONT_COLS && (font[digits[i]]>>((FONT_ROWS - y - 1)*FONT_COLS + dx))&1) {
        printf("\33[1;31m█\33[0m");
      } else {
        printf("\33[1;36m█\33[0m");
      }
    }
    printf("\n");
  }
  //printf("\033[%dA\033[%dD", DISPLAY_HEIGHT, DISPLAY_WIDTH);
}
