#ifndef MESH_ATTRIBUTE
#define MESH_ATTRIBUTE
#include "mesh.h"
#include <unordered_map>

class BaseMeshAttribute {
	Mesh* mesh;
public:
	enum AttributeType {
		VertexAttribute,
		HalfedgeAttribute,
		FaceAttribute
	};
	AttributeType type;
	BaseMeshAttribute(Mesh* mesh, AttributeType type);
	virtual ~BaseMeshAttribute();
	virtual void insertAttribute(ID id) = 0;
	virtual void removeAttribute(ID id) = 0;
};

template<typename T>
class MeshAttribute : public BaseMeshAttribute {
	std::unordered_map<ID, T> attribMap;
public:
	MeshAttribute(Mesh* mesh, AttributeType type) : BaseMeshAttribute(mesh, type){
	}
	void insertAttribute(ID id, T t) {
		attribMap[id] = t;
	}
	void insertAttribute(ID id) override {
		attribMap[id] = T();
	}
	void removeAttribute(ID id) override {
		attribMap.erase(id);
	}
	T&& operator[](ID id) const {
		return attribMap.at(id);
	}
	T&& operator()(ID id) const {
		return attribMap.at(id);
	}
	T& operator[](ID id) {
		return attribMap.at(id);
	}
	T& operator()(ID id) {
		return attribMap.at(id);
	}

	T&& operator[](const Component* comp) const {
		return attribMap.at(comp.getId());
	}
	T&& operator()(const Component* comp) const {
		return attribMap.at(comp.getId());
	}
	T& operator[](Component* comp) {
		return attribMap.at(comp.getId());
	}
	T& operator()(Component* comp) {
		return attribMap.at(comp.getId());
	}
};

#endif