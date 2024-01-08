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
	qmorphOperator->feOperator->edgeRecovery(mesh->vertexAt(1)->getMutable(), mesh->vertexAt(40)->getMutable());
}

void TestOperator::operator()() {
	proceed();
}

void TestOperator::setId(int i) {
	id = i;
}