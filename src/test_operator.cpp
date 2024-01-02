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
	for (auto& idHe : mesh->getHalfedges()) {
		if(idHe.second.isBoundary())
			display->markHalfedge(&idHe.second);
	}
	//const Halfedge* he;
	//for (auto& idHe : mesh->getHalfedges()) {
	//	display->markHalfedge(&idHe.second);
	//	if (idHe.second.isBoundary()) {
	//		he = &idHe.second;
	//		break;
	//	}
	//}
	//while (true) {
	//	display->markHalfedge(he);
	//	he = he->getNext();
	//	std::cout << "Processing ... " << std::endl;
	//}
}

void TestOperator::operator()() {
	proceed();
}