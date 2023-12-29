#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <glm.hpp>
class Vertex;
class Halfedge;
class Face;
class Mesh;
class MeshOperator;
class MeshDisplay;
class MeshIO;
class MeshRecorder;

typedef std::pair<Halfedge*, Halfedge*> Edge;

enum ComponentType {
	VERTEX, HALFEDGE, FACE
};

typedef unsigned int ID;

class Component {
protected:
	ID id;
	bool valid = true;
	bool displayed = false;
public:
	Component(ID id);

	Component(const Component& comp);

	~Component();

	inline void setId(ID id) {
		this->id = id;
	}

	inline ID getId() const {
		return this->id;
	}

	inline bool isValid() const {
		return valid;
	}

	inline bool isDisplayed() const {
		return displayed;
	}
};

class Vertex : public Component {
protected:
	Halfedge* halfedge = nullptr;
	glm::vec4 position = glm::vec4(0,0,0,1);
	glm::vec3 normal = glm::vec3(0,0,0);
	glm::vec2 uv = glm::vec2(0,0);
	bool boundary = false;
	inline Halfedge* getHalfedge() {
		return this->halfedge;
	}
public:
	Vertex(ID id);

	Vertex();

	Vertex(const Vertex& v);

	~Vertex();

	inline const Halfedge* getHalfedge() const {
		return this->halfedge;
	}
	inline void setHalfedge(Halfedge* halfedge) {
		this->halfedge = halfedge;
	}
	inline void setPosition(glm::vec4 pos) {
		position = pos;
	}
	inline glm::vec4 getPosition() const {
		return position;
	}

	inline glm::vec3 getNormal() const {
		return normal;
	}

	inline void setNormal(glm::vec3 n) {
		normal = n;
	}

	inline void setUV(glm::vec2 uv) {
		this->uv = uv;
	}

	inline glm::vec2 getUV() const {
		return uv;
	}

	inline bool isBoundary() const {
		return boundary;
	}
	friend class MeshInteriorOperator;
	friend class MeshIO;
	friend class MeshDisplay;
	friend class MeshRecorder;
};

class Halfedge : public Component {
protected:
	Vertex* source = nullptr;
	Vertex* target = nullptr;
	Halfedge* next = nullptr;
	Halfedge* prev = nullptr;
	Halfedge* sym = nullptr;

	Face* face;
	inline Vertex* getSource() {
		return this->source;
	}
	inline Vertex* getTarget() {
		return this->target;
	}
	inline Halfedge* getNext() {
		return next;
	}
	inline Halfedge* getPrev() {
		return prev;
	}
	inline Halfedge* getSym() {
		return sym;
	}
	inline Face* getFace() {
		return face;
	}
public:
	Halfedge(ID id);
	Halfedge(const Halfedge& he);

	Halfedge();
	~Halfedge();

	inline void setSource(Vertex* v) {
		source = v;
	}
	inline void setTarget(Vertex* v) {
		target = v;
	}
	inline void setSym(Halfedge* he) {
		sym = he;
		he->sym = this;
	}
	inline void setNext(Halfedge* he) {
		next = he;
		he->prev = this;
	}
	inline void setPrev(Halfedge* he) {
		prev = he;
		he->next = this;
	}
	inline void setFace(Face* f) {
		face = f;
	}
	inline const Vertex* getSource() const {
		return this->source;
	}
	inline const Vertex* getTarget() const {
		return this->target;
	}
	inline const Halfedge* getNext() const {
		return next;
	}
	inline const Halfedge* getPrev() const {
		return prev;
	}
	inline const Halfedge* getSym() const {
		return sym;
	}
	inline Face* getFace() const {
		return face;
	}

	inline bool isBoundary() const {
		return face == nullptr;
	}
	inline float getLength() const {
		return glm::length(target->getPosition() - source->getPosition());
	}
	friend class MeshInteriorOperator;
	friend class MeshIO;
	friend class MeshDisplay;
	friend class MeshRecorder;
};

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

class Face : public Component {
protected:
	Halfedge* halfedge = nullptr; 
	inline Halfedge* getHalfedge() {
		return this->halfedge;
	}
public:
	Face(ID id);

	Face();

	Face(const Face& face);

	~Face();
	inline const Halfedge* getHalfedge() const {
		return this->halfedge;
	}
	
	inline void setHalfedge(Halfedge* he) {
		halfedge = he;
	}
	friend class MeshInteriorOperator;
	friend class MeshIO;
	friend class MeshDisplay;
	friend class MeshRecorder;
};


struct pairhash {
public:
	template <typename T, typename U>
	std::size_t operator()(const std::pair<T, U>& x) const
	{
		return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
	}
};

class Mesh {
	struct SourceTargetID {
		ID sourceID;
		ID targetID;
	};
	std::unordered_map<ID, Vertex> vertices;
	std::unordered_map<ID, Halfedge> halfedges;
	std::unordered_map<ID, Face> faces;
	std::unordered_map<std::pair<ID,ID>, Halfedge*, pairhash> vertexHalfedge;
	ID vIdSum = 0;
	ID heIdSum = 0;
	ID fIdSum = 0;

	Vertex* createVertex();

	Halfedge* createHalfedge(Vertex* source, Vertex* target);

	Face* createFace();

	inline Halfedge* getHalfedge(Vertex* source, Vertex* target) {
		return vertexHalfedge[{source->getId(), target->getId()}];
	}

public:
	Mesh();

	inline const std::unordered_map<ID, Vertex>& getVertices() {
		return vertices;
	}

	inline const std::unordered_map<ID, Halfedge>& getHalfedges() {
		return halfedges;
	}

	inline const std::unordered_map<ID, Face>& getFaces() {
		return faces;
	}

	inline const Halfedge* getHalfedge(const Vertex* source, const Vertex* target) {
		return vertexHalfedge[{source->getId(), target->getId()}];
	}

	inline Vertex* vertexAt(ID id) {
		return &vertices[id];
	}

	inline Halfedge* halfedgeAt(ID id) {
		return &halfedges[id];
	}

	inline Face* faceAt(ID id) {
		return &faces[id];
	}
	friend class MeshInteriorOperator;
	friend class MeshIO;
	friend class MeshDisplay;
	friend class MeshRecorder;
};


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



class MeshRecorder {
	Mesh* mesh;
public:
	MeshRecorder(Mesh* mesh);
};