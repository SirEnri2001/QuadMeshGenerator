#include <Eigen/Dense>
#include "laplacian.h"
#include "../mesh/components.h"
#include <map>
#include "../mesh/io.h"
#include <glm.hpp>
#include <omp.h>
#include <iostream>
#include <chrono>

namespace quadro {
	float desbrunWeight(const Halfedge* halfEdge);
	Eigen::MatrixXd laplacian(const Mesh& mesh);

	void svdTest(const Eigen::MatrixXd& laplacianMatrix) {
		Eigen::setNbThreads(16);
		int n = Eigen::nbThreads();

		std::cout << "Threads used: " << n << std::endl;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		auto& ret = Eigen::BDCSVD<Eigen::MatrixXd>::BDCSVD().compute(laplacianMatrix);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		std::cout << std::chrono::duration_cast<std::chrono::seconds>(end - begin) << std::endl;

		return;
	}


	void laplacianTest(const Mesh* mesh) {
		auto laplacianMatrix = laplacian(*mesh);
		std::cout << "Vertices: " << mesh->getVertices().size() << std::endl;
		svdTest(laplacianMatrix);
		std::cout << "laplacianTest complete" << std::endl << std::flush;
	}


	Eigen::MatrixXd laplacian(const Mesh& mesh)
	{
		const auto& meshVertices = mesh.getVertices();
		int dim = meshVertices.size();

		Eigen::MatrixXd L = Eigen::MatrixXd::Constant(dim, dim, 0);
		for (int i = 0; i < dim; i++) {
			for (int j = i + 1; j < dim; j++) {
				const Vertex* vertex_i = &(meshVertices.at(i));
				const Vertex* vertex_j = &(meshVertices.at(j));

				const Halfedge* halfEdgeBetweenVerts = mesh.getHalfedge(vertex_i, vertex_j);
				if (!halfEdgeBetweenVerts) {
					continue;
				}

				float w = desbrunWeight(halfEdgeBetweenVerts);
				L(i, j) = w;
				L(j, i) = w;
				L(i, i) += -w;
				L(j, j) += -w;
			}
		}
		return L;
	}


	float desbrunWeight(const Halfedge* halfEdge) {
		float weight = 0;
		float manifoldCount = 0;
		for (int i = 0; i < 2; i++, halfEdge = halfEdge->getSym()) {
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

			float tmp1 = glm::dot(v1, v2),
				tmp2 = glm::length(glm::cross(v1, v2));
			float w = tmp1 / tmp2;
			weight += w;
		}
		weight /= manifoldCount;
		return weight;
	}

}
