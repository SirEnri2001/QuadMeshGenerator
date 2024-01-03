#include "meshcomponents.h"
#include <memory>

Component::Component(ID id) : id(id) {
}

Component::Component(const Component& comp) {
	id = comp.id;
	valid = comp.valid;
	displayed = comp.displayed;
}

Component::~Component()
{

}

Vertex::Vertex(ID id) : Component(id) {

}

Vertex::Vertex() : Component(-1) {

}

Vertex::Vertex(const Vertex& v) : Component(v) {
	halfedge = v.halfedge;
}

Vertex::~Vertex()
{

}

Halfedge::Halfedge(ID id) : Component(id) {

}
Halfedge::Halfedge(const Halfedge& he) :Component(he) {
	source = he.source;
	target = he.target;
	next = he.next;
	prev = he.prev;
	sym = he.sym;
	face = he.face;
}

Halfedge::Halfedge() : Component(-1) {

}
Halfedge::~Halfedge()
{

}

//void setSym(Halfedge* he1, Halfedge* he2) {
//	he1->setSym(he2);
//}
//
//void setNext(Halfedge* he, Halfedge* heNext) {
//	he->setNext(heNext);
//}
//
//void setPrev(Halfedge* he, Halfedge* hePrev) {
//	he->setPrev(hePrev);
//}


Face::Face(ID id) : Component(id)
{

}

Face::Face() : Component(-1)
{

}

Face::Face(const Face& face) : Component(face) {
	halfedge = face.halfedge;
}

Face::~Face() {

}

Vertex* Mesh::createVertex() {
	int id = vIdSum++;
	vertices[id] = Vertex(id);
	return &vertices[id];
}

Halfedge* Mesh::createHalfedge(Vertex* source, Vertex* target) {
	int id = heIdSum++;
	halfedges[id] = Halfedge(id);
	Halfedge* he = &halfedges[id];
	he->setTarget(target);
	he->setSource(source);
	vertexHalfedge[{source->getId(), target->getId()}] = he;
	target->setHalfedge(he);
	return &halfedges[id];
}

Mesh::Mesh() {

}


//template<class ComponentT>
//class MeshIterator {
//	Mesh* mesh;
//	std::unordered_map<ID, ComponentT>* components;
//	std::unordered_map<ID, ComponentT>::const_iterator iter;
//	ID index = 0;
//public:
//	MeshIterator(Mesh* mesh, ComponentType type) :mesh(mesh) {
//		switch (type)
//		{
//		case VERTEX:
//			components = mesh->getVertices();
//			break;
//		case HALFEDGE:
//			components = mesh->getHalfedges();
//			break;
//		case FACE:
//			components = mesh->getVertices();
//			break;
//		default:
//			break;
//		}
//		iter = components->cbegin();
//	}
//};

MeshRecorder::MeshRecorder(Mesh* mesh) :mesh(mesh)
{

}

const Face* Mesh::createFace(Halfedge* he) {
	int id = fIdSum++;
	faces[id] = Face(id);
	Face* face = &faces[id];
	if (he==nullptr) {
		return face;
	}
	Halfedge* he1 = he;
	do
	{
		he1->setFace(face);

	} while (he1 = he1->getNext(), he1 != he);
	return face;
}

void Mesh::deleteFace(Face* face) {
	Halfedge* he = face->getHalfedge();
	do
	{
		he->setFace(nullptr);

	} while (he = he->getNext(), he != face->getHalfedge());
	faces.erase(faces.find(face->getId()));
}

void Mesh::deleteEdge(Halfedge* he) {

	// set source & target vertex to another valid halfedge;
	if (he->getNext()->getSym() != he) {
		he->getTarget()->setHalfedge(he->getNext()->getSym());
		he->getPrev()->setNext(he->getSym()->getNext());
	}
	else {
		he->getTarget()->setHalfedge(nullptr);
	}
	if (he->getPrev()->getSym() != he) {
		he->getSource()->setHalfedge(he->getPrev()->getSym());
		he->getSym()->getPrev()->setNext(he->getNext());
	}
	else {
		he->getSource()->setHalfedge(nullptr);
	}

	Vertex* v1 = he->getSource();
	Vertex* v2 = he->getTarget();
	vertexHalfedge[{v1->getId(), v2->getId()}] = nullptr;
	vertexHalfedge[{v2->getId(), v1->getId()}] = nullptr;
	halfedges.erase(halfedges.find(he->getSym()->getId()));
	halfedges.erase(halfedges.find(he->getId()));
}

Halfedge* Mesh::getBoundary(Vertex* v) {
	if (v->getHalfedge() == nullptr) {
		return nullptr;
	}
	Halfedge* he = v->getHalfedge();
	do {
		if (he->isBoundary()) {
			return he;
		}
	} while (he = he->getNext()->getSym(), he != v->getHalfedge());
	return nullptr;
}

void Mesh::createEdge(Vertex* v1, Vertex* v2) {
	Halfedge* v1inbhe = getBoundary(v1);
	Halfedge* v1outbhe = v1inbhe ? v1inbhe->getNext() : nullptr;
	Halfedge* v2inbhe = getBoundary(v2);
	Halfedge* v2outbhe = v2inbhe ? v2inbhe->getNext() : nullptr;
	Halfedge* he12 = createHalfedge(v1, v2);
	Halfedge* he21 = createHalfedge(v2, v1);
	he12->setSym(he21);
	if (v1inbhe) {
		v1inbhe->setNext(he12);
		he21->setNext(v1outbhe);
	}
	else {
		he21->setNext(he12);
	}
	if (v2inbhe) {
		he12->setNext(v2outbhe);
		v2inbhe->setNext(he21);
	}
	else {
		he12->setNext(he21);
	}
}