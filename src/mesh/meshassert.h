#pragma once
#include "mesh.h"
#include <string>
#include <exception>
#include "../thread_support/thread_support.h"


#define error(x) __debugbreak();throw x;
#define validate(x) if(!x) {step_over_pause();error("Validate failed");}

class MeshAssert {
	long unsigned int pauseMark = 0l;
public:
	void callResume();
	void onPause(int mark = -1);
	void setPauseMark(int mark, bool val);
	bool isPauseMarked(int mark);
};