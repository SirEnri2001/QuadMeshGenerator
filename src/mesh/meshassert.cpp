#include "meshassert.h"
#include "meshcomponents.h"
#include <boost/stacktrace.hpp>
#include <sstream>

void MeshAssert::_error(std::string& msg) {
	std::stringstream ss;
	ss << boost::stacktrace::stacktrace();
	traceback = ss.str();
	__debugbreak();
	std::terminate();
}