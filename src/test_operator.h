#include "mesh/mesh.h"
#include "mesh/meshoperator.h"
#include "qmorph/qmorph_operator.h"
#include <future>

class TestOperator : public MeshUserOperator {
	int id = 0;
public:
	QMorphOperator* qmorphOperator;
	QMorphDisplay* display;
	TestOperator(Mesh* mesh);
	void create();
	void proceed();
	void setDisplay(QMorphDisplay* display) {
		this->display = display;
	}
	void operator()() override;
	void setId(int i);
};