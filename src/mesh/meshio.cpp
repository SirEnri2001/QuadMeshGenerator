#include "meshio.h"
#include <tiny_obj_loader.h>
#include <fstream>
#include "misc/strutil.h"

#define MAX_LINE 2048

MeshIO::MeshIO(Mesh* mesh) :mesh(mesh) {

}

MeshIO::~MeshIO() {

}

bool MeshIO::loadObj(const std::string& filename) {
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
		Halfedge* he = (&idHe.second)->getMutable();
		if (!he->isBoundary()) {
			continue;
		}
		Halfedge* he1 = he->getSym();
		while (!he1->isBoundary()) {
			he1 = he1->getNext()->getSym();
		}
		he->setPrev(he1);
		he->getTarget()->boundary = true;
		he->getSource()->boundary = true;
	}
	return true;
}

bool MeshIO::loadM(const std::string& filename) {
	std::fstream is(filename, std::fstream::in);
	std::unordered_map<int, Vertex*> fileIdVertex;

	if (is.fail())
	{
		fprintf(stderr, "Error in opening file %s\n", filename);
		return false;
	}

	char buffer[MAX_LINE];
	int id;

	while (is.getline(buffer, MAX_LINE))
	{

		std::string line(buffer);
		line = strutil::trim(line);

		strutil::Tokenizer stokenizer(line, " \r\n");

		stokenizer.nextToken();
		std::string token = stokenizer.getToken();

		if (token == "Vertex")
		{
			stokenizer.nextToken();
			token = stokenizer.getToken();
			id = strutil::parseString<int>(token);

			glm::vec4 p;
			for (int i = 0; i < 3; i++)
			{
				stokenizer.nextToken();
				token = stokenizer.getToken();
				p[i] = strutil::parseString<float>(token);
			}
			p[3] = 1.0f;

			Vertex* v = mesh->createVertex();
			v->setPosition(p);
			fileIdVertex[id] = v;
			if (!stokenizer.nextToken("\t\r\n")) continue;
			token = stokenizer.getToken();

			int sp = (int)token.find("{");
			int ep = (int)token.find("}");

			//if (sp >= 0 && ep >= 0)
			//{
			//	v->string() = token.substr(sp + 1, ep - sp - 1);
			//}
			continue;
		}


		if (token == "Face")
		{
			stokenizer.nextToken();
			token = stokenizer.getToken();
			id = strutil::parseString<int>(token);

			std::vector<Vertex*> v;
			while (stokenizer.nextToken())
			{
				token = stokenizer.getToken();
				if (strutil::startsWith(token, "{")) break;
				int vid = strutil::parseString<int>(token);
				v.push_back(fileIdVertex[vid]);
			}

			Face* face = mesh->createFace();
			Halfedge* he = nullptr, * hePrev = nullptr;
			for (int i = 0; i < v.size(); i++) {
				Vertex* v1 = v[i];
				Vertex* v2 = v[(i + 1) % v.size()];
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
			//if (strutil::startsWith(token, "{"))
			//{
			//	f->string() = strutil::trim(token, "{}");
			//}
			continue;
		}
	}
	// init sym halfedges
	for (auto& idHe : mesh->getHalfedges()) {
		Halfedge* he = idHe.second.getMutable();
		if (he->getSym() == nullptr) {
			he->setSym(mesh->createHalfedge(he->getTarget(), he->getSource()));
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
		he->getTarget()->boundary = true;
		he->getSource()->boundary = true;
	}
	return true;
}

bool MeshIO::writeObj(const std::string& filename) {
	const char* output = filename.c_str();
	std::fstream _os(output, std::fstream::out);
	if (_os.fail())
	{
		fprintf(stderr, "Error is opening file %s\n", output);
		return false;
	}

	int vid = 1;
	for (int id = 0; id < mesh->getVertices().size(); id++) {
		_os << "v";
		for (int i = 0; i < 3; i++)
		{
			_os << " " << mesh->getVertices().at(id).getPosition()[i];
		}
		_os << std::endl;
	}
	//for (int id = 0; id < mesh->getVertices().size(); id++) {
	//	_os << "vt";
	//	for (int i = 0; i < 2; i++)
	//	{
	//		_os << " " << mesh->getVertices().at(i).getPosition()[i];
	//	}
	//	_os << std::endl;
	//}
	//for (int id = 0; id < mesh->getVertices().size(); id++) {
	//	_os << "vn";
	//	for (int i = 0; i < 3; i++)
	//	{
	//		_os << " " << mesh->getVertices().at(i).getPosition()[i];
	//	}
	//	_os << std::endl;
	//}
	for (auto& idFace : mesh->getFaces()) {
		const Face* face = &idFace.second;
		_os << "f";

		const Halfedge* he = face->getHalfedge();

		do {
			int vid = he->getTarget()->getId();
			_os << " " << vid+1 << "/" << vid+1 << "/" << vid+1;
		} while (he = he->getNext(), he != face->getHalfedge());
		_os << std::endl;
	}
	_os.close();
	return true;
}