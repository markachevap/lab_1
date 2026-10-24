

Семинар 3. Pthreads: барьеры и условные
переменные
Барьер — это фрагмент кода, который характеризуется тем, что ни
один из потоков не выйдет из него, пока последний не зашел.
1  Холостой цикл
Самый простой способ организации барьера — это использование хо-
лостого цикла. Минусы такого подхода:
- Значительное падение производительности в случае, когда количе-
ство запущенных потоков превышает количество физических. Это
связано с тем, что проходит значительное время, прежде чем опера-
ционная система догадается, что поток ничего не делает и висит в
бездействии, и отключит его, чтобы дать возможность поработать
другим потокам.
- Компилятор не знает, что мы пишем многопоточную программу.
Если включить оптимизацию (использовать флаг -O2), то компи-
лятор может переставить некоторые строки, которые, с его точки
зрения, не зависят друг от друга, и тогда программа перестанет
работать.
- Мы не можем синхронно обнулить counter, поэтому эта переменная
является одноразовой. Нам придется создавать массив счетчиков,
если барьеров несколько.
Листинг 1: Организация барьера с помощью холостого цикла
#include <pthread . h>
## 1

#include <stdio . h>
#include <s t d l i b . h>
#include " timer . h"
#define NTHREADS 4
#define NBARRIERS 10000
pthread_mutex_t mutex ;
int  counter [NBARRIERS ] ;
void ∗ routine (void ∗rank ){
for  ( int  j = 0;  j < NBARRIERS;  j++){
pthread_mutex_lock(&mutex ) ;
counter [ j ]++;
pthread_mutex_unlock(&mutex ) ;
while ( counter [ j ]  != NTHREADS) ;
## }
return NULL;
## }
int main (){
pthread_t  thread_handles [NTHREADS] ;
pthread_mutex_init(&mutex , NULL) ;
double  start ,  end ;
GET_TIME( s t a r t ) ;
for  ( int  i = 0;  i < NTHREADS;  i++)
pthread_create(&thread_handles [ i ] , NULL,  routine , NULL) ;
for  ( int  i = 0;  i < NTHREADS;  i++)
pthread_join ( thread_handles [ i ] , NULL) ;
GET_TIME( end ) ;
p r i n t f ( "%e\n" ,  end − s t a r t ) ;
pthread_mutex_destroy(&mutex ) ;
## }
## 2  Семафоры
## 2

Листинг 2: Организация барьера с помощью семафоров
1 #include <pthread . h>
2 #include <stdio . h>
3 #include <s t d l i b . h>
4 #include " timer . h"
5 #include <semaphore . h>
## 6
7 #define NTHREADS 4
8 #define NBARRIERS 10000
9  pthread_mutex_t mutex ;
10  sem_t sem_barrier [NBARRIERS ] ;
11  sem_t sem_count ;
12  int  counter = 0;
## 13
14  void ∗ routine (void ∗rank ){
15    for  ( int  j = 0;  j < NBARRIERS;  j++){
## 16sem_wait(&sem_count ) ;
17i f  ( counter == NTHREADS − 1){
## 18counter = 0;
## 19sem_post(&sem_count ) ;
20for  ( int  i = 0;  i < NTHREADS − 1;  i++)
21sem_post(&sem_barrier [ j ] ) ;
## 22}
## 23else {
## 24counter++;
## 25sem_post(&sem_count ) ;
26sem_wait(&sem_barrier [ j ] ) ;
## 27}
## 28}
29    return NULL;
## 30  }
## 31
32  int main (){
33pthread_t  thread_handles [NTHREADS] ;
34pthread_mutex_init(&mutex , NULL) ;
35    for ( int  i =0;  i<NBARRIERS;  i++)
36sem_init(&sem_barrier [ i ] ,  0 ,  0 ) ;
## 3

## 37sem_init(&sem_count ,  0 ,  1 ) ;
38    double  start ,  end ;
39    GET_TIME( s t a r t ) ;
40    for  ( int  i = 0;  i < NTHREADS;  i++)
41pthread_create(&thread_handles [ i ] , NULL,  routine , NULL) ;
## 42
43    for  ( int  i = 0;  i < NTHREADS;  i++)
44pthread_join ( thread_handles [ i ] , NULL) ;
45    GET_TIME( end ) ;
46p r i n t f ( "%e\n" ,  end − s t a r t ) ;
47    for ( int  i =0;  i<NBARRIERS;  i++)
48sem_destroy(&sem_barrier [ i ] ) ;
## 49sem_destroy(&sem_count ) ;
## 50pthread_mutex_destroy(&mutex ) ;
## 51  }
По прохождению барьера в программе 2 обнуляется counter, а сема-
фор sem_count опять принимает значение 1. Однако семафор barrier_sem
может не занулиться. Именно, может реализоваться следующий сцена-
рий: потоки, все, кроме последнего, зашедшего в барьер, уснут в стро-
ке 31. Операционная система, заметив бездействие потоков, временно
прекратит их работу. Последний поток в 25 строке поднимет семафор,
пробуждая зависшие потоки. Однако, если потоки отключены от плани-
ровщика задач, то они не увидят поднятых семафоров. Тогда последний
поток, подняв семафор некоторое количество раз, продолжит работу, а
значение barrier_sem будет отлично от нуля. В этом случае последний
поток продолжит работу, дойдет до второго барьера, а т.к. barrier_sem
не занулен, то этот поток пойдет дальше второго барьера, к третьему, а
зависшие в первом барьере потоки так его и не преодолеют.
Следовательно, барьерный семафор является одноразовым. Его нель-
зя переиспользовать.
3  Условные переменные
Для реализации барьера, которая была бы лишена выявленных выше
недостатков, будем использовать новый тип данных — условную пере-
менную, pthread_cond_t.
Инициализация условной переменной
## 4

int  pthread_cond_init (
pthread_cond_t∗    cond_p ,    /∗ out∗/
pthread_condattr_t   cond_attr    /∗ in∗/
## ) ;
Освобождение ресурсов, занимаемых условной переменной
int  pthread_cond_destroy (
pthread_cond_t∗    cond_p/∗ in /out∗/
## ) ;
Условная переменная используется для того, чтобы сигнализировать
потокам о том, что событие, которое символизирует данная условная
переменная, произошло.
Условная переменная имеет тип pthread_cond_t
Условная переменная используется всегда в связке с мьютексом. При-
мер использования условной переменной для реализации барьера пока-
зан в листинге 3.
Обратим внимание на то, что в строке 29 вызов функции pthre-
ad_cond_wait поставлен в цикл while. Это сделано для того, чтобы от-
личить «пробуждение» потока вследствие срабатывания условной пере-
менной от самопроизвольного пробуждения, вызванного другими при-
чинами (spurious wakeup).
Процессы ждут связанного с условной переменной события в функ-
ции pthread_cond_wait:
pthread_cond_wait (
pthread_cond_wait∗ cond ,/∗ in /out∗/
pthread_mutex_t∗ mutex/∗ in /out∗/
## ) ;
Функция pthread_cond_wait производит следующие действия:
pthread_mutex_unlock(&mutex ) ;
wait_on_signal(&cond ) ;
pthread_mutex_lock(&mutex ) ;
Для того, чтобы сигнализировать одному какому-то потоку, что собы-
тие, связанное с условной переменной, произошло, используется функ-
ция
pthread_cond_signal (
pthread_cond_t∗ cond/∗ in /out∗/
## 5

## ) ;
При вызове этой функции какой-то один из потоков, заблокированных
в pthread_cond_wait (точнее, в ее части wait_on_signal), проснется, и
продолжит выполнение своих инструкций. Если же мы хотим пробудить
все уснувшие потоки, то нужно использоваться функцию
pthread_cond_broadcast (
pthread_cond_t∗ cond/∗ in /out∗/
## ) ;
Таким образом, общую схему работы с условными переменными мож-
но описать следующим псевдокодом:
lock  mutex
i f  condition  has  occured
s i g n a l  thread ( s )
else
unlock mutex
wait_on_signal
lock  mutex
unlock mutex
Листинг 3: Барьер с использованием условных переменных
1 #include <pthread . h>
2 #include <stdio . h>
3 #include <s t d l i b . h>
4 #include " timer . h"
5 #include <semaphore . h>
## 6
7 #define NTHREADS 4
8 #define NBARRIERS 10000
9  pthread_mutex_t mutex ;
10  int  counter ;
11  pthread_cond_t cond ;
## 12
13  void ∗ routine (void ∗rank ){
14    for  ( int  j = 0;  j < NBARRIERS;  j++){
## 15pthread_mutex_lock(&mutex ) ;
## 16counter++;
## 6

17i f  ( counter == NTHREADS){
## 18counter = 0;
## 19pthread_cond_broadcast(&cond ) ;
## 20}
21else while ( pthread_cond_wait(&cond , &mutex)  !=  0 ) ;
## 22pthread_mutex_unlock(&mutex ) ;
## 23}
24    return NULL;
## 25  }
## 26
27  int main (){
28pthread_t  thread_handles [NTHREADS] ;
29pthread_cond_init(&cond , NULL) ;
30pthread_mutex_init(&mutex , NULL) ;
31    double  start ,  end ;
32    GET_TIME( s t a r t ) ;
33    for  ( int  i = 0;  i < NTHREADS;  i++)
34pthread_create(&thread_handles [ i ] , NULL,  routine , NULL) ;
35    for  ( int  i = 0;  i < NTHREADS;  i++)
36pthread_join ( thread_handles [ i ] , NULL) ;
37    GET_TIME( end ) ;
38p r i n t f ( "%e\n" ,  end − s t a r t ) ;
## 39pthread_cond_destroy(&cond ) ;
## 40pthread_mutex_destroy(&mutex ) ;
## 41  }
В качестве еще одного примера рассмотрим применение условных пе-
ременных к задаче передачи сообщений по кругу, см. листинг 4.
Листинг 4: Использование условных переменных для циклической пере-
дачи сообщений
#include <stdio . h>
#include <s t d l i b . h>
#include <pthread . h>
const int MAX_THREADS = 1024;
const int MSG_MAX = 100;
int  thread_count ;
## 7

char∗∗  messages ;
pthread_cond_t∗ cond ;
pthread_mutex_t mutex ;
void ∗Send_msg(void∗ rank ) ;
int main ( int  argc , char∗ argv [ ] )  {
longthread ;
pthread_t∗  thread_handles ;
thread_count = s t r t o l ( argv [ 1 ] , NULL,  10);
thread_handles = malloc  ( thread_count∗ sizeof ( pthread_t ) ) ;
messages = malloc ( thread_count∗ sizeof (char ∗ ) ) ;
cond = malloc ( thread_count∗ sizeof ( pthread_cond_t ) ) ;
pthread_mutex_init(&mutex , NULL) ;
for  ( thread = 0;  thread < thread_count ;  thread++) {
messages [ thread ] = NULL;
pthread_cond_init(&cond [ thread ] , NULL) ;
## }
for  ( thread = 0;  thread < thread_count ;  thread++)
pthread_create(&thread_handles [ thread ] , NULL,
Send_msg ,  (void ∗)  thread ) ;
for  ( thread = 0;  thread < thread_count ;  thread++) {
pthread_join ( thread_handles [ thread ] , NULL) ;
## }
for  ( thread = 0;  thread < thread_count ;  thread++) {
f r e e ( messages [ thread ] ) ;
pthread_cond_destroy(&cond [ thread ] ) ;
## }
pthread_mutex_destroy(&mutex ) ;
f r e e ( messages ) ;
f r e e ( cond ) ;
f r e e ( thread_handles ) ;
## 8

## }
void ∗Send_msg(void∗ rank )  {
long my_rank = (long )  rank ;
long  dest = (my_rank + 1) % thread_count ;
char∗ my_msg = (char∗)  malloc (MSG_MAX∗ sizeof (char ) ) ;
s p r i n t f (my_msg,  " Hello ␣ to ␣%ld ␣from␣%ld " ,  dest ,  my_rank ) ;
messages [ dest ] = my_msg;
pthread_mutex_lock(&mutex ) ;
pthread_cond_signal(&cond [ dest ] ) ;
i f  ( messages [ my_rank ] == NULL)
pthread_cond_wait(&cond [ my_rank] ,&mutex ) ;
pthread_mutex_unlock(&mutex ) ;
p r i n t f ( "Thread␣%ld ␣>␣%s \n" , my_rank ,  messages [ my_rank ] ) ;
return NULL;
## }
## 9