#include <semaphore>
#include <thread>
#include <mutex>
#include <semaphore>

void begin_thread_control();
void end_thread_control();

void call_pause(); // non block
void call_resume(); // non block
bool check_pause(); //non block
void step_over_pause();//block
void restore_semaphore();
bool check_subthread();