#include "glmeshviewer.h"
#include "mesh/mesh.h"
#include "mesh/meshio.h"
#include "qmorph/qmorph_operator.h"
int main()
{
	//Mesh mesh;
	//MeshIO io(&mesh);
	//QMorphOperator qmorphOperator(&mesh);
	//io.loadM("../test/data/mesh_214370.m");
	//qmorphOperator.create();
	//qmorphOperator.doQMorphProcess();
	//io.writeObj("../111.obj");
	Viewer viewer("Mesh Debugger");
	viewer.mainLoop();
	return 0;
}
