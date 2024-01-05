#include "mesh/mesh.h"
#include "mesh/meshoperator.h"
#include "qmorph/qmorph_operator.h"
#include <future>

class TestOperator : public QMorphOperator {
	int id = 0;
public:
	MeshDisplay* display;
	TestOperator(Mesh* mesh);
	void create();
	void proceed();
	void setDisplay(MeshDisplay* display) {
		this->display = display;
	}
	void operator()() override;
	void setId(int i);
};