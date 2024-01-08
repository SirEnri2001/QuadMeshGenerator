#include "meshcomponents.h"
#include <memory>
#include "../thread_support/thread_support.h"

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
	meshAssert = std::make_unique<MeshAssert>();
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

const Face* Mesh::createFace(Halfedge* he) {
	if (integrityCheck) {
		validate(he);
	}
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
	face->setHalfedge(he);
	return face;
}

void Mesh::deleteFace(Face* face) {
	face->setDeleted(true);
	Halfedge* he = face->getHalfedge();
	do
	{
		he->setFace(nullptr);

	} while (he = he->getNext(), he != face->getHalfedge());
	faces.erase(faces.find(face->getId()));
}

void Mesh::deleteEdge(Halfedge* he) {
	//  <-v1outbhe-[v1]<-v1inbhe-
	//              ^|
	//              |he12
	//           he21|
	//              |v
	//   -v2inbhe->[v2]-v2outbhe->
	Halfedge* he21 = he;
	Halfedge* he12 = he->getSym();
	Halfedge* v1outbhe = he21->getNext();
	Halfedge* v1inbhe = he12->getPrev();
	Halfedge* v2inbhe = he21->getPrev();
	Halfedge* v2outbhe = he12->getNext();
	he->setDeleted(true);
	he->getSym()->setDeleted(true);
	if (v1outbhe != he12) {
		he21->getTarget()->setHalfedge(v1inbhe);
		v1inbhe->setNext(v1outbhe);
	}
	else {
		he->getTarget()->setHalfedge(nullptr);
	}
	if (v2outbhe != he21) {
		he->getSource()->setHalfedge(v2inbhe);
		v2inbhe->setNext(v2outbhe);
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

const Halfedge* Mesh::getBoundary(const Vertex* v) {
	if (v->getHalfedge() == nullptr) {
		return nullptr;
	}
	const Halfedge* he = v->getHalfedge();
	do {
		if (he->isBoundary()) {
			return he;
		}
	} while (he = he->getNext()->getSym(), he != v->getHalfedge());
	return nullptr;
}

//  <-v1outbhe-[v1]<-v1inbhe-
//              ^|
//              |he12
//           he21|
//              |v
//   -v2inbhe->[v2]-v2outbhe->
void Mesh::createEdge(Vertex* v1, Vertex* v2) {
	const Halfedge* v1inbhe = getBoundary(v1);
	const Halfedge* v1outbhe = v1inbhe ? v1inbhe->getNext() : nullptr;
	const Halfedge* v2inbhe = getBoundary(v2);
	const Halfedge* v2outbhe = v2inbhe ? v2inbhe->getNext() : nullptr;
	const Halfedge* he12 = createHalfedge(v1, v2);
	const Halfedge* he21 = createHalfedge(v2, v1);
	he12->getMutable()->setSym(he21->getMutable());
	if (v1inbhe) {
		v1inbhe->getMutable()->setNext(he12->getMutable());
		he21->getMutable()->setNext(v1outbhe->getMutable());
	}
	else {
		he21->getMutable()->setNext(he12->getMutable());
	}
	if (v2inbhe) {
		he12->getMutable()->setNext(v2outbhe->getMutable());
		v2inbhe->getMutable()->setNext(he21->getMutable());
	}
	else {
		he12->getMutable()->setNext(he21->getMutable());
	}
}

void Component::setId(ID id) {
	validate(this);
	this->id = id;
}

ID Component::getId() const {
	validate(this);
	return this->id;
}

bool Component::isValid() const {
	validate(this);
	return valid;
}

bool Component::isDisplayed() const {
	validate(this);
	return displayed;
}

void Component::setDeleted(bool val) {
	validate(this);
	deleted = val;
}

Halfedge* Vertex::getHalfedge() {
	validate(this);
	return this->halfedge;
}
const Halfedge* Vertex::getHalfedge() const {
	validate(this);
	return this->halfedge;
}
void Vertex::setHalfedge(Halfedge* halfedge) {
	validate(this);
	this->halfedge = halfedge;
}
void Vertex::setPosition(glm::vec4 pos) {
	validate(this);
	position = pos;
}
glm::vec4 Vertex::getPosition() const {
	validate(this);
	return position;
}

glm::vec3 Vertex::getNormal() const {
	validate(this);
	return normal;
}

void Vertex::setNormal(glm::vec3 n) {
	validate(this);
	normal = n;
}

void Vertex::setUV(glm::vec2 uv) {
	validate(this);
	this->uv = uv;
}

glm::vec2 Vertex::getUV() const {
	validate(this);
	return uv;
}

bool Vertex::isBoundary() const {
	validate(this);
	//return boundary;
	const Halfedge* he = halfedge;
	do {
		if (he->isBoundary()) {
			return true;
		}
	} while (he = he->getNext()->getSym(), he != halfedge);
	return false;
}

Vertex* Vertex::getMutable() const {
	validate(this);
	return const_cast<Vertex*>(this);
}

Vertex* Halfedge::getSource() {
	validate(this);
	return this->source;
}
Vertex* Halfedge::getTarget() {
	validate(this);
	return this->target;
}
Halfedge* Halfedge::getNext() {
	validate(this);
	return next;
}
Halfedge* Halfedge::getPrev() {
	validate(this);
	return prev;
}
Halfedge* Halfedge::getSym() {
	validate(this);
	return sym;
}
Face* Halfedge::getFace() {
	validate(this);
	return face;
}
void Halfedge::setSource(Vertex* v) {
	validate(this);
	source = v;
}
void Halfedge::setTarget(Vertex* v) {
	validate(this);
	target = v;
}
void Halfedge::setSym(Halfedge* he) {
	validate(this);
	sym = he;
	he->sym = this;
}
void Halfedge::setNext(Halfedge* he) {
	validate(this);
	next = he;
	he->prev = this;
}
void Halfedge::setPrev(Halfedge* he) {
	validate(this);
	prev = he;
	he->next = this;
}
void Halfedge::setFace(Face* f) {
	validate(this);
	face = f;
	if (f) {
		target->boundary = false;
		source->boundary = false;
	}
	else {
		target->boundary = true;
		source->boundary = true;
	}
}
const Vertex* Halfedge::getSource() const {
	validate(this);
	return this->source;
}
const Vertex* Halfedge::getTarget() const {
	validate(this);
	return this->target;
}
const Halfedge* Halfedge::getNext() const {
	validate(this);
	return next;
}
const Halfedge* Halfedge::getPrev() const {
	validate(this);
	return prev;
}
const Halfedge* Halfedge::getSym() const {
	validate(this);
	return sym;
}
const Face* Halfedge::getFace() const {
	validate(this);
	return face;
}
bool Halfedge::isBoundary() const {
	validate(this);
	return face == nullptr;
}
float Halfedge::getLength() const {
	validate(this);
	return glm::length(target->getPosition() - source->getPosition());
}
Halfedge* Halfedge::getMutable() const {
	validate(this);
	return const_cast<Halfedge*>(this);
}
const Halfedge* Face::getHalfedge() const {
	validate(this);
	return this->halfedge;
}
Halfedge* Face::getHalfedge() {
	validate(this);
	return this->halfedge;
}
void Face::setHalfedge(Halfedge* he) {
	validate(this);
	halfedge = he;
}
Face* Face::getMutable() const {
	validate(this);
	return const_cast<Face*>(this);
}
const Halfedge* Mesh::getHalfedge(const Vertex* source, const Vertex* target) const {
	if (integrityCheck) {
		validate(this);
	}
	auto iter = vertexHalfedge.find({ source->getId(), target->getId() });
	if (iter == vertexHalfedge.end()) {
		return nullptr;
	}
	return iter->second;
}

const Halfedge* Mesh::getHalfedge(const Vertex* source, const Vertex* target) {
	if (integrityCheck) {
		validate(this);
	}
	auto iter = vertexHalfedge.find({ source->getId(), target->getId() });
	if (iter == vertexHalfedge.end()) {
		return nullptr;
	}
	return iter->second;
}

const std::unordered_map<ID, Vertex>& Mesh::getVertices() const {
	if (integrityCheck) {
		validate(this);
	}
	return vertices;
}

const std::unordered_map<ID, Halfedge>& Mesh::getHalfedges() const {
	if (integrityCheck) {
		validate(this);
	}
	return halfedges;
}

const std::unordered_map<ID, Face>& Mesh::getFaces() const {
	if (integrityCheck) {
		validate(this);
	}
	return faces;
}

const Vertex* Mesh::vertexAt(ID id) {
	if (integrityCheck) {
		validate(this);
	}
	return &vertices[id];
}

const Halfedge* Mesh::halfedgeAt(ID id) {
	if (integrityCheck) {
		validate(this);
	}
	return &halfedges[id];
}

const Face* Mesh::faceAt(ID id) {
	if (integrityCheck) {
		validate(this);
	}
	return &faces[id];
}

void Mesh::setIntegrityCheck(bool val) {
	integrityCheck = val;
}

ID Mesh::getHalfedgeIdTotal() {
	return heIdSum;
}

ID Mesh::getFaceIdTotal() {
	return fIdSum;
}

ID Mesh::getVertexIdTotal() {
	return vIdSum;
}