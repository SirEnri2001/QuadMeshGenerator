#include "thread_support.h"
#include <mutex>
#include <shared_mutex>

std::binary_semaphore main_acquire_sub_release{ 1 }, sub_acquire_main_release{ 0 };

// { 1; 0 } means subthread will or already pause on breakpoint
// { 0; 1 } means a transient state that mainthread just called resume and just before subthread catch the signal

int sub_thread_count = 0;


void call_resume() {
	main_acquire_sub_release.acquire();
	sub_acquire_main_release.release();
}

bool check_pause() {
	main_acquire_sub_release.acquire();
	main_acquire_sub_release.release();
	if (!sub_acquire_main_release.try_acquire()) {
		return true;
	}
	else {
		sub_acquire_main_release.release();
		return false;
	}
}

void step_over_pause() {
	if (sub_thread_count > 0) {
		sub_acquire_main_release.acquire();
		main_acquire_sub_release.release();
	}
}



void begin_thread_control() {
	sub_thread_count++;
}

void end_thread_control() {
	sub_thread_count--;
}


void call_pause() {

}