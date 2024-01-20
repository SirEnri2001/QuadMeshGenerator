#include "frontedge_operator.h"
#include "frontedge.h"
#include "../mesh/meshcomponents.h"
#include "component_operator.h"
#include "sidedefine_operator.h"
#include "../thread_support/thread_support.h"
#include "../mesh/meshdisplay.h"
#include <algorithm>

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

FrontEdgeOperator::FrontEdgeOperator(Mesh* mesh, MeshDisplay* display) :MeshOperator(mesh, display) {
	for (auto& idV : mesh->getVertices()) {
		frontEdgeCount[&idV.second] = 0;
	}
}

void FrontEdgeOperator::create(std::shared_ptr<ComponentOperator>& compOper, std::shared_ptr<SideDefineOperator>& sideOper) {
	compOperator = compOper;
	sideOperator = sideOper;
	initFrontEdgeGroup();
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
		FrontEdge* fe = nullptr;
		do {
			FrontEdge* curFe = setFront(curHe->getSym(), true);
			FrontEdge* nextFe = setFront(curHe->getPrev()->getSym(), true);
			setNextFe(curFe, nextFe);
			if (fe == nullptr) {
				fe = curFe;
			}
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
	vNew = compOperator->splitEdge(longestHE->getMutable(),
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
	/*const Halfedge* he = v->getHalfedge();
	do {
		if (isFront(he) || isFront(he->getSym())) {
			return true;
		}
	} while (he = he->getNext()->getSym(), he != v->getHalfedge());
	return false;*/
	return frontEdgeCount[v] != 0;
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
	return std::find(frontEdgeGroups.begin(), frontEdgeGroups.end(), he) != frontEdgeGroups.end();
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
	return -1;
}

int FrontEdgeOperator::seperateFrontLoop(FrontEdge* fe2, FrontEdge* fe4) {
	if (!fe2 || !fe4) {
		display->create();
		display->createFrame();
		asserts->onPause();
		throw "Invalid argument";
	}
	const Vertex* vertex2 = fe4->he->getTarget();
	const Vertex* vertex1 = fe2->he->getTarget();
	FrontEdge* fe1 = fe4->getNextFe();
	FrontEdge* fe3 = fe2->getNextFe();

	removeFrontEdgeGroup(fe1);
	removeFrontEdgeGroup(fe2);

	bool splitFe = false;
	FrontEdge* feIter = fe1;
	int feCount = 1;
	int feCount1 = 1;
	do {
		feCount++;
		if (feIter == fe2) {
			splitFe = true;
			break;
		}
		if (feIter == fe4) {
			splitFe = false;
			break;
		}
	} while (feIter = feIter->getNextFe());
	feIter = fe3;
	do {
		feCount1++;
		if (feIter == fe4) {
			if (!splitFe) {
				asserts->onPause();
				assert(false);
			}
			break;
		}
		if (feIter == fe2) {
			if (splitFe) {
				asserts->onPause();
				assert(false);
			}
			break;
		}
	} while (feIter = feIter->getNextFe());
	if ((feCount + feCount1) % 2 != 0) {
		switchFrontEdgeGroup();
		return -1;
	}
	if (feCount%2==0 || !splitFe) {
		edgeRecovery(vertex1->getMutable(), vertex2->getMutable());
		FrontEdge* v1v2 = setFront(mesh->getHalfedge(vertex1, vertex2), true);
		FrontEdge* v2v1 = setFront(mesh->getHalfedge(vertex2, vertex1), true);
		setNextFe(fe2, v1v2);
		setNextFe(v1v2, fe1);
		setNextFe(fe4, v2v1);
		setNextFe(v2v1, fe3);
	}
	// <--fe1-(v2)<-fe4---
	//         ^|
	//         |(cutPos)
	//         |v
	//---fe2->(v1)--fe3-->

	if (!splitFe) {
		pushTailFrontEdgeGroup(fe1);
		return 0;
	}
	if (feCount % 2 == 0) {
		pushTailFrontEdgeGroup(fe1);
		pushTailFrontEdgeGroup(fe3);
		return 0;
	}
	const Vertex* vMid = compOperator->splitEdge(mesh->getHalfedge(vertex1, vertex2)->getMutable(),
		glm::vec3(vertex1->getPosition() + vertex2->getPosition()) * 0.5f);
	FrontEdge* v1vM = setFront(mesh->getHalfedge(vertex1, vMid), true);
	FrontEdge* vMv2 = setFront(mesh->getHalfedge(vMid, vertex2), true);
	FrontEdge* v2vM = setFront(mesh->getHalfedge(vertex2, vMid), true);
	FrontEdge* vMv1 = setFront(mesh->getHalfedge(vMid, vertex1), true);
	setNextFe(fe2, v1vM);
	setNextFe(v1vM, vMv2);
	setNextFe(vMv2, fe1);
	setNextFe(fe4, v2vM);
	setNextFe(v2vM, vMv1);
	setNextFe(vMv1, fe3);
	pushTailFrontEdgeGroup(fe2);
	pushTailFrontEdgeGroup(fe4);
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
		newFhe = fhe->he->getPrev()->getPrev()->getSym();
		if (compOperator->isQuad(newFhe->getFace())) {
			continue;
		}
		newFe = setFront(newFhe, true);
		if (newFeHead == nullptr) {
			newFeHead = newFe;
		}
		if (prevNewFe != nullptr) {
			setNextFe(prevNewFe, newFe);
		}
		prevNewFe = newFe;
	} while (fhe = fhe->getNextFe(), fhe != getFrontEdgeGroup());
	if (prevNewFe) {
		assert(prevNewFe->he->getTarget() == newFeHead->he->getSource());
		setNextFe(prevNewFe, newFeHead);
		updateHeadFrontEdgeGroup(newFeHead);
	}
	else {
		popFrontEdgeGroup();
	}
	return true;
}

typedef struct FaceIterTreeNode
{
	struct FaceIterTreeNode* parent;
	std::vector<std::unique_ptr<struct FaceIterTreeNode>> children;
	const Halfedge* parentConnectedHe;
	const Face* face;
	int childNum;
}FaceIterTreeNode;

// Nc & Nd must not connected
// Topology version of calculateRambdaSet
std::list<const Halfedge*>* FrontEdgeOperator::calculateRambdaSet(const Vertex* Nc, const Vertex* Nd) {
	std::list<std::unique_ptr<FaceIterTreeNode>> roots;
	std::list<FaceIterTreeNode*>faceIterTrees;
	std::list<const Face*> traversedFace;
	const Halfedge* he = Nc->getHalfedge();
	// Init the face-face tree
	do
	{
		if (he->isBoundary() || compOperator->isQuad(he->getFace())) {
			continue;
		}
		roots.push_back(std::make_unique<FaceIterTreeNode>());
		traversedFace.push_back(he->getFace());
		FaceIterTreeNode* root = roots.back().get();
		root->parent = NULL;
		root->parentConnectedHe = NULL;
		//root->childNum = 0;
		root->face = he->getFace();
		faceIterTrees.push_back(root);
	} while (he = he->getNext()->getSym(), he != Nc->getHalfedge());
	assert(faceIterTrees.size() > 0); // find at least one triangular face around Nc
	FaceIterTreeNode* targetTree = NULL;
	// BFS traverse f-f tree until find the target vertex Nd
	while (targetTree == NULL) {
		if (faceIterTrees.size() == 0) {
			display->create();
			display->createFrame();
			display->markVertex(Nc);
			display->markVertex(Nd);
			asserts->onPause();
			throw "calculate lambda set error";
		}
		FaceIterTreeNode* curTree = faceIterTrees.front();
		faceIterTrees.pop_front();
		int childIdx = 0;
		const Halfedge* he = curTree->face->getHalfedge();
		do {
			const Face* attachedFace = he->getSym()->getFace();
			if (attachedFace && !compOperator->isQuad(attachedFace)
				&& find(traversedFace.begin(), traversedFace.end(), attachedFace) == traversedFace.end()
				&& !isFront(he)
				&& (!isPushingFront || !sideOperator->isSide(he) && !sideOperator->isSide(he->getSym()))
				)
			{
				traversedFace.push_back(attachedFace);
				std::unique_ptr<FaceIterTreeNode> unewNode = std::make_unique<FaceIterTreeNode>();
				FaceIterTreeNode* newNode = unewNode.get();
				newNode->face = attachedFace;
				newNode->parentConnectedHe = he->getSym();
				newNode->parent = curTree;
				newNode->children.push_back(std::move(unewNode));
				curTree->childNum++;
				if (he->getSym()->getNext()->getTarget() == Nd) {
					targetTree = newNode;
					break;
				}
				else {
					faceIterTrees.push_back(newNode);
				}
			}
		} while (he = he->getNext(), he != curTree->face->getHalfedge());
	}
	std::list<const Halfedge*>* rambdaSet = new std::list<const Halfedge*>();
	while (targetTree->parent) {
		rambdaSet->push_back(targetTree->parentConnectedHe);
		targetTree = targetTree->parent;
	}
	return rambdaSet;
}

const Halfedge* FrontEdgeOperator::edgeRecovery(Vertex* Nc, Vertex* Nd) {
	assert(Nd != Nc); //411 714
	while (mesh->getHalfedge(Nc, Nd) == NULL) {
		auto lambdaSet = calculateRambdaSet(Nc, Nd);
		assert(lambdaSet);
		while (lambdaSet->size() > 0) {
			if (mesh->getHalfedge(
				lambdaSet->front()->getNext()->getTarget(),
				lambdaSet->front()->getSym()->getNext()->getTarget()
			)) {
				assert(lambdaSet->size() > 1);
				lambdaSet->push_back(lambdaSet->front());
				lambdaSet->pop_front();
			}
			compOperator->swapEdge(lambdaSet->front()->getMutable());
			lambdaSet->pop_front();
		}
	}
	return mesh->getHalfedge(Nc, Nd);
}

const Vertex* ComponentOperator::mergeEdge(Vertex* va, Vertex* vb) {
	//     v1
	//    /  \   /
	//   /    \ /
	//  va----vb---
	//   \    / \
	//    \  /   \
	//     v2
	const Halfedge* he1 = mesh->getHalfedge(va, vb);
	const Halfedge* he2 = he1->getSym();
	while (
		he1->getNext()->getSym()->getPrev()->getSource()
		== he1->getPrev()->getSym()->getNext()->getTarget()) {
		deleteVertexMergeFace(he1->getNext()->getTarget()->getMutable());
	}
	const Vertex* v1 = he1->getNext()->getTarget();
	const Vertex* v2 = he2->getNext()->getTarget();
	std::list<const Vertex*> vbConnectedVertices;

	Halfedge* he = vb->getHalfedge();
	do {
		if (he->getSource() != va && he->getSource() != v1 && he->getSource() != v2) {
			Halfedge* outHeAttr = new Halfedge(*he->getSym());
			Halfedge* inHeAttr = new Halfedge();
			vbConnectedVertices.push_back(he->getSource());
		}
	} while (he = he->getNext()->getSym(), he != vb->getHalfedge());
	deleteVertexMergeFace(vb);
	for (std::list<const Vertex*>::iterator iter = vbConnectedVertices.begin(); iter != vbConnectedVertices.end(); ++iter) {
		splitFace(va, (*iter)->getMutable());
	}
	return va;
}

void FrontEdgeOperator::trianglualteFrontLoopInterior(FrontEdge* fe) {
	std::vector<Halfedge*> heList;
	FrontEdge* fe1 = fe;
	do {
		heList.push_back(fe1->he->getMutable());
	} while (fe1 = fe1->getNextFe(), fe1 != fe);
	compOperator->clearFace(heList);

}