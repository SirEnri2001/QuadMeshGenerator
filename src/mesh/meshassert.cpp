#include "meshassert.h"
#include "meshcomponents.h"
#include <boost/stacktrace.hpp>
#include <sstream>

void MeshAssert::error(std::string& msg) {
	std::stringstream ss;
	ss << boost::stacktrace::stacktrace();
	traceback = ss.str();
	std::terminate();
}