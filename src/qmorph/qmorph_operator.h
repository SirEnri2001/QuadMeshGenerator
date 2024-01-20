#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <memory>
struct FrontEdge;
class ComponentOperator;
class SideDefineOperator;
class FrontEdgeOperator;
class Smoother;
class QMorphDisplay;

class QMorphOperator : public MeshUserOperator {
protected:
public:
	std::shared_ptr<FrontEdgeOperator> feOperator;
	std::shared_ptr<SideDefineOperator> sideOperator;
	std::shared_ptr<ComponentOperator> compOperator;
	Smoother* smoother;
	int doQMorphProcess();
	int doEdgeRecovery();
	int doSmooth(int epoch = 2);
	int doSeam();
	int doSplitFrontEdges();
	void create();
	void test();
	QMorphOperator(Mesh* mesh);
	void operator()() override;
	friend class QMorphDisplay;
};