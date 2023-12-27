#include "../mesh/mesh.h"
#include <list>


struct FrontEdge {
	Halfedge* he;
	FrontEdge* prevFe;
	FrontEdge* nextFe;
};

class Smoother {
	Smoother();
};

class FrontEdgeOperator : public MeshOperator {
	std::unordered_map<Halfedge*, FrontEdge> heToFe;
	std::list<FrontEdge*> frontEdgeGroups;
public:
	int countFeToFe(FrontEdge* fe1, FrontEdge* fe2) {
		int count = 1;
		FrontEdge* feIter = fe1;
		while (feIter != fe2) {
			count++;
			feIter = getNextFe(feIter);
		}
		return count;
	}
	int initFrontEdgeGroup() {
		int i = 0;

		for (CTMesh::HalfedgeIter heiter(mesh); !heiter.end(); heiter++) {
			if (mesh->isBoundary(*heiter) && !mesh->isFront(mesh->halfedgeSym(*heiter))) {
				HalfedgeHandle curHe = *heiter;
				do {
					HalfedgeHandle curFe = mesh->halfedgeSym(curHe);
					HalfedgeHandle nextFe = mesh->halfedgeSym(mesh->halfedgePrev(curHe));
					mesh->setFront(curFe, true);
					mesh->setFront(nextFe, true);
					mesh->topology_assert(mesh->halfedgeTarget(curFe) == mesh->halfedgeSource(nextFe), { curFe, nextFe });
					mesh->setNextFe(mesh->halfedgeSym(curHe), mesh->halfedgeSym(mesh->halfedgePrev(curHe)));
					curHe = mesh->halfedgePrev(curHe);
					i++;
				} while (curHe != *heiter);
				pushTailFrontEdgeGroup(mesh->halfedgeSym(*heiter));
			}
		}
		if (i % 2 != 0)
		{
			HalfedgeHandle longestHE = getFrontEdgeGroup();
			HalfedgeHandle he = longestHE;
			do {
				if (eDist(mesh->halfedgeSource(he)->point(), mesh->halfedgeTarget(he)->point()) >
					eDist(mesh->halfedgeSource(longestHE)->point(), mesh->halfedgeTarget(longestHE)->point())) {
					longestHE = he;
				}
			} while ((he = mesh->getNextFe(he)) != longestHE);
			HalfedgeHandle prevFe, nextFe;
			VertexHandle v1, v2, vNew;
			prevFe = mesh->getPrevFe(longestHE);
			nextFe = mesh->getNextFe(longestHE);

			v1 = mesh->halfedgeSource(longestHE);
			v2 = mesh->halfedgeTarget(longestHE);
			mesh->setFront(longestHE, false);
			vNew = mesh->splitEdge(longestHE,
				(mesh->getPoint(mesh->halfedgeTarget(longestHE)) + mesh->getPoint(mesh->halfedgeSource(longestHE))) / 2);

			mesh->setFront(mesh->sourceTargetHalfedge(v1, vNew), true);
			mesh->setFront(mesh->sourceTargetHalfedge(vNew, v2), true);

			mesh->setNextFe(prevFe, mesh->sourceTargetHalfedge(v1, vNew));
			mesh->setNextFe(mesh->sourceTargetHalfedge(v1, vNew), mesh->sourceTargetHalfedge(vNew, v2));
			mesh->setNextFe(mesh->sourceTargetHalfedge(vNew, v2), nextFe);
			updateHeadFrontEdgeGroup(prevFe);

		}
		return 0;
	}

	FrontEdge* getNextFe(FrontEdge* fe) {
		return fe->nextFe;
	}
	bool isFront(Halfedge* he) {
		return heToFe.find(he)!=heToFe.cend();
	}
	int frontEdgeGroupSize(FrontEdge* fe) {
		int count = 0;
		FrontEdge* feIter = fe;
		do {
			count++;
		} while (
			feIter = getNextFe(feIter), feIter != fe);
		return count;
	}

	void pushHeadFrontEdgeGroup(FrontEdge* fe) {
		frontEdgeGroups.push_front(fe);
	}

	void pushTailFrontEdgeGroup(FrontEdge* fe) {
		frontEdgeGroups.push_back(fe);
	}

	// param he: any frontedge in the group
	void removeFrontEdgeGroup(FrontEdge* fe) {
		FrontEdge* feIter = fe;
		do {
			frontEdgeGroups.remove(feIter);
		} while (feIter = getNextFe(feIter), feIter != fe);
	}

	bool isFrontEdgeGroupIndex(FrontEdge* he) {
		return find(frontEdgeGroups.begin(), frontEdgeGroups.end(), he) != frontEdgeGroups.end();
	}

	FrontEdge* popFrontEdgeGroup() {
		FrontEdge* he = frontEdgeGroups.front();
		frontEdgeGroups.pop_front();
		return he;
	}
	FrontEdge* getFrontEdgeGroup() {
		if (frontEdgeGroups.size() > 0) {
			return frontEdgeGroups.front();
		}
		else {
			return NULL;
		}
	}
	bool switchFrontEdgeGroup() {
		pushTailFrontEdgeGroup(getFrontEdgeGroup());
		popFrontEdgeGroup();
		return getFrontEdgeGroup() != NULL;
	}
	void updateHeadFrontEdgeGroup(FrontEdge* he) {
		popFrontEdgeGroup();
		pushHeadFrontEdgeGroup(he);
	}
};

class SideDefineOperator : public MeshOperator {
public:
	typedef enum SideDefineResult {
		Succeeded,
		NoSuitable,
		FrontEdgeContact,
		FrontEdgeContactDegenerate,
		SideEdgeContact,
		QuadContactByVertex,
		QuadContactByEdge
	} SideDefineResult;
	// these functions only return its finding of side he & its topology state
	SideDefineResult verticalSideSeek(FrontEdge* lfe, FrontEdge* rfe, Halfedge*& resultUpSide) {
		mesh->topology_assert(mesh->halfedgeTarget(lfe) == mesh->halfedgeSource(rfe), { lfe, rfe });
		HalfedgeHandle minAngleHe = lfe;
		VertexHandle pivotVertex = mesh->halfedgeTarget(lfe);
		Point pivot = mesh->getPoint(pivotVertex);
		Point bisector = mesh->bisector(lfe, rfe);
		//int index;
		//bisector = mesh->nearestCrossField(pivotVertex, bisector, index);
		VertexInFrontHeIterator pivotIter(mesh, pivotVertex, lfe, rfe);
		double minAngle = 360.0;
		bool traversed = false;
		for (; !pivotIter.end(); pivotIter++) {
			if (mesh->isBoundary(mesh->halfedgeSym(*pivotIter))) {
				continue;
			}
			traversed = true;
			double angle = mesh->angle(bisector + pivot, mesh->halfedgeSym(*pivotIter));
			if (angle < minAngle) {
				minAngle = angle;
				minAngleHe = *pivotIter;
			}
		}
		if (!traversed) {
			return SideDefineResult::NoSuitable;
		}
		resultUpSide = mesh->halfedgeSym(minAngleHe);
		if (mesh->halfedgeSource(mesh->getPrevFe(lfe)) == mesh->halfedgeSource(minAngleHe)
			|| mesh->halfedgeTarget(mesh->getNextFe(rfe)) == mesh->halfedgeSource(minAngleHe)) {
			// size 3 front edge group
			return SideDefineResult::FrontEdgeContactDegenerate;
		}
		if (mesh->isFront(mesh->halfedgeSource(minAngleHe))) {
			return SideDefineResult::FrontEdgeContact;
		}
		if (mesh->numQuad(mesh->halfedgeSource(minAngleHe)) > 0) {
			return SideDefineResult::QuadContactByVertex;
		}
		if (mesh->halfedgeSource(minAngleHe)->isSide) {
			return SideDefineResult::SideEdgeContact;
		}
		if (minAngle < constEpsilon) {
			//mesh->alignToCrossField(mesh->halfedgeEdge(minAngleHe), pivotVertex);
			return SideDefineResult::Succeeded;
		}
		else {
			return SideDefineResult::NoSuitable;
		}
	}
	SideDefineResult horizontalSideSeek(FrontEdge* lfe, FrontEdge* rfe, Halfedge*& resultUpSide) {
		mesh->topology_assert(mesh->halfedgeTarget(lfe) == mesh->halfedgeSource(rfe), { lfe, rfe });
		HalfedgeHandle minAngleHe = lfe;
		VertexHandle pivotVertex = mesh->halfedgeTarget(lfe);
		Point pivot = mesh->getPoint(pivotVertex);
		Point bisector = mesh->bisector(lfe, rfe);
		VertexInFrontHeIterator pivotIter(mesh, pivotVertex, lfe, rfe);
		bool traversed = false;
		double minAngle = 360.0;
		// traverse all quadrilateral surround the vertex
		//(mAV)<--.
		//   |   / \
		//   |  /   \
		//   | /     \
		//   v--minAH>(pV)
		VertexHandle minAngleVertex = mesh->halfedgeSource(lfe);
		for (pivotIter.reset(); !pivotIter.end(); pivotIter++)
		{
			if (mesh->isFront(mesh->halfedgePrev(*pivotIter))
				|| mesh->isQuad(mesh->halfedgeFace(mesh->halfedgeSym(mesh->halfedgePrev(*pivotIter))))) {
				continue;
			}
			traversed = true;
			VertexHandle curVertex =
				mesh->halfedgeTarget(mesh->halfedgeNext(mesh->halfedgeSym(mesh->halfedgePrev(*pivotIter))));
			double angle = mesh->angle(bisector + pivot, pivotVertex, mesh->getPoint(curVertex));
			if (angle < minAngle) {
				minAngle = angle;
				minAngleHe = *pivotIter;
				minAngleVertex = curVertex;
			}
		}
		if (!traversed) {
			return SideDefineResult::NoSuitable;
		}
		resMinAH = minAngleHe;
		if (minAngleVertex == mesh->halfedgeSource(mesh->getPrevFe(lfe)) ||
			minAngleVertex == mesh->halfedgeTarget(mesh->getNextFe(rfe))) {
			return SideDefineResult::FrontEdgeContactDegenerate;
		}
		if (mesh->isFront(minAngleVertex)) {
			return SideDefineResult::FrontEdgeContact;
		}
		if (mesh->numQuad(minAngleVertex)) {
			return SideDefineResult::QuadContactByVertex;
		}
		if (minAngleVertex->isSide) {
			return SideDefineResult::SideEdgeContact;
		}
		if (minAngle < constEpsilon
			&& 2.0 * eDist(mesh->getPoint(minAngleVertex), mesh->getPoint(mesh->halfedgeTarget(lfe))) <
			sqrt(3) * (mesh->length(lfe) + mesh->length(rfe))) {  //safisty second condition
			return SideDefineResult::Succeeded;
		}
		else {
			return SideDefineResult::NoSuitable;
		}
	}
	SideDefineResult verticalSideSplitSeek(FrontEdge* lfe, FrontEdge* rfe, Halfedge*& resultUpSide) {
		mesh->topology_assert(mesh->halfedgeTarget(lfe) == mesh->halfedgeSource(rfe), { lfe, rfe });
		mesh->topology_assert(mesh->halfedgeNext(lfe) != rfe, { lfe, rfe });
		HalfedgeHandle minAngleHe = lfe;
		VertexHandle pivotVertex = mesh->halfedgeTarget(lfe);
		Point pivot = mesh->getPoint(pivotVertex);
		Point bisector = mesh->bisector(lfe, rfe);
		VertexInFrontHeIterator pivotIter(mesh, pivotVertex, lfe, rfe);
		double minAngle = 360.0;

		for (; !pivotIter.end(); pivotIter++) {
			if (mesh->isBoundary(mesh->halfedgeSym(*pivotIter))) {
				continue;
			}
			double angle = mesh->angle(bisector + pivot, mesh->halfedgeSym(*pivotIter));
			if (angle < minAngle) {
				minAngle = angle;
				minAngleHe = *pivotIter;
			}
		}
		resultUpSide = mesh->halfedgeSym(minAngleHe);
		mesh->topology_assert(minAngleHe != lfe, { lfe });
		if (mesh->halfedgeSource(mesh->getPrevFe(lfe)) == mesh->halfedgeSource(minAngleHe)
			|| mesh->halfedgeTarget(mesh->getNextFe(rfe)) == mesh->halfedgeSource(minAngleHe)) {
			// size 3 front edge group
			return SideDefineResult::FrontEdgeContactDegenerate;
		}
		if (mesh->isFront(mesh->halfedgeSource(minAngleHe))) {
			return SideDefineResult::FrontEdgeContact;
		}
		return SideDefineResult::Succeeded;
	}
	SideDefineResult horizontalSideSplitSeek(FrontEdge* lfe, FrontEdge* rfe, Halfedge*& resultUpSide) {
		mesh->topology_assert(mesh->halfedgeTarget(lfe) == mesh->halfedgeSource(rfe), { lfe, rfe });
		HalfedgeHandle minAngleHe = lfe;
		VertexHandle pivotVertex = mesh->halfedgeTarget(lfe);
		Point pivot = mesh->getPoint(pivotVertex);
		Point bisector = mesh->bisector(lfe, rfe);
		Point leftLocal = mesh->getPoint(mesh->halfedgeSource(lfe)) - pivot;
		Point rightLocal = mesh->getPoint(mesh->halfedgeTarget(rfe)) - pivot;
		Point cs = leftLocal.cross(bisector);
		cout << cs[0] << cs[1] << cs[2];
		VertexInFrontHeIterator pivotIter(mesh, pivotVertex, lfe, rfe);
		double minAngle = 360.0;

		HalfedgeHandle he2 = minAngleHe;


		for (pivotIter.reset(); !pivotIter.end(); pivotIter++) {
			if ((bisector.cross(mesh->getPoint(mesh->halfedgeSource(*pivotIter)) - pivot))
				.dot(bisector.cross(mesh->getPoint(mesh->halfedgeTarget(mesh->halfedgeNext(*pivotIter))) - pivot)) < 0) {
				he2 = *pivotIter;
			}
		}
		//(mAV)<----.
		//   |    /   \
		//   |   /     he1
		//   | /          \
		//   v--minAH(he2)-->(pV)
		resMinAH = he2;
		HalfedgeHandle he1 = mesh->halfedgeNext(he2);
		// 2 halfedges in a patch: --he2-->.--he1-->
		if (mesh->halfedgeSource(he2) != mesh->halfedgeSource(lfe) && mesh->isFront(mesh->halfedgePrev(he2))) {
			return SideDefineResult::FrontEdgeContactDegenerate;
		}
		if (mesh->isQuad(mesh->halfedgeFace(mesh->halfedgeSym(mesh->halfedgePrev(he2))))) {
			return SideDefineResult::QuadContactByEdge;
		}
		if (mesh->halfedgeSource(he2)->isSide || mesh->halfedgeTarget(he1)->isSide) {
			return SideDefineResult::SideEdgeContact;
		}
		return SideDefineResult::Succeeded;
		//Note: if lfe & rfe are on one corner, lfe->target == rfe->source, and no degeneracy detected, 
		// then this function must return Succeeded
	}

	int frontEdgeSideDefine(FrontEdge* lfe, FrontEdge* rfe) {
		if (mesh->isQuad(mesh->halfedgeFace(lfe))) {
			mesh->setSide(lfe, rfe, mesh->halfedgeNext(lfe));
			return 0;
		}
		if (mesh->isQuad(mesh->halfedgeFace(rfe))) {
			mesh->setSide(lfe, rfe, mesh->halfedgeSym(mesh->halfedgePrev(rfe)));
			return 0;
		}
		HalfedgeHandle resultUpSide;
		switch (verticalSideSeek(lfe, rfe, resultUpSide))
		{
		case SideDefineResult::FrontEdgeContact:
			seperateFrontLoop(resultUpSide);
			return 1;
		case SideDefineResult::Succeeded:
			mesh->setSide(lfe, rfe, resultUpSide);
			return 0;
		default:
			break;
		}

		switch (horizontalSideSeek(lfe, rfe, resultUpSide)) {
		case SideDefineResult::FrontEdgeContact:
			mesh->swapEdge(mesh->halfedgeEdge(mesh->halfedgePrev(resultUpSide)));
			seperateFrontLoop(mesh->halfedgeNext(resultUpSide));
			return 1;
		case SideDefineResult::Succeeded:
			mesh->swapEdge(mesh->halfedgeEdge(mesh->halfedgePrev(resultUpSide)));
			mesh->setSide(lfe, rfe, mesh->halfedgeNext(resultUpSide));
			return 0;
		default:
			break;
		}
		if (horizontalSideSplitSeek(lfe, rfe, resultUpSide) == SideDefineResult::Succeeded) {
			HalfedgeHandle he2 = resultUpSide;
			HalfedgeHandle he1 = mesh->halfedgeNext(he2);
			VertexHandle pivotVertex = mesh->halfedgeTarget(lfe);
			// solve the 2x2 linear system
			Point v1 = mesh->getPoint(mesh->halfedgeSource(he2)) - mesh->getPoint(mesh->halfedgeTarget(he2));
			Point v2 = mesh->getPoint(mesh->halfedgeTarget(he1)) - mesh->getPoint(mesh->halfedgeTarget(he2));
			Point bisector = mesh->bisector(lfe, rfe);
			double* km = solveKMEquation(v1, v2, bisector);
			Point p = bisector * (km[0] + EPSILON * 10.0) + mesh->getPoint(pivotVertex);
			VertexHandle spliter = mesh->splitEdge(mesh->halfedgePrev(he2), p);
			HalfedgeHandle newHe = mesh->sourceTargetHalfedge(pivotVertex, spliter);
			assert(newHe);
			mesh->setSide(lfe, rfe, newHe);
			return 0;
		}

		switch (verticalSideSplitSeek(lfe, rfe, resultUpSide))
		{
		case SideDefineResult::FrontEdgeContact:
		case SideDefineResult::FrontEdgeContactDegenerate:
			seperateFrontLoop(resultUpSide);
			return 1;
		case SideDefineResult::Succeeded:
			mesh->topology_assert(!mesh->isQuad(mesh->halfedgeFace(resultUpSide)), { resultUpSide });
			mesh->topology_assert(!mesh->isQuad(mesh->halfedgeFace(mesh->halfedgeSym(resultUpSide))), { resultUpSide });
			mesh->setSide(lfe, rfe, NULL);
			VertexHandle v = mesh->splitEdge(resultUpSide,
				(mesh->getPoint(mesh->halfedgeSource(resultUpSide)) + mesh->getPoint(mesh->halfedgeTarget(resultUpSide))) / 2);
			resultUpSide = mesh->sourceTargetHalfedge(mesh->halfedgeTarget(lfe), v);
			mesh->setSide(lfe, rfe, resultUpSide);
			return 0;
		}
		assert(false);
	}
	int generateCorner(FrontEdge* lfe, FrontEdge* rfe) {
		mesh->setSide(mesh->getPrevFe(lfe), lfe, NULL);
		mesh->setSide(lfe, rfe, NULL);
		mesh->setSide(rfe, mesh->getNextFe(rfe), NULL);
		if (mesh->isQuad(mesh->halfedgeFace(mesh->getNextFe(rfe)))) {
			mesh->buildQuad(lfe, rfe, mesh->halfedgeSym(mesh->halfedgePrev(mesh->getNextFe(rfe))));
			mesh->setTopEdge(rfe, mesh->halfedgePrev(lfe));
			mesh->setSide(mesh->getPrevFe(lfe), lfe, mesh->halfedgeSym(mesh->getTopEdge(rfe)));
		}
		else if (mesh->isQuad(mesh->halfedgeFace(mesh->getPrevFe(lfe)))) {
			mesh->buildQuad(mesh->halfedgeSym(mesh->halfedgeNext(mesh->getPrevFe(lfe))), lfe, rfe);
			mesh->setTopEdge(lfe, mesh->halfedgeNext(rfe));
			mesh->setSide(rfe, mesh->getNextFe(rfe), mesh->getTopEdge(lfe));
		}
		else {
			if (frontEdgeSideDefine(mesh->getPrevFe(lfe), lfe)) { //fail to define side edge
				return 1; //reclassify corner must be called again
			}
			mesh->buildQuad(mesh->getLeftSide(lfe), lfe, rfe);
			mesh->setSide(mesh->getPrevFe(lfe), lfe, mesh->halfedgeSym(mesh->halfedgePrev(lfe)));
			mesh->setSide(rfe, mesh->getNextFe(rfe), mesh->halfedgeNext(rfe));
		}
		mesh->setNeedTopEdge(lfe, false);
		mesh->setNeedTopEdge(rfe, false);
		return 0;
	}
	int doSideDefine() {
		HalfedgeHandle lfe, rfe;
		int i = 0;
		// traverse all front edge
		lfe = getFrontEdgeGroup();
		i = 0;
		lfe = getFrontEdgeGroup();
		do {
			reportIter(i++, "doSideDefine");
			rfe = mesh->getNextFe(lfe);
			if (!mesh->getNeedTopEdge(lfe) && !mesh->getNeedTopEdge(rfe)) {
				continue;
			}
			int retVal = frontEdgeSideDefine(lfe, rfe);
			if (retVal != 0) {
				return -1;
			}

		} while (lfe = mesh->getNextFe(lfe), lfe != getFrontEdgeGroup());
		return 0;
	}
	int doCornerGenerate() {
		int i = 0;
		bool init = true;
		HalfedgeHandle iter = getFrontEdgeGroup();
		do {
			reportIter(i++, "doCornerGenerate");
			HalfedgeHandle lfe = iter;
			HalfedgeHandle rfe = mesh->getNextFe(lfe);
			if (init && mesh->getClass(lfe) / 2 == 1) {
				init = false;
				continue;
			}
			if (!mesh->getNeedTopEdge(lfe)) {
				continue;
			}
			if (mesh->isQuad(mesh->halfedgeFace(rfe))) {
				continue;
			}
			if (mesh->getClass(lfe) % 2 == 1) {
				if (mesh->getClass(rfe) % 2 == 1) { // form a ``|_|`` shape
					HalfedgeHandle
						lfePrev = mesh->getPrevFe(lfe),
						rfeNext = mesh->getNextFe(rfe),
						nnRfe = mesh->getNextFe(rfeNext);
					if (mesh->isQuad(mesh->halfedgeFace(mesh->getPrevFe(lfe))) || mesh->isQuad(mesh->halfedgeFace(mesh->getNextFe(rfe)))) {
						if (globalIter == 115) {
							mesh->highlight({ lfe,rfe });
							mesh->updateDebug();
						}
						generateCorner(lfe, rfe);
						return 1;
					}
					mesh->setNeedTopEdge(lfe, false);
					mesh->setNeedTopEdge(rfe, false);
					mesh->setNeedTopEdge(rfeNext, false);
					mesh->setSide(lfePrev, lfe, NULL);
					mesh->setSide(rfeNext, nnRfe, NULL);
					mesh->setFront(rfeNext, false);
					mesh->setFront(rfe, false);
					mesh->setFront(lfe, false);
					mesh->buildQuad(lfe, rfe, rfeNext);
					HalfedgeHandle newFe = mesh->halfedgeSym(mesh->halfedgeNext(rfeNext));
					mesh->setFront(newFe, true);
					mesh->setPrevFe(nnRfe, newFe);
					mesh->setNextFe(lfePrev, newFe);
					updateHeadFrontEdgeGroup(newFe);

					return 1;
				}
				else { // form a ``|_.. shape
					generateCorner(lfe, rfe);
					return 1;
				}
			}

		} while (iter = mesh->getNextFe(iter), iter != getFrontEdgeGroup());
		return 0;
	}
};

class QMorphOperator : public MeshOperator {
public:
};

class QMorph
{
public:
	QMorph(QMorphOperator* tarMesh);
	int doQMorphProcess();
	int doEdgeRecovery();
	int doClearQuard();
	int doSmooth(int epoch); //when debug, set easySmooth to true
private:
	QMorphOperator* mesh;
	FrontEdgeOperator* frontEdgeOper;
	Smoother smoother;
	

	
	bool proceedNextFeLoop(bool);
	void updateClass(HalfedgeHandle);
	void updateFeClassification(HalfedgeHandle feList = HalfedgeHandle());
	int doSeam();
	int seperateFrontLoop(HalfedgeHandle cutPos);0
};