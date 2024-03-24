#include "msc.h"
#include "../laplacian/laplacian.h"
#include "../api/eigen.h"
#include "../mesh/components.h"
#include "../mesh/display.h"
#include <Eigen/Core>
using namespace quadro;
MorseFunction::MorseFunction(Mesh* mesh) : MeshUserOperator(mesh) {
	scalarFunction = mesh->createVertexAttribute<double>();
	vertexType = mesh->createVertexAttribute<VertexType>(NONE);
	vertexTracelines = mesh->createVertexAttribute<std::vector<Traceline*>>();
}

MorseFunction::~MorseFunction()
{
	mesh->removeVertexAttribute(scalarFunction.get());
}

void MorseFunction::smoothScalarFunction(const Vertex* v) {
	double sumVal = 0.0;
	int nums = 0;
	const Halfedge* he1 = v->getHalfedge();
	do {
		nums++;
		sumVal += getScalarFunction(he1->getSource()) - getScalarFunction(v);
	} while (he1 = he1->getNext()->getSym(), he1 != v->getHalfedge());
	(*scalarFunction)[v->getMutable()] = getScalarFunction(v) - sumVal / nums;
}

void MorseFunction::extractWaveFunction() {
	Eigen::MatrixXd LMatrix = laplacian(*mesh);
	auto eigenresult = eigh(LMatrix);
	Eigen::VectorXd eigenvector = eigenresult.second.col(mesh->getVertices().size()-20);
	for (auto& idVertex : mesh->getVertices()) {
		const Vertex* vertex = &idVertex.second;
		(*scalarFunction)[vertex->getMutable()] = eigenvector[idVertex.first];
	}
	//for (auto& idHalfedge : mesh->getHalfedges()) {
	//	const Halfedge* he = &idHalfedge.second;
	//	double v = getScalarFunction(he->getSource()) - getScalarFunction(he->getTarget());
	//	if (v != 0.) {
	//		continue;
	//	}
	//	smoothScalarFunction(he->getSource());
	//	smoothScalarFunction(he->getTarget());
	//}
	display->drawVertexAttribute(scalarFunction->map<glm::vec4>(
		[](float val)->glm::vec4 {
			val = (val*50 + 0.5) / 2;
			return glm::vec4(val, val, val, 1.0);
		}
	).get());
	display->create();
}

void MorseFunction::extractMorseVertices() {
	for (auto& idVertex : mesh->getVertices()) {
		const Vertex* v = &idVertex.second;
		const Halfedge* he = v->getHalfedge();
		int lowToHigh = 0;
		int HighToLow = 0;
		bool hasLow = false;
		bool hasHigh = false;
		do {
			//  |     /
			// left  /
			//  | right
			//  |  /
			//  (v)
			double left = getScalarFunction(v) - getScalarFunction(he->getSource());
			double right = getScalarFunction(v) - getScalarFunction(he->getNext()->getTarget());
			if (left > 0 && right < 0) {
				HighToLow++;
			}
			if (left < 0 && right>0) {
				lowToHigh++;
			}
			if (left > 0 || right > 0) {
				hasHigh = true;
			}
			if (left < 0 || right < 0) {
				hasLow = true;
			}
		} while (he = he->getNext()->getSym(), he != v->getHalfedge());
		if (!hasLow) {
			setVertexType(v->getMutable(), MAXIMUM);
		}
		else if (!hasHigh) {
			setVertexType(v->getMutable(), MINIMUM);
		}
		else if (lowToHigh > 1 || HighToLow > 1) {
			setVertexType(v->getMutable(), SADDLE);
			saddleVertices.push_back(v);
		}
		else {
			setVertexType(v->getMutable(), REGULAR);
		}
	}
	display->markVertexAttribute(vertexType->map<std::pair<bool, glm::vec4>>([](MorseFunction::VertexType vertexType)->std::pair<bool, glm::vec4> {
		switch (vertexType)
		{
		case quadro::MorseFunction::NONE:
			return { false, glm::vec4(0,0,0,0) };
			break;
		case quadro::MorseFunction::REGULAR:
			return { false, glm::vec4(0,0,0,0) };
			break;
		case quadro::MorseFunction::MAXIMUM:
			return { true, glm::vec4(1,0,0,1) };
			break;
		case quadro::MorseFunction::MINIMUM:
			return { true, glm::vec4(0,0,1,1) };
			break;
		case quadro::MorseFunction::SADDLE:
			return { true, glm::vec4(0,1,0,1) };
			break;
		default:
			return { false, glm::vec4(0,0,0,0) };
			break;
		}
	}).get());
	for (auto& idHe : mesh->getHalfedges()) {
		const Halfedge* he = &idHe.second;
		double v = getScalarFunction(he->getTarget()) - getScalarFunction(he->getSource());
		if (v > 0) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(0,0,0,1), glm::vec4(1,1,1,1));
		}
		if (v < 0) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(1,1,1,1), glm::vec4(0,0,0,1));
		}
		if (v == 0) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(0,0,0, 1), glm::vec4(0, 0, 0, 1));
		}
	}
}

double MorseFunction::getGradientScalarFunction(const Halfedge* he) {
	return (getScalarFunction(he->getTarget()) - getScalarFunction(he->getSource())) 
		/ glm::length(he->getTarget()->getPosition() - he->getSource()->getPosition());
}

void MorseFunction::extractMorseFunction() {
	for (const Vertex* v : saddleVertices) {
		//
		//  v1  v2  v3
		//   \  |  /
		//    \ | /
		//     (v)
		//
		const Halfedge* he = v->getHalfedge();
		const Halfedge* heStart = he;
		bool isPositive = false;
		while (true) {
			double vv1 = getGradientScalarFunction(he->getSym());
			double vv2 = getGradientScalarFunction(he->getNext());
			if (vv1 * vv2 <0) {
				heStart = he->getNext()->getSym();
				isPositive = vv2 > 0;
				break;
			}
			he = he->getNext()->getSym();
		}
		he = heStart;
		double tempValue = 0;
		const Halfedge* tempHe = nullptr;
		do {
			double vv1 = getGradientScalarFunction(he->getSym());
			double vv2 = getGradientScalarFunction(he->getNext());
			if (tempHe == nullptr) {
				tempHe = he->getSym();
			}
			isPositive = vv1 > 0;
			if (vv1 * vv2 < 0) {
				if (isPositive) {
					traceToMaximum(tempHe);
				}
				else {
					traceToMinimum(tempHe);
				}
				tempHe = nullptr;
				tempValue = 0.0;
				continue;
			}
			if (isPositive && vv1 > tempValue || !isPositive && vv1 < tempValue) {
				tempValue = vv1;
				tempHe = he->getSym();
			}
			if (isPositive && vv2 > tempValue || !isPositive && vv2 < tempValue) {
				tempValue = vv2;
				tempHe = he->getNext();
			}
		} while (he = he->getNext()->getSym(), he != heStart);
	}
}

MorseFunction::Traceline* MorseFunction::traceToMaximum(const Halfedge* he) {
	return trace(he, true);
}

MorseFunction::Traceline* MorseFunction::traceToMinimum(const Halfedge* he) {
	return trace(he, false);
}

MorseFunction::Traceline* MorseFunction::trace(const Halfedge* he, bool toMaximum) {
	tracelines.push_back(Traceline());
	Traceline* tr = &tracelines.back();
	tr->end1 = he->getSource();
	(*vertexTracelines)[he->getSource()->getMutable()].push_back(tr);
	while (toMaximum && getVertexType(he->getTarget()) != MAXIMUM || !toMaximum && getVertexType(he->getTarget()) != MINIMUM) {
		tr->line.push_back(he);
		const Vertex* v = he->getTarget();
		const Halfedge* tempHe = nullptr;
		const Halfedge* heStart = he;
		float tempVal = 0.0;
		if (toMaximum) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(1, 0, 0, 1), glm::vec4(1, 0, 0, 1));
		}
		else {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 1, 1));
		}
		do {
			double vv1 = getGradientScalarFunction(he->getSym());
			if (toMaximum && tempVal < vv1) {
				tempVal = vv1;
				tempHe = he;
			}
			if (!toMaximum && tempVal > vv1) {
				tempVal = vv1;
				tempHe = he;
			}
		} while (he = he->getNext()->getSym(), he != heStart);
		if (tempHe == nullptr) {
			display->markVertex(v);
			asserts->onPause();
		}
		assert(tempHe!=nullptr);
		he = tempHe->getSym();;
	}
	tr->line.push_back(he);
	tr->end2 = he->getTarget();
	(*vertexTracelines)[he->getTarget()->getMutable()].push_back(tr);
	if (toMaximum) {
		display->setHalfedgeMark(he->getMutable(), glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1));
	}
	else {
		display->setHalfedgeMark(he->getMutable(), glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 1, 1));
	}
	return tr;
}

void MorseFunction::operator()() {
	extractWaveFunction();
	extractMorseVertices();
	extractMorseFunction();
	cleanUpMorseFunction();
}

double MorseFunction::getScalarFunction(const Vertex* v) {
	return (*scalarFunction)[v];
}

MorseFunction::VertexType MorseFunction::getVertexType(const Vertex* v) {
	return (*vertexType)[v];
}
void MorseFunction::setVertexType(Vertex* v, VertexType t) {
	(*vertexType)[v] = t;
}

void MorseFunction::cleanUpMorseFunction() {

}