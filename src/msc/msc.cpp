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
	display->drawVertexAttribute(scalarFunction.get());
}

void MorseFunction::markMorseFunctionPoints() {

}

void MorseFunction::operator()() {
	extractWaveFunction();
}