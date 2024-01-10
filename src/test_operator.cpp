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
#include <iostream>
#include <exception>

TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	glm::vec4 res = qmorphOperator->compOperator->bisect(
		mesh->halfedgeAt(341), mesh->halfedgeAt(356)
	);
	std::cout << res[0] << ", " << res[1] << ", " << res[2] << ", " << res[3] << std::endl;
	display->display->markPoint(res + mesh->halfedgeAt(341)->getTarget()->getPosition());
}

void TestOperator::operator()() {
	proceed();
}

void TestOperator::setId(int i) {
	id = i;
}