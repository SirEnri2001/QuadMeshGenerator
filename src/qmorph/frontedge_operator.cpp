#include "frontedge_operator.h"
#include "frontedge.h"
#include "../mesh/meshcomponents.h"
#include "component_operator.h"
#include "sidedefine_operator.h"

FrontEdge* FrontEdgeOperator::getNextFe(FrontEdge* fe) {
	return fe->nextFe;
}
const FrontEdge* FrontEdgeOperator::getNextFe(const FrontEdge* fe) {
	return fe->nextFe;
}
FrontEdge* FrontEdgeOperator::getPrevFe(FrontEdge* fe) {
	return fe->prevFe;
}
const FrontEdge* FrontEdgeOperator::getPrevFe(const FrontEdge* fe) {
	return fe->prevFe;
}
void FrontEdgeOperator::create() {}

FrontEdgeOperator::FrontEdgeOperator(Mesh* mesh) :MeshOperator(mesh) {
	for (auto& idV : mesh->getVertices()) {
		frontEdgeCount[&idV.second] = 0;
	}
}

void FrontEdgeOperator::create(std::shared_ptr<ComponentOperator>& compOper, std::shared_ptr<SideDefineOperator>& sideOper) {
	compOperator = compOper;
	sideOperator = sideOper;
}

FrontEdge* FrontEdgeOperator::getFront(const Halfedge* he) {
	return &heToFe[he];
}

void FrontEdgeOperator::updateFeClassification() {
	const double constAngle = 135.0;
	for (auto& fe1 : frontEdgeGroups) {
		FrontEdge* fe = fe1;
		do {
			if (compOperator->angle(fe->he, fe->getNextFe()->he) < constAngle) {
				fe->rightCornerSharp = true;
			}
			else {
				fe->rightCornerSharp = false;
			}
		} while (fe = fe->getNextFe(), fe != fe1);
	}
}

FrontEdge* FrontEdgeOperator::setFront(const Halfedge* he, bool val) {
	auto iter = heToFe.find(he);
	if (!val && iter != heToFe.end()) {
		frontEdgeCount[he->getSource()]--;
		frontEdgeCount[he->getTarget()]--;
		heToFe.erase(iter);
		return nullptr;
	}
	else if (val && iter == heToFe.end()) {
		FrontEdge& fe = heToFe[he] = FrontEdge();

		frontEdgeCount[he->getSource()]++;
		frontEdgeCount[he->getTarget()]++;
		fe.he = he;
		return &fe;
	}
	else if (val) {
		return &iter->second;
	}
	return nullptr;
}

void FrontEdgeOperator::setNextFe(FrontEdge* fe, FrontEdge* feNext) {
	fe->nextFe = feNext;
	feNext->prevFe = fe;
}

int FrontEdgeOperator::countFeToFe(FrontEdge* fe1, FrontEdge* fe2) {
	int count = 1;
	FrontEdge* feIter = fe1;
	while (feIter != fe2) {
		count++;
		feIter = getNextFe(feIter);
	}
	return count;
}
int FrontEdgeOperator::initFrontEdgeGroup() {
	int i = 0;

	for (auto& idHe : mesh->getHalfedges()) {
		const Halfedge* he = &idHe.second;
		if (!he->isBoundary() || isFront(he->getSym())) {
			continue;
		}
		const Halfedge* curHe = he;
		FrontEdge* fe;
		FrontEdge* prevFe;
		do {
			FrontEdge* curFe = setFront(curHe->getSym(), true);
			FrontEdge* nextFe = setFront(curHe->getPrev()->getSym(), true);
			setNextFe(curFe, nextFe);
			curHe = curHe->getPrev();
			i++;
		} while (curHe != he);
		pushTailFrontEdgeGroup(fe);
	}
	if (i % 2 == 0) {
		return 0;
	}
	FrontEdge* longestFE = getFrontEdgeGroup();
	FrontEdge* fe = longestFE;
	do {
		if (glm::length(fe->he->getSource()->getPosition() - fe->he->getTarget()->getPosition()) >
			glm::length(longestFE->he->getSource()->getPosition() - longestFE->he->getTarget()->getPosition()))
		{
			longestFE = fe;
		}
	} while ((fe = getNextFe(fe)) != longestFE);
	FrontEdge* prevFe, * nextFe;
	const Vertex* v1, * v2, * vNew;
	prevFe = getPrevFe(longestFE);
	nextFe = getNextFe(longestFE);
	v1 = longestFE->he->getSource();
	v2 = longestFE->he->getTarget();
	const Halfedge* longestHE = longestFE->he;
	setFront(longestHE, false);
	vNew = compOperator->splitEdge(longestHE,
		0.5f * (longestHE->getTarget()->getPosition() + longestHE->getSource()->getPosition()));

	FrontEdge* fe1 = setFront(mesh->getHalfedge(v1, vNew), true);
	FrontEdge* fe2 = setFront(mesh->getHalfedge(vNew, v2), true);

	setNextFe(prevFe, fe1);
	setNextFe(fe1, fe2);
	setNextFe(fe2, nextFe);
	updateHeadFrontEdgeGroup(prevFe);
	return 0;
}


bool FrontEdgeOperator::isFront(const Halfedge* he) {
	return heToFe.find(he) != heToFe.cend();
}
bool FrontEdgeOperator::isFront(const Vertex* v) {
	return frontEdgeCount[v] > 0;
}
int FrontEdgeOperator::frontEdgeGroupSize(FrontEdge* fe) {
	int count = 0;
	FrontEdge* feIter = fe;
	do {
		count++;
	} while (
		feIter = getNextFe(feIter), feIter != fe);
	return count;
}

void FrontEdgeOperator::pushHeadFrontEdgeGroup(FrontEdge* fe) {
	frontEdgeGroups.push_front(fe);
}

void FrontEdgeOperator::pushTailFrontEdgeGroup(FrontEdge* fe) {
	frontEdgeGroups.push_back(fe);
}

// param he: any frontedge in the group
void FrontEdgeOperator::removeFrontEdgeGroup(FrontEdge* fe) {
	FrontEdge* feIter = fe;
	do {
		frontEdgeGroups.remove(feIter);
	} while (feIter = getNextFe(feIter), feIter != fe);
}

bool FrontEdgeOperator::isFrontEdgeGroupIndex(FrontEdge* he) {
	return find(frontEdgeGroups.begin(), frontEdgeGroups.end(), he) != frontEdgeGroups.end();
}

FrontEdge* FrontEdgeOperator::popFrontEdgeGroup() {
	FrontEdge* he = frontEdgeGroups.front();
	frontEdgeGroups.pop_front();
	return he;
}
FrontEdge* FrontEdgeOperator::getFrontEdgeGroup() {
	if (frontEdgeGroups.size() > 0) {
		return frontEdgeGroups.front();
	}
	else {
		return NULL;
	}
}
bool FrontEdgeOperator::switchFrontEdgeGroup() {
	pushTailFrontEdgeGroup(getFrontEdgeGroup());
	popFrontEdgeGroup();
	return getFrontEdgeGroup() != NULL;
}
void FrontEdgeOperator::updateHeadFrontEdgeGroup(FrontEdge* he) {
	popFrontEdgeGroup();
	pushHeadFrontEdgeGroup(he);
}
int FrontEdgeOperator::seperateFrontLoop(const Halfedge* cutPos) {
	const Vertex* vertex1 = cutPos->getTarget();
	const Vertex* vertex2 = cutPos->getSource();
	FrontEdge* fe1, * fe2, * fe3, * fe4;
	const Halfedge* he1, * he2, * he3, * he4;
	he3 = cutPos->getNext();
	while (!isFront(he3)) {
		he3 = he3->getSym()->getNext();
	}
	fe3 = getFront(he3);
	fe2 = getPrevFe(fe3);

	he1 = cutPos->getSym()->getNext();
	while (!isFront(he1)) {
		he1 = he1->getSym()->getNext();
	}
	fe1 = getFront(he1);
	fe4 = getPrevFe(fe1);
	// <--fe1-(v2)<-fe4---
	//         ^|
	//         |(cutPos)
	//         |v
	//---fe2->(v1)--fe3-->
	if (fe3->he->getTarget() == fe4->he->getSource() || fe1->he->getTarget() == fe2->he->getSource()) {
		compOperator->swapEdge(cutPos);
		return 0;
	}

	FrontEdge* fe_iter = fe3;
	int count_fe1 = 2, count_fe2 = 2;
	bool splitFe = false;
	while (fe_iter != fe2) {
		fe_iter = getNextFe(fe_iter);
		count_fe1++;
		if (fe_iter == fe4) {
			splitFe = true;
			break;
		}
	}
	if (count_fe1 < 4) {
		return -1;
	}
	fe_iter = fe1;
	while (fe_iter != fe4) {
		fe_iter = getNextFe(fe_iter);
		count_fe2++;
		if (fe_iter == fe2) {
			assert(splitFe);
			break;
		}
	}
	if (count_fe2 < 4) {
		return -1;
	}
	removeFrontEdgeGroup(fe1);
	removeFrontEdgeGroup(fe2);
	sideOperator->setSide(fe4, NULL);
	sideOperator->setSide(fe2, NULL);
	// ensure the connectivity (bidirection linked list)
	if (std::min(count_fe1, count_fe2) % 2 == 1) {
		const Vertex* mid = compOperator->splitEdge(cutPos, (vertex1->getPosition() + vertex2->getPosition()) / 2.0f);
		FrontEdge* fe5 = setFront(mesh->getHalfedge(vertex2, mid), true);
		FrontEdge* fe6 = setFront(mesh->getHalfedge(vertex1, mid), true);
		FrontEdge* fe7 = setFront(mesh->getHalfedge(mid, vertex2), true);
		FrontEdge* fe8 = setFront(mesh->getHalfedge(mid, vertex1), true);
		setNextFe(fe4, fe5);
		setNextFe(fe5, fe8);
		setNextFe(fe8, fe3);
		setNextFe(fe2, fe6);
		setNextFe(fe6, fe7);
		setNextFe(fe7, fe1);
	}
	else if (true || count_fe1 % 2 == 0 && count_fe2 % 2 == 0) {
		FrontEdge* fe5 = setFront(mesh->getHalfedge(vertex2, vertex1), true);
		FrontEdge* fe6 = setFront(mesh->getHalfedge(vertex1, vertex2), true);
		setNextFe(fe4, fe5);
		setNextFe(fe5, fe3);
		setNextFe(fe2, fe6);
		setNextFe(fe6, fe1);
	}
	else {
		return -1; //TODO: suspend process of current frontedge group
	}
	// Then update getFrontEdgeGroup() & frontEdgesGroup
	if (splitFe) {
		pushTailFrontEdgeGroup(fe1);
	}
	pushTailFrontEdgeGroup(fe3);
	return 0;
}

bool FrontEdgeOperator::proceedNextFeLoop(bool reclasssify)
{
	const Halfedge* newFhe = NULL;
	FrontEdge* newFe = nullptr;
	FrontEdge* prevNewFe = nullptr;
	FrontEdge* newFeHead = nullptr;
	if (frontEdgeGroupSize(getFrontEdgeGroup()) == 4) {
		return false;
	}
	FrontEdge* fhe = getFrontEdgeGroup();
	int i = 0;
	do
	{
		if (fhe->getNextFe()->he == fhe->he->getSym()) {
			setFront(fhe->he, false);
			setFront(fhe->getNextFe()->he, false);
			fhe = fhe->getNextFe();
		}
		if (!fhe->needTop)
		{
			if (fhe->getNextFe()->he == fhe->he->getNext()) {
				continue;
			}
			if (!fhe->he->getNext()->getSym()->isBoundary()
				&& !compOperator->isQuad(fhe->he->getNext()->getSym()->getFace())) {
				const Halfedge* lfehe = NULL, * rfehe = NULL;
				const Halfedge* viIter = fhe->he;

				do {
					if (!compOperator->isQuad(viIter->getFace()) && compOperator->isQuad(viIter->getSym()->getFace())) {
						lfehe = viIter;
					}
					if (compOperator->isQuad(viIter->getFace()) && !compOperator->isQuad(viIter->getSym()->getFace())) {
						rfehe = viIter->getSym();
					}
				} while (viIter = viIter->getNext()->getSym(), viIter != fhe->he);
				assert(lfehe && rfehe);
				FrontEdge* lfe = setFront(lfehe, true);
				FrontEdge* rfe = setFront(rfehe, true);
				setNextFe(prevNewFe, lfe);
				setNextFe(lfe, rfe);
				prevNewFe = rfe;
			}
			continue;
		}

		newFhe = fhe->getTop()->getSym();


		if (compOperator->isQuad(newFhe->getFace()) || compOperator->isQuad(newFhe->getSym()->getFace()))
		{
			if (compOperator->isQuad(newFhe->getFace()) && compOperator->isQuad(newFhe->getSym()->getFace())) {
				continue;
			}

			// Build the new front edge list
			// Mark all the front elements.
			newFe = setFront(newFhe, true);
			if (prevNewFe) {
				setNextFe(prevNewFe, newFe);
			}
			if (newFe) {
				prevNewFe = newFe;
			}
			if (!newFeHead) {
				newFeHead = newFe;
			}
		}
		else {
			assert(false);
		}
	} while (setFront(fhe->he, false), fhe = fhe->getNextFe(), fhe != getFrontEdgeGroup());
	if (prevNewFe) {
		setNextFe(prevNewFe, newFeHead);
		updateHeadFrontEdgeGroup(newFeHead);
	}
	else {
		popFrontEdgeGroup();
	}
	return true;
}