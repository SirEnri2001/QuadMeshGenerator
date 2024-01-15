#include "meshassert.h"
#include "meshcomponents.h"
#include "../thread_support/thread_support.h"
#include <sstream>

void MeshAssert::callResume() {
	call_resume();
}

void MeshAssert::onPause(int mark) {
	if (mark == -1 || 1 << mark & pauseMark) {
		step_over_pause();
	}
}

void MeshAssert::setPauseMark(int mark, bool val) {
	if (val) {
		pauseMark |= 1 << mark;
	}
	else {
		pauseMark = ~(~pauseMark | (1 << mark));
	}
}

bool MeshAssert::isPauseMarked(int mark) {
	return 1 << mark & pauseMark;
}