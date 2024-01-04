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
	//mesh->createEdge(va->getMutable(), newVertex);
	//mesh->createEdge(newVertex, vb->getMutable());
	//if (!isBoundary0) {
	//	mesh->createEdge(newVertex, v1->getMutable());
	//	mesh->createFace(mesh->getHalfedge(newVertex, v1));
	//	mesh->createFace(mesh->getHalfedge(v1, newVertex));
	//}
	//if (!isBoundary1) {
	//	mesh->createEdge(newVertex, v2->getMutable());
	//	mesh->createFace(mesh->getHalfedge(newVertex, v2));
	//	mesh->createFace(mesh->getHalfedge(v2, newVertex));
	//}
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

float ComponentOperator::angle(glm::vec3 cp1, const Vertex* pivot, glm::vec3 cp2) {
	using namespace glm;
	vec3 v1 = cp1 - vec3(pivot->getPosition());
	vec3 v2 = cp2 - vec3(pivot->getPosition());
	return degrees(acos_limited(dot(v1,v2) / (length(v1) * length(v2))));
}

float ComponentOperator::angle(const Halfedge* bhe, const Halfedge* bhe_next) {
	using namespace glm;
	vec3 BA = vec3(bhe->getSource()->getPosition() - bhe->getTarget()->getPosition());
	vec3 BC = vec3(bhe_next->getTarget()->getPosition() - bhe_next->getSource()->getPosition());
	vec3 normal = cross(vec3(bhe->getNext()->getTarget()->getPosition() - bhe->getTarget()->getPosition()),
		vec3(bhe->getSource()->getPosition() - bhe->getTarget()->getPosition())
	);
	vec3 p = bhe->getTarget()->getPosition();
	float res;
	if (dot(normal, cross(BC,BA)) < 0.0) {
		res = 360.0F - acos_limited(dot(BC, BA) / (length(BC) * length(BA))) / M_PI * 180;
	}
	else {
		res = acos_limited(dot(BA, BC) / (length(BA) * length(BC))) / M_PI * 180;
	}
	if (res != res) {
		assert(false);
	}
	return res;
}

float ComponentOperator::angle(glm::vec3 cpoint, const Halfedge* he) {
	using namespace glm;
	vec3 cp1 = he->getTarget()->getPosition() - he->getSource()->getPosition();
	vec3 cp2 = cpoint - vec3(he->getSource()->getPosition());
	/*if (normalOfVertex(mesh, halfedgeSource(he)) * (cp2 ^ cp1) < 0.0) {
		return 360.0 - acos(cp1 * cp2 / (cp1.norm() * cp2.norm())) / PI * 180;
	}*/
	return acos_limited(dot(cp2, cp1) / (length(cp1) * length(cp2))) / M_PI * 180;
}

// calculate the bisector of angle between he1 & he1_next. result based on local coordination.
// he1 and he1_next must be connected and continous. 
glm::vec4 ComponentOperator::bisect(const Halfedge* he1, const Halfedge* he1_next) {
	using namespace glm;
	vec3 joint;
	//assert(he1 == nextBoundaryHalfedge(he1_next) || he1_next == nextBoundaryHalfedge(he1));
	//--he1-->--he1_next-->
	joint = he1->getTarget()->getPosition();
	//<--v1--.--v2-->
	vec3 cp1 = vec3(he1->getSource()->getPosition()) - joint;
	vec3 cp2 = vec3(he1_next->getTarget()->getPosition()) - joint;
	cp1 = normalize(cp1);
	cp2 = normalize(cp2);
	vec3 cp3 = cp1 + cp2;
	vec3 localNormal = normalVertex(he1->getSource());
	localNormal = normalize(localNormal);
	if (length(cross(cp2, cp1)) < 1e-4 && dot(cp2, cp1) < 0.0) {
		vec3 bisector = cross(cp1, localNormal);
		bisector = normalize(bisector);
		return vec4(bisector, 1.0);
	}
	if (dot(cross(cp2, cp1), localNormal) < 0) {
		cp3 = -cp3;
	}
	return vec4(normalize(cp3), 1.0);
}

glm::vec3 ComponentOperator::normalVertex(const Vertex* vertex) {
	using namespace glm;
	vec3 vsum(0.0, 0.0, 0.0);
	const Halfedge* he = vertex->getHalfedge();
	do {
		vec3 a, b, c;
		a = he->getTarget()->getPosition();
		b = he->getNext()->getTarget()->getPosition();
		c = he->getPrev()->getSource()->getPosition();
		vsum += cross(c - b, a - b);
	} while (he = he->getNext()->getSym(), he != vertex->getHalfedge());
	return normalize(vsum);
}

int ComponentOperator::faceEdges(const Face* face) {
	int count = 0;
	const Halfedge* he = face->getHalfedge();
	do {
		count++;
	} while (he = he->getNext(), he != face->getHalfedge());
	return count;
}

float ComponentOperator::acos_limited(float x) {
	if (x > 1.0) {
		std::cerr << "WARNING: acos_limited: x > 1.0" << std::endl;
		return 0.0;
	}
	if (x < -1.0) {
		std::cerr << "WARNING: acos_limited: x < -1.0" << std::endl;
		return M_PI;
	}
	return acos(x);
}

ComponentOperator::ComponentOperator(Mesh* mesh) : MeshInteriorOperator(mesh) {

}

