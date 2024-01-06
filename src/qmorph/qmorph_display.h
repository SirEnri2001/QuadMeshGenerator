#include "../mesh/meshdisplay.h"
#include "qmorph_operator.h"

class QMorphDisplay {
	Mesh* mesh;
protected:
public:
	QMorphOperator* qmorphOperator;
	MeshDisplay* display;
	QMorphDisplay(QMorphOperator* oper, MeshDisplay* display);
	void markFrontEdges();
	void markFrontEdgeClass();
};