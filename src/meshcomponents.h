#include <memory>
#include <vector>

class Vertex;
class Halfedge;
class Face;
class Mesh;

typedef std::pair<Halfedge*, Halfedge*> Edge;

enum ComponentType {
	VERTEX, HALFEDGE, FACE
};

class Component {
	int id;
public:
	Component(int id) : id(id) {
	}

	~Component()
	{

	}

	inline void setId(int id) {
		this->id = id;
	}

	inline int getId() const {
		return this->id;
	}
};

class Vertex : public Component {
	Halfedge* halfedge;
public:
	Vertex(int id) : Component(id) {
		halfedge = nullptr;
	}

	~Vertex()
	{

	}

	inline Halfedge* getHalfedge() const {
		return this->halfedge;
	}
	inline void setHalfedge(Halfedge* halfedge) {
		this->halfedge = halfedge;
	}
};

class Halfedge : public Component {
	Vertex* source;
	Vertex* target;
	Halfedge* next;
	Halfedge* prev;
	Halfedge* sym;
public:
	Halfedge(int id) : Component(id) {
		source = nullptr;
		target = nullptr;
		next = nullptr;
		prev = nullptr;
		sym = nullptr;
	}
	~Halfedge()
	{

	}
	inline Vertex* getSource() const {
		return this->source;
	}
	inline Vertex* getTarget() const {
		return this->target;
	}
	void setSym(Halfedge* he) {
		sym = he;
		he->sym = this;
	}
	void setNext(Halfedge* he) {
		next = he;
		he->prev = this;
	}
	void setPrev(Halfedge* he) {
		prev = he;
		he->next = this;
	}
};

void setSym(Halfedge* he1, Halfedge* he2) {
	he1->setSym(he2);
}

void setNext(Halfedge* he, Halfedge* heNext) {
	he->setNext(heNext);
}

void setPrev(Halfedge* he, Halfedge* hePrev) {
	he->setPrev(hePrev);
}

class Face : public Component {
	Halfedge* halfedge;
public:
	Face(int id) : Component(id)
	{
		halfedge = nullptr;
	}

	~Face() {

	}
	inline Halfedge* getHalfedge() const {
		return this->halfedge;
	}
	inline void setHalfedge(Halfedge* he) {
		halfedge = he;
	}
};

class Mesh : public Component {
	std::unique_ptr<std::vector<Vertex>> vertices;
	std::unique_ptr<std::vector<Halfedge>> halfedges;
	std::unique_ptr<std::vector<Face>> faces;
	int vIdSum = 0;
	int heIdSum = 0;
	int fIdSum = 0;
public:
	const std::vector<Vertex>& getVertices() {
		return *vertices;
	}

	const std::vector<Halfedge>& getHalfedge() {
		return *halfedges;
	}

	const std::vector<Face>& getFaces() {
		return *faces;
	}

	Vertex* createVertex() {
		vertices->push_back(Vertex(vIdSum++));
		return &vertices->back();
	}

	Halfedge* createHalfedge() {
		halfedges->push_back(Halfedge(heIdSum++));
		return &halfedges->back();
	}

	Face* createFace() {
		faces->push_back(Face(fIdSum++));
		return &faces->back();
	}
};

