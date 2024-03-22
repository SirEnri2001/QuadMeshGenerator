#include "msc.h"
#include "../laplacian/laplacian.h"
#include "../api/eigen.h"
#include "../mesh/components.h"
#include "../mesh/display.h"
#include <Eigen/Core>
using namespace quadro;
MorseFunction::MorseFunction(Mesh* mesh) : MeshUserOperator(mesh) {
	scalarFunction = mesh->createVertexAttribute<float>();
}

MorseFunction::~MorseFunction()
{
	mesh->removeVertexAttribute(scalarFunction.get());
}

void MorseFunction::extractWaveFunction() {
	Eigen::MatrixXd LMatrix = laplacian(*mesh);
	auto eigenresult = eigh(LMatrix);
	Eigen::VectorXd eigenvector = eigenresult.second.col(mesh->getVertices().size()-10);
	for (auto& idVertex : mesh->getVertices()) {
		const Vertex* vertex = &idVertex.second;
		(*scalarFunction)[vertex->getMutable()] = eigenvector[idVertex.first];
	}
	Mesh* pMesh = mesh;
	display->drawVertexAttribute(scalarFunction.get()->map<glm::vec4>(
		[pMesh](float attrib) -> glm::vec4 {
			Mesh* mesh = pMesh;
			if (attrib > 0) {
				return glm::vec4(1, 0, 0, 1);
			}
			if (attrib < 0) {
				return glm::vec4(0, 1, 0, 1);
			}
			return glm::vec4(0, 0, 0, 1);
		}).get());
}

void MorseFunction::markMorseFunctionPoints() {

}

void MorseFunction::operator()() {
	extractWaveFunction();
}