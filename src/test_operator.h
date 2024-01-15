#include "mesh/mesh.h"
#include "mesh/meshoperator.h"
#include "qmorph/qmorph_operator.h"
#include <future>
#include <string>

class TestOperator : public MeshUserOperator {
	int id = 0;
public:
	std::string integrationTestPath;
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
	void integrationTest(std::string path);
};