#include "qmorph_display.h"
#include "frontedge.h"
#include "frontedge_operator.h"
#include "sidedefine_operator.h"
#include "component_operator.h"
#include "../mesh/meshcomponents.h"
#include "smoother.h"

QMorphDisplay::QMorphDisplay(QMorphOperator* oper, MeshDisplay* display):display(display), qmorphOperator(oper), mesh(oper->mesh) {

}

void QMorphDisplay::markFrontEdges() {
	for (auto& fe : qmorphOperator->feOperator->frontEdgeGroups) {
		const FrontEdge* fe1 = fe;
		do {
			display->markHalfedge(fe1->he);
		} while (fe1 = fe1->getNextFe(), fe1 != fe);
	}
}

void QMorphDisplay::markFrontEdgeClass() {
	for (auto& fe : qmorphOperator->feOperator->frontEdgeGroups) {
		const FrontEdge* fe1 = fe;
		do {
			glm::vec4 sourceColor = fe1->getPrevFe()->rightCornerSharp?glm::vec4(1,1,0,1): glm::vec4(0, 1, 1, 1);
			glm::vec4 targetColor = fe1->rightCornerSharp ? glm::vec4(1, 1, 0, 1) : glm::vec4(0, 1, 1, 1);
			display->markHalfedge(fe1->he, sourceColor, targetColor);
		} while (fe1 = fe1->getNextFe(), fe1 != fe);
	}
}