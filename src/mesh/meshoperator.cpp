#include "meshoperator.h"

MeshOperator::MeshOperator(Mesh* mesh) :mesh(mesh)
{

}

void MeshOperator::create() {

}

MeshInteriorOperator::MeshInteriorOperator(Mesh* mesh) : MeshOperator(mesh) {

}

MeshUserOperator::MeshUserOperator(Mesh* mesh) : MeshOperator(mesh) {

}