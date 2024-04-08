#pragma once
#include"../mesh/components.h"
#include "../mesh/core.h"
#include <Eigen/core>

namespace quadro {
	class Parameterization {
		Eigen::MatrixXd laplacian;
		Parameterization(const Mesh* mesh);

		// calculate each 

	};
}
