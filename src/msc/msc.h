#include "../mesh/meshoperator.h"

class MorseFunction : public MeshUserOperator {
public:
	MorseFunction(Mesh* mesh);
	void extractMorseFunction();
	void paintMorseFunction();

	void extractWaveFunction();
	void paintWaveFunction();

	void operator()();
};