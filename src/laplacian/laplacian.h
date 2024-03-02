#pragma once
#include <Eigen/Core>
#include "../mesh/mesh.h"

Eigen::MatrixXd laplacian(const Mesh& mesh);

void laplacianTest(const Mesh* mesh);