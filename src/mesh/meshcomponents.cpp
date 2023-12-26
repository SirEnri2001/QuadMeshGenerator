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

Face* Mesh::createFace() {
	int id = fIdSum++;
	faces[id] = Face(id);
	return &faces[id];
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