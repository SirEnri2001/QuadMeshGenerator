#include "thread_support.h"
#include <mutex>

std::mutex pause_mutex;
std::mutex pause_call_mutex;

void call_pause() {
	pause_mutex.try_lock();
}

void call_resume() {
	pause_mutex.unlock();
}

void lock_on_pause() {
	pause_mutex.lock();
	pause_mutex.unlock();
}

void step_over_pause() {
	pause_mutex.lock();
}

bool check_pause() {
	if (pause_mutex.try_lock()) {
		pause_mutex.unlock();
		return false; // not at pause state
	}
	return true;
}