#pragma once
#include <Eigen/Core>
#include "../mesh/core.h"

namespace quadro {
	void laplacianTest(const Mesh* mesh);
	Eigen::MatrixXd laplacian(const Mesh& mesh);
}