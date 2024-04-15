#include "eigen.h"
#include "httplib.h"

using namespace httplib;

std::pair<Eigen::VectorXd, Eigen::MatrixXd> eigh(const Eigen::MatrixXd& matrix) {
	httplib::Client cli("localhost", 5000);
	cli.set_connection_timeout(0, 300000); // 300 milliseconds
	cli.set_read_timeout(60, 0); // 5 seconds
	cli.set_write_timeout(60, 0); // 5 seconds
	size_t binary_size = matrix.size() * sizeof(double);
	size_t dimension = matrix.cols();
	httplib::MultipartFormDataItems req_body = {
		{ "data", std::string((char*)matrix.data(), binary_size), "data.bin", "application/octet-stream" }
	};
	if (auto res = cli.Post("/eigh", req_body)) {
		if (res->status == StatusCode::OK_200) {
			Eigen::VectorXd eigenvalues = Eigen::VectorXd::Map((double*)res->body.c_str(), dimension);
			Eigen::MatrixXd eigenvectors = Eigen::MatrixXd::Map((double*)res->body.c_str() + dimension, dimension, dimension);
			return std::pair<Eigen::VectorXd, Eigen::MatrixXd>(eigenvalues, eigenvectors);
		}
	}
	else {
		auto err = res.error();
		std::cout << "HTTP error: " << httplib::to_string(err) << " Please check whether server is available"<<std::endl;
		assert(false);
	}
	return std::pair<Eigen::VectorXd, Eigen::MatrixXd>();
}

Eigen::MatrixXd solve(const Eigen::MatrixXd& a, const Eigen::MatrixXd& b) {
	// https://numpy.org/doc/stable/reference/generated/numpy.linalg.solve.html
	httplib::Client cli("localhost", 5000);
	cli.set_connection_timeout(0, 3000000); // 300 milliseconds
	cli.set_read_timeout(60, 0); // 5 seconds
	cli.set_write_timeout(60, 0); // 5 seconds
	size_t M = a.cols();
	size_t K = b.cols();
	httplib::MultipartFormDataItems req_body = {
		{ "data_a", std::string((char*)a.data(), M * M*sizeof(double)), "data_a.bin", "application/octet-stream" },
		{ "data_b", std::string((char*)b.data(), M * K * sizeof(double)), "data_b.bin", "application/octet-stream" }
	};
	while (true) {
		if (auto res = cli.Post("/solve", req_body)) {
			if (res->status == StatusCode::OK_200) {
				Eigen::MatrixXd resMat = Eigen::MatrixXd::Map((double*)res->body.c_str(), M, K);
				return resMat;
			}
		}
		else {
			auto err = res.error();
			std::cout << "HTTP error: " << httplib::to_string(err) << " Please check whether server is available" << std::endl;
			//assert(false);
		}
	}
	
	
	return Eigen::MatrixXd();
}