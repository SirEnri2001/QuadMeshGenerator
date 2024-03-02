#include <Eigen/Dense>
#include "laplacian.h"
#include "../mesh/meshcomponents.h"
#include <map>
#include "../mesh/meshio.h"
#include <glm.hpp>

float desbrunWeight(const Halfedge* halfEdge);


Eigen::MatrixXd laplacian(const Mesh &mesh)
{
	std::map<ID, float> weights;	// keep track of the weight of each edge
	const auto meshVertices = mesh.getVertices();
	int dim = meshVertices.size();


	Eigen::MatrixXd L = Eigen::MatrixXd::Constant(dim, dim, 0);

	// fill in weights wij
	for (int i = 0; i < dim; i++) {
		for (int j = i+1; j < dim; j++) {
			const Vertex* vertex_i = &meshVertices.at(i);
			const Vertex* vertex_j = &meshVertices.at(j);

			// check if there is a half edge between vertex_i and vertex_j
			const Halfedge* currentHalfEdge = vertex_i->getHalfedge();
			do {
				currentHalfEdge = currentHalfEdge->getNext()->getSym();
			} while (currentHalfEdge != vertex_i->getHalfedge() && 
				currentHalfEdge->getSym()->getTarget() != vertex_j);

			if (currentHalfEdge == vertex_i->getHalfedge()) {
				continue;
			}

			assert(currentHalfEdge->getSym()->getTarget() == vertex_j);

			float w = desbrunWeight(currentHalfEdge);
			
			L(i, j) = w;
			L(j, i) = w;
			L(i, i) += -w;
			L(j, j) += -w;
		}
	}

	return L;
}

void laplacianTest(const Mesh* mesh) {
	auto laplacianMatrix = laplacian(*mesh);
	std::cout << laplacianMatrix;
}

float desbrunWeight(const Halfedge* halfEdge) {
	float weight = 0;
	float manifoldCount = 0;
	for (int i = 0; i < 2; i++) {
		if (halfEdge->getNext() == nullptr) {
			halfEdge = halfEdge->getSym();
			continue;
		}
		manifoldCount++;

		const Vertex* p1 = halfEdge->getTarget();
		const Vertex* p2 = halfEdge->getNext()->getTarget();
		const Vertex* p3 = halfEdge->getNext()->getNext()->getTarget();

		glm::vec3 v1, v2;
		v1 = glm::vec3(p1->getPosition() - p2->getPosition());
		v1 = glm::normalize(v1);
		v2 = glm::vec3(p3->getPosition() - p2->getPosition());
		v2 = glm::normalize(v2);

		float w1 = glm::dot(v1, v2) / glm::length(glm::cross(v1, v2));
		weight += w1;
	}
	weight /= manifoldCount;

	return weight;
}


