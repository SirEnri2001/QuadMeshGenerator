#include "test_operator.h"
#include "mesh/meshcomponents.h"
#include "mesh/meshdisplay.h"
#include <iostream>
#include <exception>

TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	for (auto& idV : mesh->getVertices()) {
		display->markHalfedge(idV.second.getHalfedge());
	}
	__debugbreak();
	throw "Test exception";
}

void TestOperator::operator()() {
	proceed();
}