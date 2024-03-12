#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <glm.hpp>
#include <memory>
#include "meshassert.h"

class Vertex;
class Halfedge;
class Face;
class Mesh;
class MeshOperator;
class MeshDisplay;
class MeshIO;
class MeshRecorder;
class MeshAttribute;
class MeshNormal;
class MeshAssert;

typedef std::pair<Halfedge*, Halfedge*> Edge;
typedef unsigned int ID;

class Component {
protected:
	ID id;
	bool valid = true;
	bool displayed = false;
	bool deleted = false;
public:
	Component(ID id);
	Component(const Component& comp);
	~Component();
	void setId(ID id);
	ID getId() const;
	bool isValid() const;
	bool isDisplayed() const;
	void setDeleted(bool val);
};

class Vertex : public Component {
protected:
	Halfedge* halfedge = nullptr;
	glm::vec4 position = glm::vec4(0,0,0,1);
	glm::vec3 normal = glm::vec3(0,0,0);
	glm::vec4 color = glm::vec4(1, 0, 1, 1);
	glm::vec2 uv = glm::vec2(0,0);
	bool boundary = true;
public:

	Vertex(ID id);

	Vertex();

	Vertex(const Vertex& v);

	~Vertex();
	Halfedge* getHalfedge();
	const Halfedge* getHalfedge() const;
	void setHalfedge(Halfedge* halfedge);
	void setPosition(glm::vec4 pos);
	glm::vec4 getPosition() const;
	glm::vec3 getNormal() const;
	glm::vec4 getColor() const;
	void setColor(glm::vec4 col);
	void setNormal(glm::vec3 n);
	void setUV(glm::vec2 uv);
	glm::vec2 getUV() const;
	bool isBoundary() const;
	Vertex* getMutable() const;
	friend class Halfedge;
	friend class MeshIO;
};

class Halfedge : public Component {
protected:
	Vertex* source = nullptr;
	Vertex* target = nullptr;
	Halfedge* next = nullptr;
	Halfedge* prev = nullptr;
	Halfedge* sym = nullptr;
	Face* face = nullptr;
public:
	Halfedge(ID id);
	Halfedge(const Halfedge& he);
	Halfedge();
	~Halfedge();
	Vertex* getSource();
	Vertex* getTarget();
	Halfedge* getNext();
	Halfedge* getPrev();
	Halfedge* getSym();
	Face* getFace();
	void setSource(Vertex* v);
	void setTarget(Vertex* v);
	void setSym(Halfedge* he);
	void setNext(Halfedge* he);
	void setPrev(Halfedge* he);
	void setFace(Face* f);
	const Vertex* getSource() const;
	const Vertex* getTarget() const;
	const Halfedge* getNext() const;
	const Halfedge* getPrev() const;
	const Halfedge* getSym() const;
	const Face* getFace() const;
	bool isBoundary() const;
	float getLength() const;
	Halfedge* getMutable() const;
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
	const Halfedge* getHalfedge() const;
	Halfedge* getHalfedge();
	void setHalfedge(Halfedge* he);
	Face* getMutable() const;
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
	std::unordered_map<ID, Vertex> vertices;
	std::unordered_map<ID, Halfedge> halfedges;
	std::unordered_map<ID, Face> faces;
	std::unordered_map<std::pair<ID,ID>, Halfedge*, pairhash> vertexHalfedge;
	ID vIdSum = 0;
	ID heIdSum = 0;
	ID fIdSum = 0;
	std::unique_ptr<MeshAssert> meshAssert;
	bool integrityCheck = false;
public:
	Mesh();
	Vertex* createVertex();
	Halfedge* createHalfedge(Vertex* source, Vertex* target);
	const Face* createFace(Halfedge* he);
	void deleteFace(Face* face);
	void deleteEdge(Halfedge* he);
	inline void deleteVertex(Vertex* v) {
		vertices.erase(vertices.find(v->getId()));
	}
	const Halfedge* getBoundary(const Vertex* v);
	void createEdge(Vertex* v1, Vertex* v2);
	const Halfedge* getHalfedge(const Vertex* source, const Vertex* target) const;
	const Halfedge* getHalfedge(const Vertex* source, const Vertex* target);
	const std::unordered_map<ID, Vertex>& getVertices() const;
	const std::unordered_map<ID, Halfedge>& getHalfedges() const;
	const std::unordered_map<ID, Face>& getFaces() const;
	const Vertex* vertexAt(ID id);
	const Halfedge* halfedgeAt(ID id);
	const Face* faceAt(ID id);
	void setIntegrityCheck(bool val);
	ID getHalfedgeIdTotal();
	ID getFaceIdTotal();
	ID getVertexIdTotal();
	void deleteMesh();
	//std::unique_ptr<MeshNormal> attribNormal;
	//std::unique_ptr<MeshUV> attribUV;
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