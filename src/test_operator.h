#include "mesh/core.h"
#include "mesh/operator.h"
#include <future>
#include <string>

namespace quadro {
	class TestOperator : public MeshUserOperator {
		int id = 0;
	public:
		std::string integrationTestPath;
		TestOperator(Mesh* mesh);
		void create();
		void proceed();
		void setDisplay(MeshDisplay* display) {
			this->display = display;
		}
		void operator()() override;
		void setId(int i);
		void integrationTest(std::string path);
	};
}