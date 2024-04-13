#include "msc.h"
#include "../laplacian/laplacian.h"
#include "../api/eigen.h"
#include "../mesh/components.h"
#include "../mesh/display.h"
#include <Eigen/Core>
#include <iostream>
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
	Eigen::VectorXd eigenvector = eigenresult.second.col(mesh->getVertices().size()-40);
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
	tracelines.push_back(std::make_unique<Traceline>());
	Traceline* tr = tracelines.back().get();
	tr->end1 = he->getSource();
	(*vertexTracelines)[he->getSource()->getMutable()].push_back(tr);
	while (toMaximum && getVertexType(he->getTarget()) != MAXIMUM || !toMaximum && getVertexType(he->getTarget()) != MINIMUM) {
		tr->line.push_back(he);
		const Vertex* v = he->getTarget();
		const Halfedge* tempHe = nullptr;
		const Halfedge* heStart = he;
		float tempVal = 0.0;
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
	tr->toMaximum = toMaximum;
	(*vertexTracelines)[he->getTarget()->getMutable()].push_back(tr);
	return tr;
}

MorseFunction::Traceline* MorseFunction::trace(const Vertex* end1, const Vertex* end2) {
	struct Tree {
		const Vertex* v;
		const Halfedge* he;
		Tree* parent;
	};
	if (mesh->getHalfedge(end1, end2) != nullptr) {
		tracelines.push_back(std::make_unique<Traceline>(Traceline{ end1, end2, {mesh->getHalfedge(end1, end2)}, getScalarFunction(end1) < getScalarFunction(end2) }));
		return tracelines.back().get();
	}
	bool toMaximum = getScalarFunction(end1) < getScalarFunction(end2);
	std::vector<std::unique_ptr<Tree>> trees;
	trees.push_back(std::make_unique<Tree>(Tree({ end1, nullptr, nullptr})));
	Tree* root = trees.back().get();
	Tree* tree = root;
	const Vertex* v = end1;
	const Halfedge* he = v->getHalfedge();
	std::vector<Tree*> currentRingTrees;
	std::vector<Tree*> nextRingTrees;
	std::unordered_map<int, int> traversed;
	traversed[v->getId()] = 1;
	bool find = false;
	currentRingTrees.push_back(root);
	while (true) {
		for (Tree* t : currentRingTrees) {
			v = t->v;
			he = v->getHalfedge();
			do {
				if (v == end2) {
					find = true;
					tree = t;
					break;
				}
				if (traversed[he->getSource()->getId()] == 1) {
					continue;
				}
				trees.push_back(std::make_unique<Tree>(Tree{ he->getSource(), he, t }));
				nextRingTrees.push_back(trees.back().get());
				traversed[he->getSource()->getId()] = 1;
			} while (he = he->getNext()->getSym(), he != v->getHalfedge());
			if (find) {
				break;
			}
		}
		currentRingTrees = nextRingTrees;
		if (find) {
			break;
		}
	}
	tracelines.push_back(std::make_unique<Traceline>(Traceline{ end1, end2, {}, getScalarFunction(end1) < getScalarFunction(end2) }));
	Traceline* tr = tracelines.back().get();
	while (tree != nullptr && tree->he!=nullptr) {
		tr->line.push_front(tree->he);
		tree = tree->parent;
	}
	(*vertexTracelines)[end1->getMutable()].push_back(tr);
	(*vertexTracelines)[end2->getMutable()].push_back(tr);
	return tr;
}

void MorseFunction::operator()() {
	extractWaveFunction();
	extractMorseVertices();
	extractMorseFunction();
	cleanUpMorseFunction();
	extractQuasiDualComplex();
	paintMorseFunction();
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

void MorseFunction::removeMorseFunctionVertex(const Vertex* v) {
	if ((*vertexTracelines)[v].size() == 0) {
		return;
	}
	for (Traceline* tr : (*vertexTracelines)[v]) {
		const Vertex* other = tr->end1 == v ? tr->end2 : tr->end1;
		std::vector<Traceline*>& lines = (*vertexTracelines)[other->getMutable()];
		lines.erase(std::remove(lines.begin(), lines.end(), tr), lines.end());
		tr->isValid = false;
		//tracelines.erase(std::remove_if(tracelines.begin(), tracelines.end(), [tr](const std::unique_ptr<Traceline>& tr1) {return tr1.get() != tr; }), tracelines.end());
	}
	(*vertexTracelines)[v->getMutable()].clear();
}

void MorseFunction::cleanUpMorseFunction() {
	bool optimized = true;
	do {
		optimized = true;
		for (const auto& idVertex : mesh->getVertices()) {
			const Vertex* v = &idVertex.second;
			if ((*vertexType)[v] != MAXIMUM && (*vertexType)[v] != MINIMUM) {
				continue;
			}
			if ((*vertexTracelines)[v].size() != 2) {
				continue;
			}
			Traceline* tr =
				(*vertexTracelines)[v][0]->line.size() > (*vertexTracelines)[v][1]->line.size() ?
				(*vertexTracelines)[v][1] : (*vertexTracelines)[v][0];
			const Vertex* saddleV = tr->end1;
			const Vertex* end1ToBeConnected = tr == (*vertexTracelines)[v][1]? (*vertexTracelines)[v][0]->end1 : (*vertexTracelines)[v][1]->end1;
			const Vertex* end2ToBeConnected = nullptr;
			for (Traceline* tr1 : (*vertexTracelines)[saddleV]) {
				if ((*vertexType)[tr1->end2] == (*vertexType)[v] && tr1->end2 != v) {
					end2ToBeConnected = tr1->end2;
				}
			}
			removeMorseFunctionVertex(saddleV);
			removeMorseFunctionVertex(v);
			trace(end1ToBeConnected, end2ToBeConnected);
			optimized = false;
		}
	} while (!optimized);
}

void MorseFunction::extractQuasiDualComplex() {
	for (auto& idVertex : mesh->getVertices()) {
		const Vertex* v = &idVertex.second;
		if (getVertexType(v) != SADDLE) {
			continue;
		}
		for (int i = 0; i < (*vertexTracelines)[v].size(); i++) {
			Traceline* tr = (*vertexTracelines)[v][i];
			if (getVertexType(tr->end2) != MINIMUM) {
				continue;
			}
			Traceline* nextTr = (*vertexTracelines)[v][(i+1) % (*vertexTracelines)[v].size()];
			trace(tr->end2, nextTr->end2);
		}
		removeMorseFunctionVertex(v);
	}
}


void MorseFunction::paintMorseFunction() {
	std::unordered_map<int, std::string> saddleIdMap;
	std::unordered_map<int, std::string> extremeIdMap;
	int saddleCount = 0;
	int extremeCount = 0;
	//for (const auto& uptr_tr : tracelines) {
	//	const Traceline* tr = uptr_tr.get();
	//	if (!tr->isValid) {
	//		continue;
	//	}
	//	if (saddleIdMap.find(tr->end1->getId()) == saddleIdMap.end()) {
	//		saddleIdMap[tr->end1->getId()] = "s" + std::to_string(++saddleCount);
	//	}
	//	if (extremeIdMap.find(tr->end2->getId()) == extremeIdMap.end()) {
	//		extremeIdMap[tr->end2->getId()] = "v" + std::to_string(++extremeCount);
	//	}
	//	std::cout << tr->end1->getId() << " - " << tr->end2->getId() << std::endl;
	//	//std::cout << tr.end1->getId() << " - " << tr.end2->getId() << std::endl;
	//}

	for (auto& idVertex : mesh->getVertices()) {
		const Vertex* v = &idVertex.second;
		if (getVertexType(v) == MAXIMUM || getVertexType(v) == MINIMUM) {
			std::cout << "Extreme : " << (*vertexTracelines)[v].size() << std::endl;
		}
	}
	
	for (const auto& uptr_tr : tracelines) {
		const Traceline& tr = *uptr_tr;
		if (!tr.isValid) {
			continue;
		}
		std::cout << tr.end1->getId() << " - " << tr.end2->getId() << std::endl;
		glm::vec4 traceColor = glm::vec4(0, 0, 1, 1);
		if (tr.toMaximum) {
			traceColor = glm::vec4(1, 0, 0, 1);
		}
		display->setMarked(tr.end1->getMutable(), true);
		display->setMarkColor(tr.end1->getMutable(), glm::vec4(0, 1, 0, 1));
		display->setMarked(tr.end2->getMutable(), true);
		display->setMarkColor(tr.end2->getMutable(), traceColor);
		for (const Halfedge* he : tr.line) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(1, 1, 1, 1), glm::vec4(traceColor));
		}
	}
	//for (const auto& uptr_tr : tracelines) {
	//	const Traceline& tr = *uptr_tr;
	//	if (tr.isValid) {
	//		continue;
	//	}
	//	for (const Halfedge* he : tr.line) {
	//		display->setHalfedgeMark(he->getMutable(), glm::vec4(1, 0, 1, 1), glm::vec4(1, 0, 1, 1));
	//	}
	//}

}

void MorseFunction::paintWaveFunction() {
	for (auto& idHe : mesh->getHalfedges()) {
		const Halfedge* he = &idHe.second;
		double v = getScalarFunction(he->getTarget()) - getScalarFunction(he->getSource());

		if (v > 0) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(0, 0, 0, 1), glm::vec4(1, 1, 1, 1));
		}
		if (v < 0) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(1, 1, 1, 1), glm::vec4(0, 0, 0, 1));
		}
		if (v == 0) {
			display->setHalfedgeMark(he->getMutable(), glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1));
		}
	}
}