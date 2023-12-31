#include "component_operator.h"
#include "../mesh/meshcomponents.h"
#include <queue>

//             v1
//            /  \
//           /    \
//          /      \
//         /        \
//--bhe1->va-oldEdge->vb--bhe2->
//         \        /
//          \      /
//           \    /
//            \  /
//             v2
// Only allowed in triangular mesh
const Vertex* ComponentOperator::splitEdge(Halfedge* oldHe, glm::vec3 pos)
{
	Halfedge* oldEdge = oldHe;
	Vertex* newVertex = mesh->createVertex();
	newVertex->setPosition(glm::vec4(pos,1.0));
	const Vertex* va = oldHe->getSource();
	const Vertex* vb = oldHe->getTarget();
	const Vertex* v1 = oldHe->getNext()->getTarget();
	const Vertex* v2 = oldHe->getSym()->getNext()->getTarget();
	bool isBoundary0 = oldHe->isBoundary();
	bool isBoundary1 = oldHe->getSym()->isBoundary();
	if (!isBoundary0) {
		mesh->deleteFace(oldHe->getFace());
	}
	if (!isBoundary1) {
		mesh->deleteFace(oldHe->getSym()->getFace());
	}
	mesh->deleteEdge(oldEdge);
	mesh->createEdge(va->getMutable(), newVertex);
	mesh->createEdge(newVertex, vb->getMutable());
	if (!isBoundary0) {
		mesh->createEdge(newVertex, v1->getMutable());
		mesh->createFace(mesh->getHalfedge(newVertex, v1));
		mesh->createFace(mesh->getHalfedge(v1, newVertex));
	}
	if (!isBoundary1) {
		mesh->createEdge(newVertex, v2->getMutable());
		mesh->createFace(mesh->getHalfedge(newVertex, v2));
		mesh->createFace(mesh->getHalfedge(v2, newVertex));
	}
	return newVertex;
}

Halfedge* ComponentOperator::swapEdge(Halfedge* oldEdge)
{
	Halfedge* he1 = oldEdge;
	Halfedge* he2 = oldEdge->getSym();
	Halfedge* hePrev = oldEdge->getPrev();

	Vertex* va = he1->getSource();
	Vertex* vb = he1->getTarget();
	Vertex* v1 = he1->getNext()->getTarget();
	Vertex* v2 = he2->getNext()->getTarget();
	mesh->deleteFace(he1->getFace());
	mesh->deleteFace(he2->getFace());
	mesh->deleteEdge(oldEdge);

	mesh->createEdge(v1, v2);
	mesh->createFace(mesh->getHalfedge(v1, v2));
	mesh->createFace(mesh->getHalfedge(v2, v1));
	return hePrev->getPrev();
}

const Halfedge* ComponentOperator::splitFace(Vertex* v1, Vertex* v2) {
	// find the face that contains v1 and v2
	Face* face = nullptr;
	Halfedge* he = v1->getHalfedge();

	do {
		Halfedge* he1 = he;
		do {
			if (he1->getTarget() == v2) {
				face = he->getFace();
			}
		} while (he1 = he1->getNext(), he1 != he);
	} while (he = he->getNext()->getSym(), he != v1->getHalfedge());
	mesh->deleteFace(face);
	mesh->createEdge(v1, v2);
	mesh->createFace(mesh->getHalfedge(v1, v2));
	mesh->createFace(mesh->getHalfedge(v2, v1));
	return mesh->getHalfedge(v1, v2);
}

void ComponentOperator::deleteVertexMergeFace(Vertex* tar)
{
	if (!tar->getHalfedge()) {
		mesh->deleteVertex(tar);
		return;
	}
	while (tar->getHalfedge()) {
		deleteEdgeMergeFace(tar->getHalfedge());
	}
	mesh->deleteVertex(tar);
	return;
}

void ComponentOperator::deleteEdgeMergeFace(Halfedge* tar)
{
	// <--he1-(va)<-he4---
	//         ^|
	//      hea||heb
	//         |v
	//---he2->(vb)--he3-->
	Vertex* va = tar->getTarget();
	Vertex* vb = tar->getSource();
	Halfedge* hea = tar;
	Halfedge* he1 = tar->getNext();
	Halfedge* he2 = tar->getPrev();
	mesh->deleteFace(tar->getFace());
	mesh->deleteFace(tar->getSym()->getFace());
	mesh->deleteEdge(tar);
	mesh->createFace(he1);
	if (he2->isBoundary()) {
		// island hole
		mesh->createFace(he2);
	}
	return;
}

void ComponentOperator::clearFace(std::vector<Halfedge*> heVector) {
	std::vector<Vertex*> vVector;
	for (auto& he : heVector) {
		vVector.push_back(he->getSource());
	}
	for (int i = 0; i < heVector.size(); i++) {
		//           v2
		//     ^     ^
		//      \   he2
		//    curHe  |
		//        \  |
		//         \ |
		// v0--he1-->v1
		// 
		//  he1==heVector[i]
		std::list<Vertex*> q;
		Halfedge* he1 = heVector[i];
		Halfedge* he2 = heVector[(i + 1) % heVector.size()];
		Vertex* v0 = he1->getSource();
		Vertex* v1 = he1->getTarget();
		Vertex* v2 = he2->getTarget();
		Halfedge* curHe = he1->getNext();
		std::unordered_map<Vertex*, bool> markDelete;
		while (curHe != he2)
		{
			auto vViter = find(vVector.begin(), vVector.end(), curHe->getTarget());
			if (vViter != vVector.end()) {
				curHe =  curHe->getSym()->getNext();
				deleteEdgeMergeFace(mesh->getHalfedge(v1, *vViter));
				continue;
			}
			if (std::find(q.begin(), q.end(), curHe->getTarget())!=q.end()) {
				q.push_back(curHe->getTarget());
			}
			curHe = curHe->getSym()->getNext();
		}
		while (!q.empty()) {
			//HalfedgeHandle he = vertexHalfedge(q.front());
			//assert(q.front() == halfedgeTarget(vertexHalfedge(q.front())));
			if (q.front()->getHalfedge()) {
				Halfedge* he = q.front()->getHalfedge();
				do {
					if (std::find(vVector.begin(), vVector.end(), he->getSource()) == vVector.end()
						&& std::find(q.begin(), q.end(), he->getSource()) != q.end()) {
						q.push_back(he->getSource());
					}
				} while (he = he->getNext()->getSym(), he != q.front()->getHalfedge());
			}
			deleteVertexMergeFace(q.front());
			q.pop_back();
		}
	}
}

void ComponentOperator::clearFace(std::initializer_list<Halfedge*> heList)
{
	clearFace(std::vector<Halfedge*>(heList));
}

int ComponentOperator::numQuad(const Halfedge* edge)
{
	int num = 0;
	if (edge->getFace() != NULL && isQuad(edge->getFace()))
		num++;
	if (edge->getSym()->getFace() != NULL && isQuad(edge->getSym()->getFace()))
		num++;
	return num;
}

int ComponentOperator::numQuad(const Halfedge* edge, const Halfedge* edge1)
{
	int num = 0;
	if (edge->getFace() != NULL && isQuad(edge->getFace()))
		num++;
	if (edge1->getFace() != NULL && isQuad(edge1->getFace()))
		num++;
	return num;
}

int ComponentOperator::numTriangles(const Vertex* vert) {
	assert(vert != NULL);
	int num = 0;
	const Halfedge* he = vert->getHalfedge();
	if (he == nullptr) {
		return 0;
	}
	do {
		if (!isQuad(he->getFace()))
			num++;
	} while (he = he->getNext()->getSym(), he != vert->getHalfedge());
	return num;
}

int ComponentOperator::numQuad(const Vertex* vert) {
	assert(vert != NULL);
	int num = 0;
	const Halfedge* he = vert->getHalfedge();
	if (he == nullptr) {
		return 0;
	}
	do {
		if (isQuad(he->getFace()))
			num++;
	} while (he = he->getNext()->getSym(), he != vert->getHalfedge());
	return num;
}


bool ComponentOperator::isQuad(const Face* face)
{
	assert(face);
	int vertNum = 0;
	const Halfedge* he = face->getHalfedge();
	do {
		vertNum++;
	} while (he = he->getNext(), he != face->getHalfedge());
	return vertNum == 4;
}
