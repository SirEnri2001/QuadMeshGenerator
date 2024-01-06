#include "test_operator.h"
#include "mesh/meshcomponents.h"
#include "mesh/meshdisplay.h"
#include "qmorph/frontedge.h"
#include "qmorph/frontedge_operator.h"
#include "qmorph/sidedefine_operator.h"
#include "qmorph/component_operator.h"
#include "qmorph/qmorph_display.h"
#include "qmorph/smoother.h"
#include <iostream>
#include <exception>

TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	auto lambdaSet = qmorphOperator->feOperator->edgeRecovery(
		mesh->getVertices().at(15).getMutable(),
		mesh->getVertices().at(25).getMutable());
	display->display->markVertex(mesh->getVertices().at(15).getMutable());
	display->display->markVertex(mesh->getVertices().at(25).getMutable());
}

void TestOperator::operator()() {
	proceed();
}

void TestOperator::setId(int i) {
	id = i;
}