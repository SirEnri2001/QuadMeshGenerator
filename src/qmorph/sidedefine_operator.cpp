#include "sidedefine_operator.h"
#include "../mesh/meshcomponents.h"
#include "frontedge.h"
#include "component_operator.h"
#include "frontedge_operator.h"
#include "util.h"
#include "../thread_support/thread_support.h"
#include "../mesh/meshdisplay.h"

SideDefineOperator::SideDefineOperator(Mesh* mesh, MeshDisplay* display) :MeshOperator(mesh, display) {
	for (auto& v : mesh->getVertices()) {
		sideCount[&v.second] = 0;
	}
	for (auto& he : mesh->getHalfedges()) {
		isHeSide[&he.second] = false;
	}
}
void SideDefineOperator::create(std::shared_ptr<ComponentOperator>& compOper, std::shared_ptr<FrontEdgeOperator>& feOper) {
	compOperator = compOper;
	feOperator = feOper;
}

bool SideDefineOperator::isSide(const Vertex* v) {
	return sideCount[v] > 0;
}
bool SideDefineOperator::isSide(const Halfedge* he) {
	return isHeSide[he];
}
void SideDefineOperator::setSide(const Halfedge* he, bool val) {
	if (val && !isHeSide[he]) {
		isHeSide[he] = true;
		sideCount[he->getSource()]++;
		sideCount[he->getTarget()]++;
	}
	else if (!val && isHeSide[he]) {
		isHeSide[he] = false;
		sideCount[he->getSource()]--;
		sideCount[he->getTarget()]--;
	}
}

const Halfedge* SideDefineOperator::getLeftSide(const FrontEdge* fe) {
	return leftSideEdges[fe];
}

const Halfedge* SideDefineOperator::getRightSide(const FrontEdge* fe) {
	return rightSideEdges[fe];
}

void SideDefineOperator::setSide(const FrontEdge* lfe, const Halfedge* rightSideForLfe) {
	if (rightSideForLfe) {
		setSide(rightSideForLfe, true);
		setSide(rightSideForLfe->getSym(), true);
		leftSideEdges[lfe->getNextFe()] = rightSideForLfe->getSym();
		rightSideEdges[lfe] = rightSideForLfe;
		return;
	}
	setSide(getRightSide(lfe), false);
	setSide(getLeftSide(lfe->getNextFe()), false);
	leftSideEdges[lfe->getNextFe()] = nullptr;
	rightSideEdges[lfe] = nullptr;
}
// these functions only return its finding of side he & its topology state
SideDefineResult SideDefineOperator::verticalSideSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide) {
	const Halfedge* minAngleHe = lfe->he;
	const Vertex* pivotVertex = lfe->he->getTarget();
	glm::vec4 pivot = pivotVertex->getPosition();
	glm::vec4 bisector = compOperator->bisect(lfe->he, rfe->he);
	//int index;
	//bisector = mesh->nearestCrossField(pivotVertex, bisector, index);
	double minAngle = 360.0;
	bool traversed = false;
	const Halfedge* pivotIter = lfe->he;
	do {
		if (pivotIter->getSym()->isBoundary()) {
			continue;
		}
		traversed = true;
		double angle = compOperator->angle(bisector + pivot, pivotIter->getSym());
		if (angle < minAngle) {
			minAngle = angle;
			minAngleHe = pivotIter;
		}
	} while (pivotIter = pivotIter->getNext()->getSym(), pivotIter != rfe->he->getSym());
	if (!traversed) {
		return SideDefineResult::NoSuitable;
	}
	resultUpSide = minAngleHe->getSym();
	if (lfe->getPrevFe()->he->getSource() == minAngleHe->getSource()
		|| rfe->getNextFe()->he->getTarget() == minAngleHe->getSource()) {
		// size 3 front edge group
		return SideDefineResult::FrontEdgeContactDegenerate;
	}
	if (feOperator->isFront(minAngleHe->getSource())) {
		return SideDefineResult::FrontEdgeContact;
	}
	if (compOperator->numQuad(minAngleHe->getSource()) > 0) {
		return SideDefineResult::QuadContactByVertex;
	}
	if (isSide(minAngleHe->getSource())) {
		return SideDefineResult::SideEdgeContact;
	}
	if (minAngle < constEpsilon) {
		//mesh->alignToCrossField(mesh->halfedgeEdge(minAngleHe), pivotVertex);
		return SideDefineResult::Succeeded;
	}
	else {
		return SideDefineResult::NoSuitable;
	}
}
SideDefineResult SideDefineOperator::horizontalSideSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide) {
	const Halfedge* minAngleHe = lfe->he;
	const Vertex* pivotVertex = lfe->he->getTarget();
	glm::vec4 pivot = pivotVertex->getPosition();
	glm::vec4 bisector = compOperator->bisect(lfe->he, rfe->he);
	const Halfedge* pivotIter = lfe->he;
	bool traversed = false;
	double minAngle = 360.0;
	// traverse all quadrilateral surround the vertex
	//(mAV)<--.
	//   |   / \
		//   |  /   \
		//   | /     \
		//   v--minAH>(pV)
	const Vertex* minAngleVertex = lfe->he->getSource();
	do
	{
		if (feOperator->isFront(pivotIter->getPrev())
			|| compOperator->isQuad(pivotIter->getPrev()->getSym()->getFace())) {
			continue;
		}
		traversed = true;
		const Vertex* curVertex = pivotIter->getPrev()->getSym()->getNext()->getTarget();
		double angle = compOperator->angle(bisector + pivot, pivotVertex, curVertex->getPosition());
		if (angle < minAngle) {
			minAngle = angle;
			minAngleHe = pivotIter;
			minAngleVertex = curVertex;
		}
	} while (pivotIter = pivotIter->getNext()->getSym(), pivotIter != rfe->he->getSym());
	if (!traversed) {
		return SideDefineResult::NoSuitable;
	}
	resultUpSide = minAngleHe;
	if (minAngleVertex == lfe->getPrevFe()->he->getSource() ||
		minAngleVertex == rfe->getNextFe()->he->getTarget()) {
		return SideDefineResult::FrontEdgeContactDegenerate;
	}
	if (feOperator->isFront(minAngleVertex)) {
		return SideDefineResult::FrontEdgeContact;
	}
	if (compOperator->numQuad(minAngleVertex)) {
		return SideDefineResult::QuadContactByVertex;
	}
	if (isSide(minAngleVertex)) {
		return SideDefineResult::SideEdgeContact;
	}
	if (minAngle < constEpsilon
		&& 2.0 * glm::length(minAngleVertex->getPosition() - lfe->he->getTarget()->getPosition()) <
		sqrt(3) * (lfe->he->getLength() + rfe->he->getLength())) {  //safisty second condition
		return SideDefineResult::Succeeded;
	}
	else {
		return SideDefineResult::NoSuitable;
	}
}
SideDefineResult SideDefineOperator::verticalSideSplitSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide) {
	const Halfedge* minAngleHe = lfe->he;
	const Vertex* pivotVertex = lfe->he->getTarget();
	glm::vec4 pivot = pivotVertex->getPosition();
	glm::vec4 bisector = compOperator->bisect(lfe->he, rfe->he);
	const Halfedge* pivotIter = lfe->he;
	double minAngle = 360.0;

	do {
		if (pivotIter->getSym()->isBoundary()) {
			continue;
		}
		double angle = compOperator->angle(bisector + pivot, pivotIter->getSym());
		if (angle < minAngle) {
			minAngle = angle;
			minAngleHe = pivotIter;
		}
	} while (pivotIter = pivotIter->getNext()->getSym(), pivotIter != rfe->he->getSym());
	resultUpSide = minAngleHe->getSym();
	if (lfe->getPrevFe()->he->getSource() == minAngleHe->getSource()
		|| rfe->getNextFe()->he->getTarget() == minAngleHe->getSource()) {
		// size 3 front edge group
		return SideDefineResult::FrontEdgeContactDegenerate;
	}
	if (feOperator->isFront(minAngleHe->getSource())) {
		return SideDefineResult::FrontEdgeContact;
	}
	return SideDefineResult::Succeeded;
}
SideDefineResult SideDefineOperator::horizontalSideSplitSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide) {
	using namespace glm;
	const Halfedge* minAngleHe = lfe->he;
	const Vertex* pivotVertex = lfe->he->getTarget();
	vec4 pivot = pivotVertex->getPosition();
	vec4 bisector = compOperator->bisect(lfe->he, rfe->he);
	vec4 leftLocal = lfe->he->getSource()->getPosition() - pivot;
	vec4 rightLocal = rfe->he->getTarget()->getPosition() - pivot;
	vec4 cs = vec4(cross(vec3(leftLocal), vec3(bisector)), 0);
	const Halfedge* pivotIter = lfe->he;
	double minAngle = 360.0;
	const Halfedge* he2 = minAngleHe;
	do {
		if (dot(cross(vec3(bisector), vec3(pivotIter->getSource()->getPosition() - pivot)),
			cross(vec3(bisector), vec3(pivotIter->getNext()->getTarget()->getPosition() - pivot))) < 0) {
			he2 = pivotIter;
		}
	} while (pivotIter = pivotIter->getNext()->getSym(), pivotIter != rfe->he->getSym());
	//(mAV)<----.
	//   |    /   \
		//   |   /     he1
		//   | /          \
		//   v--minAH(he2)-->(pV)
	resultUpSide = he2;
	const Halfedge* he1 = he2->getNext();
	// 2 halfedges in a patch: --he2-->.--he1-->
	if (he2->getSource() != lfe->he->getSource() && feOperator->isFront(he2->getPrev())) {
		return SideDefineResult::FrontEdgeContactDegenerate;
	}
	if (compOperator->isQuad(he2->getPrev()->getSym()->getFace())) {
		return SideDefineResult::QuadContactByEdge;
	}
	if (isSide(he2->getSource()) || isSide(he1->getTarget())) {
		return SideDefineResult::SideEdgeContact;
	}
	return SideDefineResult::Succeeded;
	//Note: if lfe & rfe are on one corner, lfe->target == rfe->source, and no degeneracy detected, 
	// then this function must return Succeeded
}

int SideDefineOperator::frontEdgeSideDefine(FrontEdge* lfe, FrontEdge* rfe) {
	if (compOperator->isQuad(lfe->he->getFace())) {
		setSide(lfe, lfe->he->getNext());
		return 0;
	}
	if (compOperator->isQuad(rfe->he->getFace())) {
		setSide(lfe, rfe->he->getPrev()->getSym());
		return 0;
	}
	const Halfedge* resultUpSide;
	//if (lfe->he->getId() == 128) {//128
	//	display->create();
	//	display->createFrame();
	//	display->markHalfedge(lfe->he);
	//	display->markHalfedge(lfe->getNextFe()->he);
	//	step_over_pause();
	//}
	switch (verticalSideSeek(lfe, rfe, resultUpSide))
	{
	case SideDefineResult::FrontEdgeContact:
		feOperator->seperateFrontLoop(resultUpSide);
		return 1;
	case SideDefineResult::Succeeded:
		setSide(lfe, resultUpSide);
		return 0;
	default:
		break;
	}

	switch (horizontalSideSeek(lfe, rfe, resultUpSide)) {
	case SideDefineResult::FrontEdgeContact:
		compOperator->swapEdge(resultUpSide->getPrev()->getMutable());
		feOperator->seperateFrontLoop(resultUpSide->getNext());
		return 1;
	case SideDefineResult::Succeeded:
		compOperator->swapEdge(resultUpSide->getPrev()->getMutable());
		setSide(lfe, resultUpSide->getNext());
		return 0;
	default:
		break;
	}
	if (horizontalSideSplitSeek(lfe, rfe, resultUpSide) == SideDefineResult::Succeeded) {
		const Halfedge* he2 = resultUpSide;
		const Halfedge* he1 = he2->getNext();
		const Vertex* pivotVertex = lfe->he->getTarget();
		// solve the 2x2 linear system
		glm::vec4 v1 = he2->getSource()->getPosition() - he2->getTarget()->getPosition();
		glm::vec4 v2 = he1->getTarget()->getPosition() - he2->getTarget()->getPosition();
		glm::vec4 bisector = compOperator->bisect(lfe->he, rfe->he);
		float* km = solveKMEquation(glm::vec3(v1), glm::vec3(v2), glm::vec3(bisector));
		glm::vec4 p = bisector * (km[0] + FLT_EPSILON * 10.0F) + pivotVertex->getPosition();
		const Vertex* spliter = compOperator->splitEdge(he2->getPrev()->getMutable(), p);
		const Halfedge* newHe = mesh->getHalfedge(pivotVertex, spliter);
		assert(newHe);
		setSide(lfe, newHe);
		return 0;
	}

	switch (verticalSideSplitSeek(lfe, rfe, resultUpSide))
	{
	case SideDefineResult::FrontEdgeContact:
	case SideDefineResult::FrontEdgeContactDegenerate:
		feOperator->seperateFrontLoop(resultUpSide);
		return 1;
	case SideDefineResult::Succeeded:
		setSide(lfe, NULL);
		const Vertex* v = compOperator->splitEdge(resultUpSide->getMutable(),
			(resultUpSide->getSource()->getPosition() + resultUpSide->getTarget()->getPosition()) / 2.f);
		resultUpSide = mesh->getHalfedge(lfe->he->getTarget(), v);
		setSide(lfe, resultUpSide);
		return 0;
	}
	assert(false);
}
int SideDefineOperator::generateCorner(FrontEdge* lfe, FrontEdge* rfe) {
	setSide(lfe->getPrevFe(), NULL);
	setSide(lfe, NULL);
	setSide(rfe, NULL);
	if (compOperator->isQuad(rfe->getNextFe()->he->getFace())) {
		compOperator->buildQuad(
			lfe->he->getMutable(),
			rfe->he->getMutable(),
			rfe->getNextFe()->he->getPrev()->getSym()->getMutable(),
			feOperator->edgeRecovery(
				rfe->getNextFe()->he->getPrev()->getSym()->getTarget()->getMutable(),
				lfe->he->getSource()->getMutable()
			)->getMutable()
		);
		rfe->top = lfe->he->getPrev();
		setSide(lfe->getPrevFe(), rfe->top->getSym());
	}
	else if (compOperator->isQuad(lfe->getPrevFe()->he->getFace())) {
		compOperator->buildQuad(
			lfe->getPrevFe()->he->getNext()->getSym()->getMutable(), 
			lfe->he->getMutable(),
			rfe->he->getMutable(),
			feOperator->edgeRecovery(
				rfe->he->getTarget()->getMutable(),
				lfe->getPrevFe()->he->getNext()->getSym()->getSource()->getMutable()
			)->getMutable()
		);
		
		lfe->top = rfe->he->getNext();
		setSide(rfe, lfe->top);
	}
	else {
		if (frontEdgeSideDefine(lfe->getPrevFe(), lfe)) { //fail to define side edge
			return 1; //reclassify corner must be called again
		}
		compOperator->buildQuad (
			getLeftSide(lfe)->getMutable(),
			lfe->he->getMutable(),
			rfe->he->getMutable(),
			feOperator->edgeRecovery(
				rfe->he->getTarget()->getMutable(),
				getLeftSide(lfe)->getSource()->getMutable()
			)->getMutable()
		);
		setSide(lfe->getPrevFe(), lfe->he->getPrev()->getSym());
		setSide(rfe, rfe->he->getNext());
	}
	lfe->needTop = false;
	rfe->needTop = false;
	return 0;
}
int SideDefineOperator::doSideDefine() {
	FrontEdge* lfe, * rfe;
	int i = 0;
	// traverse all front edge
	lfe = feOperator->getFrontEdgeGroup();
	do {
		rfe = lfe->getNextFe();
		if (!lfe->needTop && !rfe->needTop) {
			continue;
		}
		int retVal = frontEdgeSideDefine(lfe, rfe);
		if (retVal != 0) {
			return -1;
		}

	} while (lfe = lfe->getNextFe(), lfe != feOperator->getFrontEdgeGroup());
	return 0;
}
int SideDefineOperator::doCornerGenerate() {
	int i = 0;
	bool init = true;
	FrontEdge* iter = feOperator->getFrontEdgeGroup();
	do {
		FrontEdge* lfe = iter;
		FrontEdge* rfe = lfe->getNextFe();
		if (init && lfe->isLeftCornerSharp()) {
			init = false;
			continue;
		}
		if (!lfe->needTop) {
			continue;
		}
		if (compOperator->isQuad(rfe->he->getFace())) {
			continue;
		}
		if (lfe->isRightCornerSharp()) {
			if (rfe->isRightCornerSharp()) { // form a ``|_|`` shape
				FrontEdge
					* lfePrev = lfe->getPrevFe(),
					* rfeNext = rfe->getNextFe(),
					* nnRfe = rfeNext->getNextFe();
				if (compOperator->isQuad(lfePrev->he->getFace()) || compOperator->isQuad(rfeNext->he->getFace())) {
					generateCorner(lfe, rfe);
					return 1;
				}
				lfe->needTop = false;
				rfe->needTop = false;
				rfeNext->needTop = false;
				setSide(lfePrev, NULL);
				setSide(rfeNext, NULL);
				compOperator->buildQuad(
					lfe->he->getMutable(),
					rfe->he->getMutable(),
					rfeNext->he->getMutable(),
					feOperator->edgeRecovery(
						rfeNext->he->getTarget()->getMutable(),
						lfe->he->getSource()->getMutable()
					)->getMutable()
				);
				const Halfedge* newFeHe = rfeNext->he->getNext()->getSym();
				FrontEdge* newFe = feOperator->setFront(newFeHe, true);
				feOperator->setNextFe(newFe, nnRfe);
				feOperator->setNextFe(lfePrev, newFe);
				feOperator->setFront(rfeNext->he, false);
				feOperator->setFront(rfe->he, false);
				feOperator->setFront(lfe->he, false);
				feOperator->updateHeadFrontEdgeGroup(newFe);

				return 1;
			}
			else { // form a ``|_.. shape
				generateCorner(lfe, rfe);
				return 1;
			}
		}

	} while (iter = iter->getNextFe(), iter != feOperator->getFrontEdgeGroup());
	return 0;
}