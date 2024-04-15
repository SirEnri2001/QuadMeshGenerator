#pragma once

#include "../mesh/operator.h"
#include "../mesh/components.h"
#include <Eigen/Core>


namespace quadro {
	class WaveQuadrangulation :public MeshUserOperator {
	private:
		// Data


		// For Gaussian-Newton iterations
		static const Eigen::Matrix<double, 4, 1> e1; // [1,0,0,0]^T
		static const Eigen::Matrix4d H; // H = e_1 * e_4^T - e_2 * e_3^T
		Eigen::MatrixXd c_ij;
		Eigen::MatrixXd f_i;

	private:
		void initializeUnitVectorFields();
	public:
		WaveQuadrangulation(Mesh* m);

		// do one iteration
		void optimizeN(int interation_count);
		void optimize();

		// extract wave function


		// reset
		void resetWave();
	};

	


}