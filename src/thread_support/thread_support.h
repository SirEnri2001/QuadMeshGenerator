#include <mutex>

extern std::mutex pause_mutex;
extern std::mutex pause_call_mutex;

void call_pause(); // non block
void call_resume(); // non block
void lock_on_pause(); // block
bool check_pause(); //non block
void step_over_pause();//block