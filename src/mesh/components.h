#pragma once
#include <vector>
#include <unordered_map>
#include <glm.hpp>
#include <memory>
#include <algorithm>
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
		* Constructor.
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
		/**
		* @param Component::id
		*/
		void setId(ID id);
		/**
		* @return Component::id
		*/
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
		* Default constructor for STL that requires a default constructor, e.g. std::vector.
		*/
		Vertex();
		/**
		* Destructor.
		*/
		~Vertex();
		/**
		* @return Vertex::halfedge
		*/
		Halfedge* getHalfedge();
		/**
		* @see vertex::getHalfedge();
		*/
		const Halfedge* getHalfedge() const;
		/**
		* @param Vertex::halfedge
		*/
		void setHalfedge(Halfedge* halfedge);
		/**
		* @param the position of the vertex. Position attribute is stored in Mesh.
		* @see Mesh::setVertexPosition
		*/
		void setPosition(glm::vec4 pos);
		/**
		* @return the position of the vertex. Position attribute is stored in Mesh.
		* @see Mesh::getVertexPosition
		*/
		glm::vec4 getPosition() const;
		/**
		* @param the normal of the vertex. Normal attribute is stored in Mesh.
		* @see Mesh::setVertexNormal
		*/
		glm::vec3 getNormal() const;
		/**
		* @return the normal of the vertex. Normal attribute is stored in Mesh.
		* @see Mesh::setVertexNormal
		*/
		void setNormal(glm::vec3 n);
		/**
		* @return Whether the vertex is on boundary.
		*/
		bool isBoundary() const;
		/**
		* @return the non-const pointer of the const vertex object using const_cast.
		*/
		Vertex* getMutable() const;
		friend class Halfedge;
		friend class MeshIO;
	};

	class Halfedge : public Component {
	protected:
		/**
		* The source vertex. Must not be null.
		*/
		Vertex* source = nullptr;
		/**
		* The target vertex. Must not be null.
		*/
		Vertex* target = nullptr;
		/**
		* Next halfedge. Must not be null. Point to sym iff no other edges connected.
		*/
		Halfedge* next = nullptr;
		/**
		* Next halfedge. Must not be null. Point to sym iff no other edges connected.
		*/
		Halfedge* prev = nullptr;
		/**
		* Symmetric(Twin) halfedge. Must not be null.
		*/
		Halfedge* sym = nullptr;
		/**
		* Face attached to this halfedge. Can be null if this halfedge is on boundary.
		*/
		Face* face = nullptr;
	public:
		/**
		* Constructor.
		*/
		Halfedge(ID id, Mesh* mesh);
		/**
		* Constructor.
		*/
		Halfedge(Mesh* mesh);
		/**
		* Copy constructor.
		*/
		Halfedge(const Halfedge& he);
		/**
		* Default constructor for STL that requires a default constructor, e.g. std::vector.
		*/
		Halfedge();
		/**
		* Destructor.
		*/
		~Halfedge();
		/**
		* @return Halfedge::source
		*/
		Vertex* getSource();
		/**
		* @return Halfedge::target
		*/
		Vertex* getTarget();
		/**
		* @return Halfedge::next
		*/
		Halfedge* getNext();
		/**
		* @return Halfedge::prev
		*/
		Halfedge* getPrev();
		/**
		* @return Halfedge::sym
		*/
		Halfedge* getSym();
		/**
		* @return Halfedge::face
		*/
		Face* getFace();
		/**
		* @param Halfedge::source
		*/
		void setSource(Vertex* v);
		/**
		* @param Halfedge::target
		*/
		void setTarget(Vertex* v);
		/**
		* @param Halfedge::sym
		*/
		void setSym(Halfedge* he);
		/**
		* @param Halfedge::next
		*/
		void setNext(Halfedge* he);
		/**
		* @param Halfedge::prev
		*/
		void setPrev(Halfedge* he);
		/**
		* @param Halfedge::face
		*/
		void setFace(Face* f);
		/**
		* @see Halfedge::getSource()
		*/
		const Vertex* getSource() const;
		/**
		* @see Halfedge::getTarget()
		*/
		const Vertex* getTarget() const;
		/**
		* @see Halfedge::getNext()
		*/
		const Halfedge* getNext() const;
		/**
		* @see Halfedge::getPrev()
		*/
		const Halfedge* getPrev() const;
		/**
		* @see Halfedge::getSym()
		*/
		const Halfedge* getSym() const;
		/**
		* @see Halfedge::getFace()
		*/
		const Face* getFace() const;
		/**
		* @return face==nullptr
		*/
		bool isBoundary() const;
		/**
		* @return Euclidean distance
		*/
		float getLength() const;
		/**
		* @return non-const pointer of this
		*/
		Halfedge* getMutable() const;
	};

	class Face : public Component {
	protected:
		/**
		* One halfedge attached to this face. Must not be null.
		*/
		Halfedge* halfedge = nullptr;
	public:
		/**
		* Constructor.
		* @param id
		* @param mesh
		*/
		Face(ID id, Mesh* mesh);
		/**
		* Constructor.
		* @param mesh
		*/
		Face(Mesh* mesh);
		/**
		* Copy constructor.
		*/
		Face(const Face& face);
		/**
		* Default constructor.
		*/
		Face();
		/**
		* Destructor.
		*/
		~Face();
		/**
		* @return Face::halfedge
		*/
		Halfedge* getHalfedge();
		/**
		* @see Face::getHalfedge()
		*/
		const Halfedge* getHalfedge() const;
		/**
		* @param Face::halfedge
		*/
		void setHalfedge(Halfedge* he);
		/**
		* @return The non-const pointer.
		*/
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
	public:
		/**
		* AttributeType enum tells which type of component that this attribute is for
		*/
		enum AttributeType {
			VertexAttribute,
			HalfedgeAttribute,
			FaceAttribute
		};
		/**
		* Type of component that this attribute is for
		*/
		AttributeType type;
		/**
		* @param mesh
		* @param type
		*/
		BaseMeshAttribute(AttributeType type);
		/**
		* Destructor. Remember to call Mesh::remove[Vertex|Halfedge|Face]Attribute before calling this.
		*/
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
		/**
		* Actual data structure for storing attribute.
		*/
		std::unordered_map<ID, T> attribMap;
		T defaultValue;
	public:
		/**
		* Constructor.
		* @param mesh
		* @param type
		* @param default value
		*/
		MeshAttribute(AttributeType type, T defaultValue = T()) : 
			BaseMeshAttribute(type), defaultValue(defaultValue) {
		}
		/**
		* Insert a component by id into attribute and assign it with the default value.
		* @param id The id of the component.
		*/
		void insertAttribute(ID id) {
			attribMap.insert(std::pair<ID, T>(id, defaultValue));
		}
		/**
		* Remove entry from attributes.
		* @param id Component ID that are going to be deleted.
		*/
		void removeAttribute(ID id) override {
			attribMap.erase(id);
		}
		/**
		* Get the value of the const component attribute by pointer.
		* @param id The pointer of the const component.
		* @return value of the component's attribute.
		*/
		const T operator[](const Component* comp) const {
			return attribMap.at(comp->getId());
		}
		/**
		* @see operator[](Component* comp)
		*/
		const T operator()(const Component* comp) const {
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
		/**
		* @see operator[](Component* comp)
		*/
		T& operator()(Component* comp) {
			return attribMap.at(comp->getId());
		}
		/**
		* @param func Lambda expression that map a value (with a type) to another value (with another type)
		* @return The map result.
		*/
		template<typename U, typename Func>
		std::unique_ptr<MeshAttribute<U>> map(Func&& func) {
			std::unique_ptr<MeshAttribute<U>> retAttrib = std::make_unique<MeshAttribute<U>>(this->type);
			for (auto& idAttrib : attribMap) {
				retAttrib->attribMap.insert(std::pair<ID, U>(idAttrib.first, (U)func(idAttrib.second)));
			}
			return std::move(retAttrib);
		}

		template<typename U>
		friend class MeshAttribute;
	};

	/**
	* Data structure used for hashing std::pair<ID, ID>
	*/
	struct pairhash {
	public:
		template <typename T, typename U>
		std::size_t operator()(const std::pair<T, U>& x) const
		{
			return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
		}
	};

	/**
	* Mesh defines the polygon mesh data structure as well as basic operations.
	*/
	class Mesh {
		/**
		* stores vertices.
		*/
		std::unordered_map<ID, Vertex> vertices;
		/**
		* stores halfedges.
		*/
		std::unordered_map<ID, Halfedge> halfedges;
		/**
		* stores faces.
		*/
		std::unordered_map<ID, Face> faces;
		/**
		* A map to halfedges with given two vertices IDs.
		*/
		std::unordered_map<std::pair<ID, ID>, Halfedge*, pairhash> vertexHalfedge;
		/**
		* Self-increasing id for adding vertex.
		*/
		ID vIdSum = 0;
		/**
		* Self-increasing id for adding halfedges.
		*/
		ID heIdSum = 0;
		/**
		* Self-increasing id for adding faces.
		*/
		ID fIdSum = 0;
		/**
		* Topological status check and debugging
		*/
		std::unique_ptr<MeshAssert> meshAssert;
		/**
		* integrityCheck
		*/
		bool integrityCheck = false;
		/**
		* stores vertexAttributes
		*/
		std::vector<BaseMeshAttribute*> vertexAttributes;
		/**
		* stores halfedgeAttributes
		*/
		std::vector<BaseMeshAttribute*> halfedgeAttributes;
		/**
		* stores faceAttributes
		*/
		std::vector<BaseMeshAttribute*> faceAttributes;
		/**
		* stores positionAttrib for vertex
		*/
		std::unique_ptr<MeshAttribute<glm::vec4>> positionAttrib;
		/**
		* stores vertexNormalAttrib for vertex
		*/
		std::unique_ptr<MeshAttribute<glm::vec3>> vertexNormalAttrib;
	public:
		/**
		* Default constructor.
		*/
		Mesh();
		/**
		* Create a vertex with no halfedges attached to it (i.e. isolated vertex).
		* @see Vertex::halfedge
		* @return created vertex (isolated)
		*/
		Vertex* createVertex();
		/**
		* Create a halfedge with specified source and target vertices. 
		* This function won't create sym halfedge, so you should use createEdge instead of this.
		* @see Mesh::createEdge
		* @see Halfedge::source
		* @see Halfedge::target
		* @param source Source vertex
		* @param target Target vertex
		* @return created halfedge (sym==null)
		*/
		Halfedge* createHalfedge(Vertex* source, Vertex* target);
		/**
		* Create a face with a specified halfedge.
		* @param he Boundary halfedge (Halfedge::face==null)
		* @return created Face
		*/
		const Face* createFace(Halfedge* he);
		/**
		* Delete a face and assign its surrounding halfedge's face to be null (i.e. create a boundary).
		* @param face Face to be deleted
		*/
		void deleteFace(Face* face);
		/**
		* Delete an edge (i.e. two sym halfedges). Meanwhile maintain source & target's halfedge. 
		* Might yield a isolated vertex (Vertex::halfedge==null) if this edge is the only edge connected to this vertex
		* @param he One halfedge of the edge to be deleted.
		*/
		void deleteEdge(Halfedge* he);
		/**
		* Should only be called when v is isolated.
		* @param v Isolated vertex to be deleted.
		*/
		void deleteVertex(Vertex* v);
		/**
		* Get the boundary halfedge (face==null) of this vertex. 
		* @param v Vertex (not isolated)
		* @return One of the boundary halfedge. If v is connected with two or more boundaries, returns one of them.
		*/
		const Halfedge* getBoundary(const Vertex* v);
		/**
		* Create a pair of halfedges with given vertex. These vertex must have only one boundary halfedge. 
		* You should use Mesh::getHalfedge to fetch the result.
		* @param v1 One of the vertices
		* @param v2 One of the vertices
		*/
		void createEdge(Vertex* v1, Vertex* v2);
		/**
		* @param source Source vertex of the halfedge
		* @param target Target vertex of the halfedge
		* @return The result halfedge on the mesh
		*/
		const Halfedge* getHalfedge(const Vertex* source, const Vertex* target);
		/**
		* @see Mesh::getHalfedge()
		*/
		const Halfedge* getHalfedge(const Vertex* source, const Vertex* target) const;
		/**
		* Useful for traversing the vertices in mesh.
		* @return const reference of Mesh::vertices
		*/
		const std::unordered_map<ID, Vertex>& getVertices() const;
		/**
		* Useful for traversing the halfedges in mesh.
		* @return const reference of Mesh::halfedges
		*/
		const std::unordered_map<ID, Halfedge>& getHalfedges() const;
		/**
		* Useful for traversing the faces in mesh.
		* @return const reference of Mesh::faces
		*/
		const std::unordered_map<ID, Face>& getFaces() const;
		/**
		* @param id ID
		* @return Vertex with this id.
		*/
		const Vertex* vertexAt(ID id);
		/**
		* @param id ID
		* @return Halfedge with this id.
		*/
		const Halfedge* halfedgeAt(ID id);
		/**
		* @param id ID
		* @return Face with this id.
		*/
		const Face* faceAt(ID id);
		void setIntegrityCheck(bool val);
		/**
		* @return Mesh::heIdSum
		*/
		ID getHalfedgeIdTotal();
		/**
		* @return Mesh::fIdSum
		*/
		ID getFaceIdTotal();
		/**
		* @return Mesh::vIdSum
		*/
		ID getVertexIdTotal();
		/**
		* Clear vertices, halfedge, faces storage.
		*/
		void deleteMesh();
		/**
		* @param v Vertex
		* @return Position of Vertex
		*/
		glm::vec4 getVertexPosition(const Vertex* v) const;
		/**
		* @param v Vertex
		* @param pos Position to be set
		*/
		void setVertexPosition(Vertex* v, glm::vec4 pos);
		/**
		* @param v Vertex
		* @return Normal of Vertex
		*/
		glm::vec3 getVertexNormal(const Vertex* v) const;
		/**
		* @param v Vertex
		* @param normal Normal to be set
		*/
		void setVertexNormal(Vertex* v, glm::vec3 normal);
		/**
		* Create a vertex MeshAttribute and attach it to the mesh
		* @param defaultVal Default Value given to this attribute
		* @return unique_ptr of the MeshAttribute
		*/
		template<typename T>
		std::unique_ptr<MeshAttribute<T>> createVertexAttribute(T defaultValue = T());
		/**
		* Create a Halfedge MeshAttribute and attach it to the mesh
		* @param defaultVal Default Value given to this attribute
		* @return unique_ptr of the MeshAttribute
		*/
		template<typename T>
		std::unique_ptr<MeshAttribute<T>> createHalfedgeAttribute(T defaultValue = T());
		/**
		* Create a Face MeshAttribute and attach it to the mesh
		* @param defaultVal Default Value given to this attribute
		* @return unique_ptr of the MeshAttribute
		*/
		template<typename T>
		std::unique_ptr<MeshAttribute<T>> createFaceAttribute(T defaultValue = T());
		/**
		* Remove a vertex MeshAttribute from the mesh.
		* @param attrib MeshAttribute to be removed
		*/
		template<typename T>
		void removeVertexAttribute(MeshAttribute<T>* attrib);
		/**
		* Remove a halfedge MeshAttribute from the mesh.
		* @param attrib MeshAttribute to be removed
		*/
		template<typename T>
		void removeHalfedgeAttribute(MeshAttribute<T>* attrib);
		/**
		* Remove a face MeshAttribute from the mesh.
		* @param attrib MeshAttribute to be removed
		*/
		template<typename T>
		void removeFaceAttribute(MeshAttribute<T>* attrib);
		friend class MeshInteriorOperator;
		friend class MeshIO;
		friend class MeshDisplay;
		friend class MeshRecorder;
	};


	template<typename T>
	std::unique_ptr<MeshAttribute<T>> Mesh::createVertexAttribute(T defaultValue) {
		std::unique_ptr<MeshAttribute<T>> attrib = std::make_unique<MeshAttribute<T>>(
			BaseMeshAttribute::VertexAttribute, defaultValue
		);
		vertexAttributes.push_back(attrib.get());
		for (auto& idVertex : getVertices()) {
			attrib->insertAttribute(idVertex.first);
		}
		return std::move(attrib);
	}

	template<typename T>
	std::unique_ptr<MeshAttribute<T>> Mesh::createHalfedgeAttribute(T defaultValue) {
		std::unique_ptr<MeshAttribute<T>> attrib = std::make_unique<MeshAttribute<T>>(
			BaseMeshAttribute::HalfedgeAttribute, defaultValue
		);
		halfedgeAttributes.push_back(attrib.get());
		for (auto& idHe : getHalfedges()) {
			attrib->insertAttribute(idHe.first);
		}
		return std::move(attrib);
	}

	template<typename T>
	std::unique_ptr<MeshAttribute<T>> Mesh::createFaceAttribute(T defaultValue) {
		std::unique_ptr<MeshAttribute<T>> attrib = std::make_unique<MeshAttribute<T>>(
			BaseMeshAttribute::FaceAttribute, defaultValue
		);
		faceAttributes.push_back(attrib.get());
		for (auto& idFace : getFaces()) {
			attrib->insertAttribute(idFace.first);
		}
		return std::move(attrib);
	}

	template<typename T>
	void Mesh::removeVertexAttribute(MeshAttribute<T>* attrib) {
		auto it = std::find(vertexAttributes.begin(), vertexAttributes.end(), attrib);
		vertexAttributes.erase(it);
	}

	template<typename T>
	void Mesh::removeHalfedgeAttribute(MeshAttribute<T>* attrib) {
		auto it = std::find(halfedgeAttributes.begin(), halfedgeAttributes.end(), attrib);
		halfedgeAttributes.erase(it);
	}

	template<typename T>
	void Mesh::removeFaceAttribute(MeshAttribute<T>* attrib) {
		auto it = std::find(faceAttributes.begin(), faceAttributes.end(), attrib);
		faceAttributes.erase(it);
	}
}