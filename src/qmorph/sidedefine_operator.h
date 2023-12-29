#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <memory>
#include <unordered_map>
class ComponentOperator;
class FrontEdgeOperator;
struct FrontEdge;

typedef enum SideDefineResult {
	Succeeded,
	NoSuitable,
	FrontEdgeContact,
	FrontEdgeContactDegenerate,
	SideEdgeContact,
	QuadContactByVertex,
	QuadContactByEdge
} SideDefineResult;
class SideDefineOperator : public MeshOperator {
	std::shared_ptr<ComponentOperator> compOperator;
	std::shared_ptr<FrontEdgeOperator> feOperator;
	std::unordered_map<const Vertex*, int> sideCount;
	std::unordered_map<const Halfedge*, bool> isHeSide;
	std::unordered_map<const FrontEdge*, const Halfedge*> leftSideEdges;
	std::unordered_map<const FrontEdge*, const Halfedge*> rightSideEdges;
	const double constEpsilon = 10.0;
public:
	SideDefineOperator(Mesh* mesh);
	void create(std::shared_ptr<ComponentOperator>& compOper, std::shared_ptr<FrontEdgeOperator>& feOper);
	bool isSide(const Vertex* v);
	bool isSide(const Halfedge* he);
	void setSide(const Halfedge* he, bool val);
	const Halfedge* getLeftSide(const FrontEdge* fe);
	const Halfedge* getRightSide(const FrontEdge* fe);
	void setSide(const FrontEdge* lfe, const Halfedge* rightSideForLfe);
	SideDefineResult verticalSideSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide);
	SideDefineResult horizontalSideSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide);
	SideDefineResult verticalSideSplitSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide);
	SideDefineResult horizontalSideSplitSeek(FrontEdge* lfe, FrontEdge* rfe, const Halfedge*& resultUpSide);
	int frontEdgeSideDefine(FrontEdge* lfe, FrontEdge* rfe);
	int generateCorner(FrontEdge* lfe, FrontEdge* rfe);
	int doSideDefine();
	int doCornerGenerate();

};
