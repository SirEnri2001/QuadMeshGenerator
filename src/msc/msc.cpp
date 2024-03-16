#include "msc.h"
#include "../laplacian/laplacian.h"
#include "../api/eigen.h"
#include "../mesh/meshcomponents.h"
#include <Eigen/Core>

MorseFunction::MorseFunction(Mesh* mesh) : MeshUserOperator(mesh) {

}

void MorseFunction::extractWaveFunction() {
	Eigen::MatrixXd LMatrix = laplacian(*mesh);
	auto eigenresult = eigh(LMatrix);
	Eigen::VectorXd eigenvector = eigenresult.second.col(mesh->getVertices().size()-10);
	for (auto& idVertex : mesh->getVertices()) {
		const Vertex* vertex = &idVertex.second;
		if (eigenvector[idVertex.first] < 0) {
			vertex->getMutable()->setColor(glm::vec4(1,0, 0, 1));
		}
		if (eigenvector[idVertex.first] > 0) {
			vertex->getMutable()->setColor(glm::vec4(0, 1, 0, 1));
		}
	}
}

void MorseFunction::markMorseFunctionPoints() {

}

void MorseFunction::operator()() {
	extractWaveFunction();
}