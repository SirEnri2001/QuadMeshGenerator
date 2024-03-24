#include "wave_quadrangulation.h"

using Eigen::Matrix;
using Eigen::MatrixXd;
using Eigen::Matrix4d;

namespace quadro {

    const Matrix<double, 4, 1> WaveQuadrangulation::e1(1, 0, 0, 0);
    const Matrix4d WaveQuadrangulation::H(e1* Matrix<double, 1, 4>(0, 0, 0, 1) - Matrix<double, 4, 1>(0, 1, 0, 0) * Matrix<double, 1, 4>(0, 0, 1, 0));


    WaveQuadrangulation::WaveQuadrangulation(Mesh* m) : MeshUserOperator(m), 
        c_ij(MatrixXd(m->getVertices().size(), m->getVertices().size())),
        f_i(MatrixXd(m->getVertices().size(), 4))
    {

    }

}