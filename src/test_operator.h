#include "mesh/mesh.h"
#include "mesh/meshoperator.h"
#include <future>

class TestOperator : public MeshUserOperator {
public:
	MeshDisplay* display;
	TestOperator(Mesh* mesh);
	void create();
	void proceed();
	void setDisplay(MeshDisplay* display) {
		this->display = display;
	}
	void operator()() override;
};