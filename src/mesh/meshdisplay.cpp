#include "meshdisplay.h"



glm::vec3 MeshDisplay::calculateSurfaceNormal(const Face* face) {
	glm::vec3 result(0, 0, 0);
	const Halfedge* he = face->getHalfedge();
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

glm::vec3 MeshDisplay::calculateVertexNormal(const Vertex* vertex) {
	glm::vec3 result(0, 0, 0);
	const Halfedge* he = vertex->getHalfedge();
	if (he == nullptr) {
		return glm::vec3(0, 1, 0);
	}
	do {
		if(he->getFace())
			result += calculateSurfaceNormal(he->getFace());
	} while (he = he->getNext()->getSym(), he != vertex->getHalfedge());
	return glm::normalize(result);
}

MeshDisplay::MeshDisplay(Mesh* mesh) :mesh(mesh) {

}
void MeshDisplay::create() {
	using namespace glm;
	indices.clear();
	vertexBuffer.clear();
	vertexBufferOffset.clear();
	std::vector<const Vertex*> vertsUntriangulated;
	int ind = 0;
	int faceId = 0;
	for (int faceId = 0; faceId < mesh->getFaceIdTotal(); faceId++) {
		int index = faceId;
		auto& idFace = mesh->getFaces().find(faceId);
		if (idFace == mesh->getFaces().cend()) {
			continue;
		}
		const Face* face = &idFace->second;
		glm::vec3 normal = calculateSurfaceNormal(face);
		glm::vec4 normal4 = glm::vec4(normal, 1.0);

		vertsUntriangulated.clear();
		const Halfedge* he = face->getHalfedge();
		do {
			vertsUntriangulated.push_back(he->getTarget());
		} while (he = he->getNext(), he != face->getHalfedge());
		if (vertsUntriangulated.size() == 3) {
			vertexBufferOffset[face->getId()] = ind;
			indices.push_back(ind++);
			indices.push_back(ind++);
			indices.push_back(ind++);
			vertexBuffer.push_back(vertsUntriangulated[0]->getPosition());
			vertexBuffer.push_back(normal4);
			vertexBuffer.push_back(vec4(0.7, 0.7, 0.6, 1.0));
			vertexBuffer.push_back(vertsUntriangulated[1]->getPosition());
			vertexBuffer.push_back(normal4);
			vertexBuffer.push_back(vec4(0.7, 0.7, 0.6, 1.0));
			vertexBuffer.push_back(vertsUntriangulated[2]->getPosition());
			vertexBuffer.push_back(normal4);
			vertexBuffer.push_back(vec4(0.7, 0.7, 0.6, 1.0));
			continue;
		}

		// trianglualte
		// find the most left convex point
		he = face->getHalfedge();
		const Vertex* currentConvex = nullptr;
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
			const Halfedge* he = face->getHalfedge();
			int startInd = 1;
			vertexBufferOffset[face->getId()] = ind;
			do {
				vertexBuffer.push_back(he->getTarget()->getPosition());
				vertexBuffer.push_back(normal4);
				vertexBuffer.push_back(vec4(0.7, 0.7, 0.6, 1.0));
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

void MeshDisplay::createFrame() {
	frameIndices.clear();
	frameVertexBuffer.clear();
	pointScatter.clear();
	pointIndices.clear();
	for (int i = 0; i < mesh->getHalfedgeIdTotal(); i++) {
		auto& idHe = mesh->getHalfedges().find(i);
		if (idHe == mesh->getHalfedges().cend()) {
			frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 0));
			frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 0));
			frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 0));
			frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 0));
			frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 0));
			frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 0));
			continue;
		}
		const Halfedge* he = &mesh->getHalfedges().at(i);
		frameVertexBuffer.push_back(he->getSource()->getPosition());
		frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 1));
		frameVertexBuffer.push_back(glm::vec4(calculateVertexNormal(he->getSource()), 0));
		frameVertexBuffer.push_back(he->getTarget()->getPosition());
		frameVertexBuffer.push_back(glm::vec4(0, 0, 0, 1));
		frameVertexBuffer.push_back(glm::vec4(calculateVertexNormal(he->getTarget()), 0));
		frameIndices.push_back(2*i);
		frameIndices.push_back(2*i+1);
	}
}

void MeshDisplay::markHalfedge(const Halfedge* he, glm::vec4 color) {
	frameVertexBuffer[he->getId() * 6 + 1] = glm::vec4(1, 0, 0, 1);
	frameVertexBuffer[he->getId() * 6 + 4] = glm::vec4(0, 0, 1, 1);

	markCount++;
}

void MeshDisplay::markFace(const Face* face, glm::vec4 color) {
	int startId = vertexBufferOffset[face->getId()];
	int endId = face->getId() < mesh->getFaceIdTotal() - 1 ?
		vertexBufferOffset[face->getId() + 1] : 
		vertexBuffer.size()/3;
	for (int i = startId; i < endId; i++) {
		vertexBuffer[i * 3 + 2] = color;
	}
}

void MeshDisplay::markBoundaries() {
	for (auto& idHe : mesh->getHalfedges()) {
		if(idHe.second.isBoundary())
			markHalfedge(&idHe.second);
	}
}

void MeshDisplay::markVertex(const Vertex* vertex) {
	pointScatter.push_back(vertex->getPosition());
	pointScatter.push_back(glm::vec4(1, 0, 0, 1));
	pointScatter.push_back(glm::vec4(calculateVertexNormal(vertex),0));
	pointIndices.push_back(pointIndices.size());
}
