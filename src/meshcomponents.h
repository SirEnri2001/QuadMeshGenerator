#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <glm.hpp>
class Vertex;
class Halfedge;
class Face;
class Mesh;

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
public:
	Vertex(ID id);

	Vertex();

	Vertex(const Vertex& v);

	~Vertex();

	inline Halfedge* getHalfedge() const {
		return this->halfedge;
	}
	inline void setHalfedge(Halfedge* halfedge) {
		this->halfedge = halfedge;
	}
	inline void setPosition(glm::vec4 pos) {
		position = pos;
	}
	inline glm::vec4 getPosition() {
		return position;
	}

	inline glm::vec3 getNormal() {
		return normal;
	}

	inline void setNormal(glm::vec3 n) {
		normal = n;
	}

	inline void setUV(glm::vec2 uv) {
		this->uv = uv;
	}

	inline glm::vec2 getUV() {
		return uv;
	}
};

class Halfedge : public Component {
protected:
	Vertex* source=nullptr;
	Vertex* target=nullptr;
	Halfedge* next=nullptr;
	Halfedge* prev=nullptr;
	Halfedge* sym = nullptr;

	Face* face;
public:
	Halfedge(ID id);
	Halfedge(const Halfedge& he);

	Halfedge();
	~Halfedge();
	inline Vertex* getSource() const {
		return this->source;
	}
	inline Vertex* getTarget() const {
		return this->target;
	}
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
	inline void setFace(Face* f) {
		face = f;
	}
	inline bool isBoundary() {
		return face == nullptr;
	}
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
public:
	Face(ID id);

	Face();

	Face(const Face& face);

	~Face();
	inline Halfedge* getHalfedge() const {
		return this->halfedge;
	}
	inline void setHalfedge(Halfedge* he) {
		halfedge = he;
	}
};

class MeshOperator;

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

	inline Halfedge* getHalfedge(Vertex* source, Vertex* target) {
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
	friend class MeshOperator;
};

class MeshDisplay {
	Mesh* mesh;
public:
	std::vector<glm::vec4> vertexBuffer;
	std::vector<ID> indices;

	std::vector<glm::vec4> lineSegment;
	std::vector<glm::vec4> pointScatter;

	glm::vec3 calculateSurfaceNormal(const Face* face);

	MeshDisplay(Mesh* mesh);
	void create();
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

class MeshOperator {
	Mesh* mesh;
public:
	MeshOperator(Mesh* mesh);

	bool loadObj(const std::string& filename);
};

class MeshRecorder {
	Mesh* mesh;
public:
	MeshRecorder(Mesh* mesh);
};