#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <memory>
struct FrontEdge;
class ComponentOperator;
class SideDefineOperator;
class FrontEdgeOperator;
class Smoother;

class QMorphOperator : public MeshUserOperator {
	std::shared_ptr<FrontEdgeOperator> feOperator;
	std::shared_ptr<SideDefineOperator> sideOperator;
	std::shared_ptr<ComponentOperator> compOperator;
	Smoother* smoother;
public:
	int doQMorphProcess();
	int doEdgeRecovery();
	int doSmooth(int epoch = 2);
	int doSeam();
	void create();
	QMorphOperator(Mesh* mesh);
	void operator()() override;
};