#include "qmorph_operator.h"
#include "frontedge.h"
#include "frontedge_operator.h"
#include "sidedefine_operator.h"
#include "component_operator.h"
#include "../mesh/meshcomponents.h"
#include "smoother.h"

int QMorphOperator::doQMorphProcess() {
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
		if (sideOperator->doCornerGenerate()) {
			continue;
		}
		if (sideOperator->doSideDefine() == -1) { //fail to sideDefine because frontEdges are splited
			continue;
		}

		doEdgeRecovery();
		feOperator->proceedNextFeLoop();
		if (feOperator->getFrontEdgeGroup()) {
			feOperator->switchFrontEdgeGroup();
		}
	}
	return 0;
}

int QMorphOperator::doEdgeRecovery() {
	FrontEdge* iter = feOperator->getFrontEdgeGroup();
	int i = 0;
	do {
		if (iter->needTop) {
			FrontEdge* frontHe = iter;
			// left and right side & topology check
			compOperator->buildQuad(
				sideOperator->getLeftSide(frontHe)->getMutable(), 
				frontHe->he->getMutable(), 
				sideOperator->getRightSide(frontHe)->getMutable(),
				feOperator->edgeRecovery(
					sideOperator->getLeftSide(frontHe)->getSource()->getMutable(),
					sideOperator->getRightSide(frontHe)->getMutable()->getTarget()
				)->getMutable()
			);
			frontHe->setTop(frontHe->he->getNext()->getNext());
		}
	} while (iter = iter->getNextFe(), iter != feOperator->getFrontEdgeGroup());
	return 0;
}
int QMorphOperator::doSmooth(int epoch) {
	int i = 0;
	while (i < epoch) {
		for (auto& idV : mesh->getVertices()) {
			const Vertex* vertex = &idV.second;
			if (vertex->isBoundary()) {
				continue;
			}
			smoother->triangleInteriorSmooth(vertex, true);
		}
		i++;
	}
	return 0;
}

int QMorphOperator::doSeam() {
	//FrontEdge* feIter = feOperator->getFrontEdgeGroup();
	//const double seamEpsilon = 30.0;
	//int i = 0;
	//do {
	//	if (feIter->he->getSym()->isBoundary()
	//		|| compOperator->isQuad(feIter->he->getFace())
	//		|| compOperator->isQuad(feIter->getNextFe()->he->getFace())) {
	//		continue;
	//	}
	//	if (compOperator->angle(feIter->he, feIter->getNextFe()->he) < seamEpsilon) {
	//		// v1p--prevFe--v1---v2n---nextNextFe--v2nn
	//		//              | nextFe
	//		//           feIter /
	//		//              |  /
	//		//              | /
	//		//              v2

	//		HalfedgeHandle nextFe = mesh->getNextFe(feIter);
	//		HalfedgeHandle nextNextFe = mesh->getNextFe(mesh->getNextFe(feIter));
	//		HalfedgeHandle prevFe = mesh->getPrevFe(feIter);
	//		VertexHandle v1p = mesh->halfedgeSource(prevFe);
	//		VertexHandle v1 = mesh->halfedgeTarget(prevFe);
	//		VertexHandle v2 = mesh->halfedgeTarget(feIter);
	//		VertexHandle v2n = mesh->halfedgeTarget(nextFe);
	//		VertexHandle v2nn = mesh->halfedgeTarget(nextNextFe);
	//		mesh->setSide(prevFe, feIter, NULL);
	//		mesh->setSide(feIter, nextFe, NULL);
	//		mesh->setSide(nextFe, nextNextFe, NULL);
	//		mesh->setFront(feIter, false);
	//		mesh->setFront(nextFe, false);
	//		typedef HalfedgeHandle HalfedgeAttribute; // TODO: encapsulation
	//		// save attributes
	//		//HalfedgeAttribute
	//			//revFeAttribute = new CToolHalfedge(),
	//			//nextNextFeAttribute = new CToolHalfedge();

	//		//prevFe->attributeCopyTo(prevFeAttribute);
	//		//nextNextFe->attributeCopyTo(nextNextFeAttribute);
	//		mesh->edgeRecovery(v1, v2n);

	//		// TODO: might delete edge with special attributes
	//		mesh->clearFace({ feIter,nextFe, mesh->vertexHalfedge(v2n, v1) });
	//		if (i == 64 && globalIter == 27) {
	//			mesh->highlight({ feIter,mesh->getNextFe(feIter) });
	//			mesh->updateDebug();
	//		}
	//		v1 = mesh->mergeEdge(v1, v2n);
	//		// v1p--prevFe-> v1 --nextNextFe-> v2nn
	//		//               |
	//		//               |
	//		//               v2
	//		prevFe = mesh->vertexHalfedge(v1p, v1);
	//		nextNextFe = mesh->vertexHalfedge(v1, v2nn);
	//		//prevFeAttribute->attributeCopyTo(prevFe);
	//		//nextNextFeAttribute->attributeCopyTo(nextNextFe);
	//		//delete prevFeAttribute;
	//		//delete feIterAttribute;
	//		//delete nextFeAttribute;
	//		//delete nextNextFeAttribute;

	//		mesh->setNextFe(prevFe, nextNextFe);
	//		mesh->setNextFe(mesh->getPrevFe(prevFe), prevFe);
	//		mesh->setNextFe(nextNextFe, mesh->getNextFe(nextNextFe));
	//		feIter = prevFe;
	//		updateHeadFrontEdgeGroup(feIter);
	//		return -1;
	//	}
	//} while (feIter = mesh->getNextFe(feIter), feIter != getFrontEdgeGroup());
	return 0;
}

void QMorphOperator::create() {
	feOperator = std::make_shared<FrontEdgeOperator>(mesh, display);
	sideOperator = std::make_shared<SideDefineOperator>(mesh, display);
	compOperator = std::make_shared<ComponentOperator>(mesh, display);
	feOperator->create(compOperator, sideOperator);
	sideOperator->create(compOperator, feOperator);
	compOperator->create();
	smoother = new Smoother(mesh);
}

void QMorphOperator::test() {
}

void QMorphOperator::operator()() {
	create();
	doQMorphProcess();
	//test();
}

QMorphOperator::QMorphOperator(Mesh* mesh) : MeshUserOperator(mesh) {

}