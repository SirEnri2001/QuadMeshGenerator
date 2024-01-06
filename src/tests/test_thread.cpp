#include "test_thread.h"
#include <thread>
#include <iostream>
int main() {
	std::cout << "Test thread start" << std::endl;
	pause_mutex.lock();
	std::cout << "locked" << std::endl;
	pause_mutex.unlock();
	std::cout << "released" << std::endl;
	return 0;
}