#include <Eigen/Dense>

std::pair<Eigen::VectorXd, Eigen::MatrixXd> eigh(const Eigen::MatrixXd& matrix);
Eigen::MatrixXd solve(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b);