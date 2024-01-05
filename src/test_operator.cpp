#include "test_operator.h"
#include "mesh/meshcomponents.h"
#include "mesh/meshdisplay.h"
#include <iostream>
#include <exception>

TestOperator::TestOperator(Mesh* mesh) : MeshUserOperator(mesh) {
	display = nullptr;
	compOperator = new ComponentOperator(mesh);
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	const Halfedge* he = &mesh->getHalfedges().at(0);
	compOperator->splitEdge(he->getMutable(), glm::vec3((he->getSource()->getPosition() + he->getTarget()->getPosition()) * 0.5f));
	
	
	//if (!he->isBoundary()) {
	//	mesh->deleteFace(he->getFace()->getMutable());
	//}
	//if (!he->getSym()->isBoundary()) {
	//	mesh->deleteFace(he->getSym()->getFace()->getMutable());
	//}
	//mesh->deleteEdge(he->getMutable());
}

void TestOperator::operator()() {
	proceed();
}

void TestOperator::setId(int i) {
	id = i;
}