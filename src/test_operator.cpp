#include "test_operator.h"
#include "mesh/meshcomponents.h"
#include "mesh/meshdisplay.h"
#include <iostream>

TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	while (true) {
		for (auto& idV : mesh->getVertices()) {
			display->markHalfedge(idV.second.getHalfedge());
		}
	}
}

void TestOperator::operator()() {
	proceed();
}