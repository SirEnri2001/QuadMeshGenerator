#pragma once
#include <vector>
#include <unordered_map>
#include <glm.hpp>
#include <memory>
#include "core.h"

namespace quadro {
	typedef std::pair<Halfedge*, Halfedge*> Edge;
	/**
	* Component as a base class for mesh components, such as vertices, halfedges and faces.
	*/
	class Component {
	protected:
		/**
		* Mesh pointer that points the owner mesh.
		*/
		Mesh* mesh;
		/**
		* ID distinguishes different components of the same type.
		*/
		ID id;
	public:
		/**
		* Recommended constructor.
		* @param id The id of new component.
		* @param mesh The owner mesh of new component.
		*/
		Component(ID id, Mesh* mesh);
		/**
		* Copy constructor.
		*/
		Component(const Component& comp);
		/**
		* Destructor.
		*/
		virtual ~Component();
		void setId(ID id);
		ID getId() const;
	};

	/**
	* Vertex class is the vertex of the mesh.
	*/
	class Vertex : public Component {
	protected:
		/**
		* One inward halfedge of the vertex. Could be null if the vertex is isolated or just created.
		*/
		Halfedge* halfedge = nullptr;
		/**
		* Cached whether the vertex is on boundary.
		*/
		bool boundary = true;
	public:
		/**
		* Constructor.
		*/
		Vertex(ID id, Mesh* mesh);
		/**
		* Constructor.
		*/
		Vertex(Mesh* mesh);
		/**
		* Copy constructor.
		*/
		Vertex(const Vertex& v);
		/**
		* Copy constructor for STL that requires a default constructor, e.g. std::vector.
		*/
		Vertex();
		/**
		* Destructor.
		*/
		~Vertex();
		/**
		* Get the inward halfedge of the vertex. Could be null if the vertex is isolated or just created.
		*/
		Halfedge* getHalfedge();
		/**
		* Get the inward halfedge of the vertex. Could be null if the vertex is isolated or just created.
		*/
		const Halfedge* getHalfedge() const;
		/**
		* Set the inward halfedge of the vertex. Could be null if the vertex is isolated or just created.
		*/
		void setHalfedge(Halfedge* halfedge);
		/**
		* Set the position of the vertex. Position attribute is stored in Mesh.
		* @see Mesh::setVertexPosition
		*/
		void setPosition(glm::vec4 pos);
		/**
		* Get the position of the vertex. Position attribute is stored in Mesh.
		* @see Mesh::getVertexPosition
		*/
		glm::vec4 getPosition() const;
		/**
		* Set the normal of the vertex. Normal attribute is stored in Mesh.
		* @see Mesh::setVertexNormal
		*/
		glm::vec3 getNormal() const;
		/**
		* Get the normal of the vertex. Normal attribute is stored in Mesh.
		* @see Mesh::setVertexNormal
		*/
		void setNormal(glm::vec3 n);
		/**
		* Whether the vertex is on boundary.
		*/
		bool isBoundary() const;
		/**
		* Get the non-const pointer of the const vertex object using const_cast.
		*/
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
		Halfedge(ID id, Mesh* mesh);
		Halfedge(Mesh* mesh);
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

	class Face : public Component {
	protected:
		Halfedge* halfedge = nullptr;
	public:
		Face(ID id, Mesh* mesh);
		Face(Mesh* mesh);
		Face(const Face& face);
		Face();
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


	/**
	* BaseMeshAttribute as a base class used in Mesh for iterate all attributes.
	*/
	class BaseMeshAttribute {
		Mesh* mesh;
	public:
		/**
		* AttributeType enum tells which type of component that this attribute is for
		*/
		enum AttributeType {
			VertexAttribute,
			HalfedgeAttribute,
			FaceAttribute
		};
		AttributeType type;
		BaseMeshAttribute(Mesh* mesh, AttributeType type);
		virtual ~BaseMeshAttribute();
		/**
		* Insert a component by id into attribute and assign it with the default value.
		* @param id The id of the component.
		*/
		virtual void insertAttribute(ID id) = 0;
		/**
		* Remove a component from attribute by id.
		* @param id The id of the component.
		*/
		virtual void removeAttribute(ID id) = 0;
	};

	/**
	* MeshAttribute as a base class used in all places that stores attributes on vertices, halfedges and faces. MeshAttribute should be created by 
	* the template function Mesh::create[Vertex|Halfedge|Face]Attribute. When the unique_ptr gets destructed, Mesh::remove[Vertex|Halfedge|Face]Attribute
	*  must be called to unregister the attribute.
	* @see Mesh::createVertexAttribute
	* @see Mesh::createHalfedgeAttribute
	* @see Mesh::createFaceAttribute
	* @see Mesh::removeVertexAttribute
	* @see Mesh::removeHalfedgeAttribute
	* @see Mesh::removeFaceAttribute
	*/
	template<typename T>
	class MeshAttribute : public BaseMeshAttribute {
		std::unordered_map<ID, T> attribMap;
	public:
		MeshAttribute(Mesh* mesh, AttributeType type) : BaseMeshAttribute(mesh, type) {
		}
		void removeAttribute(ID id) override {
			attribMap.erase(id);
		}
		/**
		* Get the value of the const component attribute by pointer.
		* @param id The pointer of the const component.
		* @return value of the component's attribute.
		*/
		T operator[](const Component* comp) const {
			return attribMap.at(comp->getId());
		}
		T operator()(const Component* comp) const {
			return attribMap.at(comp->getId());
		}
		/**
		* Get or set the value of the component attribute by pointer.
		* @param id The pointer of the component.
		* @return lvalue reference of the component's attribute.
		*/
		T& operator[](Component* comp) {
			return attribMap.at(comp->getId());
		}
		T& operator()(Component* comp) {
			return attribMap.at(comp->getId());
		}
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
		std::unordered_map<std::pair<ID, ID>, Halfedge*, pairhash> vertexHalfedge;
		ID vIdSum = 0;
		ID heIdSum = 0;
		ID fIdSum = 0;
		std::unique_ptr<MeshAssert> meshAssert;
		bool integrityCheck = false;

		std::vector<BaseMeshAttribute*> vertexAttributes;
		std::vector<BaseMeshAttribute*> halfedgeAttributes;
		std::vector<BaseMeshAttribute*> faceAttributes;
		std::unique_ptr<MeshAttribute<glm::vec4>> positionAttrib;
		std::unique_ptr<MeshAttribute<glm::vec3>> vertexNormalAttrib;
	public:
		Mesh();
		Vertex* createVertex();
		Halfedge* createHalfedge(Vertex* source, Vertex* target);
		const Face* createFace(Halfedge* he);
		void deleteFace(Face* face);
		void deleteEdge(Halfedge* he);
		void deleteVertex(Vertex* v);
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
		glm::vec4 getVertexPosition(const Vertex* v) const;
		void setVertexPosition(Vertex* v, glm::vec4 pos);
		glm::vec3 getVertexNormal(const Vertex* v) const;
		void setVertexNormal(Vertex* v, glm::vec3 normal);
		template<typename T>
		std::unique_ptr<MeshAttribute<T>> createVertexAttribute();
		template<typename T>
		std::unique_ptr<MeshAttribute<T>> createHalfedgeAttribute();
		template<typename T>
		std::unique_ptr<MeshAttribute<T>> createFaceAttribute();
		template<typename T>
		void removeVertexAttribute(MeshAttribute<T>* attrib);
		template<typename T>
		void removeHalfedgeAttribute(MeshAttribute<T>* attrib);
		template<typename T>
		void removeFaceAttribute(MeshAttribute<T>* attrib);
		friend class MeshInteriorOperator;
		friend class MeshIO;
		friend class MeshDisplay;
		friend class MeshRecorder;
	};

}