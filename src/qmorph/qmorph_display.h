#include "../mesh/meshdisplay.h"
#include "qmorph_operator.h"

class QMorphDisplay {
	Mesh* mesh;
protected:
	QMorphOperator* qmorphOperator;
	MeshDisplay* display;
public:
	QMorphDisplay(QMorphOperator* oper, MeshDisplay* display);
	void markFrontEdges();
	void markFrontEdgeClass();
};