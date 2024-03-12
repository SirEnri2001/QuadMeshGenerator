#include "../mesh/meshoperator.h"

class MorseFunction : public MeshUserOperator {
public:
	void extractMorseFunction();
	void paintMorseFunction();

	void extractWaveFunction();
	void paintWaveFunction();
};