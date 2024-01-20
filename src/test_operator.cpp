#include "test_operator.h"
#include "mesh/meshcomponents.h"
#include "mesh/meshdisplay.h"
#include "qmorph/frontedge.h"
#include "qmorph/frontedge_operator.h"
#include "qmorph/sidedefine_operator.h"
#include "qmorph/component_operator.h"
#include "qmorph/qmorph_display.h"
#include "qmorph/smoother.h"
#include "thread_support/thread_support.h"
#include "mesh/meshio.h"
#include <iostream>
#include <exception>
#include <filesystem>
namespace fs = std::filesystem;

TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	integrationTest(integrationTestPath);
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
		QMorphOperator qmorphOper(&mesh);
		qmorphOper.setAsserts(asserts);
		display->display->setMesh(&mesh);
		display->qmorphOperator = &qmorphOper;
		qmorphOper.setDisplay(display->display);
		qmorphOper();
		std::cout << "[Test Completed] " << entry.path() << std::endl;
	}
	std::cout << "[ INTEGRATION TEST COMPLETED. ALL CASES PASSED ]" << std::endl;
}