#pragma once
#include"../mesh/components.h"
#include "../mesh/core.h"
#include <Eigen/core>
#include <vector>


namespace quadro {
	class Parameterization {
		Mesh* mMesh;
		const std::unordered_map<ID, Vertex>& vertices;
		// TODO: comment: 
		Eigen::MatrixXd A;
		Eigen::MatrixX2d B;

	public:
		Parameterization(Mesh* mesh);
		void Init();

	};
}
