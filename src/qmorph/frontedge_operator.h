#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <memory>
#include <unordered_map>
struct FrontEdge;
class ComponentOperator;
class SideDefineOperator;
class FrontEdgeOperator : public MeshOperator {
	std::unordered_map<const Halfedge*, FrontEdge> heToFe;
	std::unordered_map<const Vertex*, int> frontEdgeCount;
	std::list<FrontEdge*> frontEdgeGroups;
	std::shared_ptr<ComponentOperator> compOperator;
	std::shared_ptr<SideDefineOperator> sideOperator;
	FrontEdge* getNextFe(FrontEdge* fe);
	const FrontEdge* getNextFe(const FrontEdge* fe);
	FrontEdge* getPrevFe(FrontEdge* fe);
	const FrontEdge* getPrevFe(const FrontEdge* fe);
	void create();
public:
	FrontEdgeOperator(Mesh* mesh);
	void create(std::shared_ptr<ComponentOperator>& compOper, std::shared_ptr<SideDefineOperator>& sideOper);
	FrontEdge* getFront(const Halfedge* he);
	void updateFeClassification();
	FrontEdge* setFront(const Halfedge* he, bool val);
	void setNextFe(FrontEdge* fe, FrontEdge* feNext);
	int countFeToFe(FrontEdge* fe1, FrontEdge* fe2);
	int initFrontEdgeGroup();
	bool isFront(const Halfedge* he);
	bool isFront(const Vertex* v);
	int frontEdgeGroupSize(FrontEdge* fe);
	void pushHeadFrontEdgeGroup(FrontEdge* fe);
	void pushTailFrontEdgeGroup(FrontEdge* fe);
	// param he: any frontedge in the group
	void removeFrontEdgeGroup(FrontEdge* fe);
	bool isFrontEdgeGroupIndex(FrontEdge* he);
	FrontEdge* popFrontEdgeGroup();
	FrontEdge* getFrontEdgeGroup();
	bool switchFrontEdgeGroup();
	void updateHeadFrontEdgeGroup(FrontEdge* he);
	int seperateFrontLoop(const Halfedge* cutPos);
	bool proceedNextFeLoop(bool reclasssify = true);
	const Halfedge* edgeRecovery(Vertex*, Vertex*); //return a CTHEdgeHandle* source former param and target latter
	std::list<const Halfedge*>* calculateRambdaSet(const Vertex*, const Vertex*);
	const Vertex* mergeEdge(Vertex* va, Vertex* vb);
};