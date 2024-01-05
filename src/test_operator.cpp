#include "test_operator.h"
#include "mesh/meshcomponents.h"
#include "mesh/meshdisplay.h"
#include "qmorph/frontedge.h"
#include "qmorph/frontedge_operator.h"
#include "qmorph/sidedefine_operator.h"
#include "qmorph/component_operator.h"
#include "qmorph/smoother.h"
#include <iostream>
#include <exception>

TestOperator::TestOperator(Mesh* mesh) : QMorphOperator(mesh) {
	display = nullptr;
}

void TestOperator::create() {

}

void TestOperator::proceed() {
	feOperator->initFrontEdgeGroup();
	while (doSmooth(), feOperator->getFrontEdgeGroup())
	{
		if (feOperator->frontEdgeGroupSize(feOperator->getFrontEdgeGroup()) == 4) {
			FrontEdge* fe = feOperator->getFrontEdgeGroup();
			sideOperator->setSide(fe, NULL);
			sideOperator->setSide(fe->getNextFe(), NULL);
			sideOperator->setSide(fe->getNextFe()->getNextFe(), NULL);
			sideOperator->setSide(fe->getPrevFe(), NULL);
			compOperator->buildQuad(
				fe->getPrevFe()->getPrevFe()->he->getMutable(),
				fe->getPrevFe()->he->getMutable(),
				fe->he->getMutable(),
				fe->getNextFe()->he->getMutable());
			feOperator->setFront(fe->getPrevFe()->getPrevFe()->he, false);
			feOperator->setFront(fe->getPrevFe()->he, false);
			feOperator->setFront(fe->getNextFe()->he, false);
			feOperator->setFront(fe->he, false);
			feOperator->popFrontEdgeGroup();
			continue;
		}

		if (doSeam() != 0)
		{
			continue;
		}
		feOperator->updateFeClassification();
		//if (sideOperator->doCornerGenerate()) {
		//	continue;
		//}
		//if (sideOperator->doSideDefine() == -1) { //fail to sideDefine because frontEdges are splited
		//	continue;
		//}

		//doEdgeRecovery();
		//feOperator->proceedNextFeLoop();
		//if (feOperator->getFrontEdgeGroup()) {
		//	feOperator->switchFrontEdgeGroup();
		//}
	}
	return;
}

void TestOperator::operator()() {
	proceed();
}

void TestOperator::setId(int i) {
	id = i;
}