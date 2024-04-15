#include "Parameterization.h"
#include "../laplacian/laplacian.h"

using std::vector;

namespace quadro {

	Parameterization::Parameterization(Mesh* mesh) :
		mMesh(mesh), 
		vertices(mesh->getVertices()), 
		A(Eigen::MatrixXd::Constant(vertices.size(), vertices.size(), 0)), 
		B(Eigen::MatrixX2d(vertices.size(), 0))	
	{}

	void Parameterization::Init()
	{
		int vertCount = vertices.size();
		Eigen::VectorXd zeroCoeffs(vertCount, 0);

		// fill A and B 
		const Eigen::MatrixXd laplacianMat = laplacian(*mMesh);
		A = laplacianMat;

		auto itr = vertices.begin();
		for (int vi = 0; vi < vertCount; vi++, itr++) {
			// get current vertex
			const Vertex* curVert = &itr->second;

			// TODO: check if current vertex is at a corner of MSC
			bool isCorner;
			if (isCorner) {
				A.row(vi) = zeroCoeffs;
			}

			// fill in laplacian
			auto he = curVert->getHalfedge();
			do {
				const Vertex* neighborVert = he->getSource();
				
				// TODO: check if at corner
				bool atCorner;
				if (atCorner) {
					// TODO: update B
					continue;
				}

				// TODO: check if at different pathc
				bool differentPatch;
				if (differentPatch) {
					// TODO: update B

					continue;
				}


				he = he->getNext()->getSym();
			} while (he != curVert->getHalfedge());
		}
		
	}

}
