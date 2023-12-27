#include "meshdisplay.h"



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