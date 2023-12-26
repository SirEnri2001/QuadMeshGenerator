#include "meshcomponents.h"
#include <memory>
#include <tiny_obj_loader.h>

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
	return &halfedges[id];
}

Face* Mesh::createFace() {
	int id = fIdSum++;
	faces[id] = Face(id);
	return &faces[id];
}
Mesh::Mesh() {

}


glm::vec3 MeshDisplay::calculateSurfaceNormal(const Face* face) {
	glm::vec3 result(0, 0, 0);
	Halfedge* he = face->getHalfedge();
	do {
		result[0] +=
			(he->getSource()->getPosition()[1] - he->getTarget()->getPosition()[1])
			* (he->getSource()->getPosition()[2] + he->getTarget()->getPosition()[2]);
		result[1] +=
			(he->getSource()->getPosition()[2] - he->getTarget()->getPosition()[2])
			* (he->getSource()->getPosition()[0] + he->getTarget()->getPosition()[0]);
		result[2] +=
			(he->getSource()->getPosition()[0] - he->getTarget()->getPosition()[0])
			* (he->getSource()->getPosition()[1] + he->getTarget()->getPosition()[1]);
	} while (he = he->getNext(), he != face->getHalfedge());
	return glm::normalize(result);
}

MeshDisplay::MeshDisplay(Mesh* mesh) :mesh(mesh) {

}
void MeshDisplay::create() {
	using namespace glm;
	std::vector<Vertex*> vertsUntriangulated;
	int ind = 0;
	for (auto& faceIndex : mesh->getFaces()) {
		int index = faceIndex.first;
		const Face* face = &faceIndex.second;
		glm::vec3 normal = calculateSurfaceNormal(face);
		glm::vec4 normal4 = glm::vec4(normal, 1.0);

		vertsUntriangulated.clear();
		Halfedge* he = face->getHalfedge();
		do {
			vertsUntriangulated.push_back(he->getTarget());
		} while (he = he->getNext(), he != face->getHalfedge());
		if (vertsUntriangulated.size() == 3) {
			indices.push_back(ind++);
			indices.push_back(ind++);
			indices.push_back(ind++);
			vertexBuffer.push_back(vertsUntriangulated[0]->getPosition());
			vertexBuffer.push_back(normal4);
			vertexBuffer.push_back(vertsUntriangulated[1]->getPosition());
			vertexBuffer.push_back(normal4);
			vertexBuffer.push_back(vertsUntriangulated[2]->getPosition());
			vertexBuffer.push_back(normal4);
			continue;
		}

		// trianglualte
		// find the most left convex point
		he = face->getHalfedge();
		Vertex* currentConvex = nullptr;
		do {
			//  (v1)-->(v2)
			//        /
			//       he
			//      /
			//     v
			//  (v3)-->(v4)
			vec3 v1 = vec3(he->getPrev()->getSource()->getPosition());
			vec3 v2 = vec3(he->getSource()->getPosition());
			vec3 v3 = vec3(he->getTarget()->getPosition());
			vec3 v4 = vec4(he->getNext()->getTarget()->getPosition());
			if (dot(cross(v3 - v2, v2 - v1), normal) < 0) {
				currentConvex = he->getSource(); //v2
				break;
			}
		} while (he = he->getNext(), he != face->getHalfedge());

		// convex polygon triangulate
		if (true || currentConvex == nullptr) {
			Halfedge* he = face->getHalfedge();
			int startInd = 1;
			do {
				vertexBuffer.push_back(he->getTarget()->getPosition());
				vertexBuffer.push_back(normal4);
				if (he == face->getHalfedge()->getNext() || he == face->getHalfedge()->getPrev()) {
					continue;
				}
				indices.push_back(ind);
				indices.push_back(ind + startInd++);
				indices.push_back(ind + startInd);
			} while (he = he->getNext(), he != face->getHalfedge());
			ind += startInd + 1;
			continue;
		}

		//Vertex* currentVertex;
		//// concave polygon triangulate
		//int i = 0;
		//while (vertsUntriangulated.size() > 2) {
		//	vec3 v0 = vec3(vertsUntriangulated[i]->getPosition());
		//	vec3 v1 = vec3(vertsUntriangulated[(i + 1) % vertsUntriangulated.size()]->getPosition());
		//	vec3 v2 = vec3(vertsUntriangulated[(i + 2) % vertsUntriangulated.size()]->getPosition());
		//	if (abs(cross(vec3(v2 - v1), vec3(v0 - v1)).length()) < FLT_EPSILON) {
		//		vertsUntriangulated.erase(vertsUntriangulated.cbegin() + i + 1); 
		//		i = i % vertsUntriangulated.size();
		//		continue;
		//	}
		//	if (dot(cross(vec3(v2 - v1), vec3(v1 - v0)), normal) < 0) {
		//		i = (i + 1) % vertsUntriangulated.size();
		//		continue;
		//	}
		//	bool isIntersect = false;
		//	for (auto& vert : vertsUntriangulated) {
		//		vec3 v = vert->getPosition();
		//		if (dot(cross(v2 - v1, v - v1), cross(v - v1, v0 - v1))<0 && dot(cross(v1 - v0, v - v0), cross(v - v0, v2 - v0)) < 0) {
		//			isIntersect = true;
		//			break;
		//		}
		//	}
		//	if (isIntersect) {
		//		continue;
		//	}
		//	indices.push_back(ind++);
		//	indices.push_back(ind++);
		//	indices.push_back(ind++);
		//	vertexBuffer.push_back(vec4(v0, 1.0));
		//	vertexBuffer.push_back(vec4(v1, 1.0));
		//	vertexBuffer.push_back(vec4(v2, 1.0));
		//	vertsUntriangulated.erase(vertsUntriangulated.cbegin() + (i + 1) % vertsUntriangulated.size());
		//	i = i % vertsUntriangulated.size();
		//}
	}
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

MeshOperator::MeshOperator(Mesh* mesh) :mesh(mesh)
{

}

bool MeshOperator::loadObj(const std::string& filename) {
	std::cout << "Loading " << filename << std::endl;
	std::string warn;
	std::string err;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;
	// Triangulate by default
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), NULL, false);

	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "ERR: " << err << std::endl;
	}

	if (!ret) {
		std::cerr << "Failed to load obj" << std::endl;
		return false;
	}

	std::cout << "[LoadOBJ] # of shapes in .obj : " << shapes.size() << std::endl;
	std::cout << "[LoadOBJ] # of materials in .obj : " << materials.size() << std::endl;

	bool hasColor = !attrib.colors.empty();
	bool hasNormal = !attrib.normals.empty();

	for (int i = 0; i < attrib.vertices.size() / 3; i++) {
		Vertex* v = mesh->createVertex();
		v->setPosition(glm::vec4(
			attrib.vertices[3 * i + 0],
			attrib.vertices[3 * i + 1],
			attrib.vertices[3 * i + 2],
			1.0f));
	}


	// Iterate all shapes
	for (size_t s = 0; s < 1; ++s)
	{
		int indexSum = 0;
		for (auto& numVerticesPerFace : shapes[s].mesh.num_face_vertices) {
			std::vector<Vertex*> verts;
			Face* face = mesh->createFace();
			Halfedge* he = nullptr, * hePrev = nullptr;
			for (int i = 0; i < numVerticesPerFace; i++) {
				Vertex* v1 = mesh->vertexAt(shapes[s].mesh.indices[indexSum + i].vertex_index);
				Vertex* v2 = mesh->vertexAt(shapes[s].mesh.indices[indexSum + (i + 1) % numVerticesPerFace].vertex_index);
				he = mesh->createHalfedge(v1, v2);
				he->setFace(face);
				if (hePrev) {
					hePrev->setNext(he);
				}
				hePrev = he;
				if (face->getHalfedge() == nullptr) {
					face->setHalfedge(he);
				}
			}
			he->setNext(face->getHalfedge());
			indexSum += numVerticesPerFace;
		}
	}
	Halfedge* bHe = nullptr;
	// init sym halfedges
	for (auto& idHe : mesh->getHalfedges()) {
		Halfedge* he = mesh->halfedgeAt(idHe.first);
		if (he->getSym() == nullptr) {
			he->setSym(mesh->createHalfedge(he->getTarget(), he->getSource()));
			bHe = he->getSym();
			continue;
		}
		he->setSym(mesh->getHalfedge(he->getTarget(), he->getSource()));
	}

	for (auto& idHe : mesh->getHalfedges()) {
		Halfedge* he = mesh->halfedgeAt(idHe.first);
		if (!he->isBoundary()) {
			continue;
		}
		Halfedge* he1 = he->getSym();
		while (!he1->isBoundary()) {
			he1 = he1->getNext()->getSym();
		}
		he->setPrev(he1);
	}
	return true;
}

MeshRecorder::MeshRecorder(Mesh* mesh) :mesh(mesh)
{

}