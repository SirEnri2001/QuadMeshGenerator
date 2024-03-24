#include "test_operator.h"
#include "mesh/components.h"
#include "mesh/display.h"
#include "thread_support/thread_support.h"
#include "mesh/io.h"
#include <iostream>
#include <exception>
#include <filesystem>
#include "msc/msc.h"
#include "wave_quadrangulation/wave_quadrangulation.h"

namespace fs = std::filesystem;
using namespace quadro;
TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	MorseFunction mf(mesh);
	mf.setDisplay(display);
	mf();
}

void TestOperator::operator()() {
	proceed();
}

void TestOperator::setId(int i) {
	id = i;
}

void TestOperator::integrationTest(std::string path) {
	std::cout << "[ INTEGRATION TEST STARTING ... ]" << std::endl;
	for (const auto& entry : fs::directory_iterator(path)) {
		std::cout << "[Loading Test Case] " << entry.path() << std::endl;
		Mesh mesh;
		MeshIO meshio(&mesh);
		meshio.loadM(entry.path().string());
		std::cout << "[Test Completed] " << entry.path() << std::endl;
	}
	std::cout << "[ INTEGRATION TEST COMPLETED. ALL CASES PASSED ]" << std::endl;
}