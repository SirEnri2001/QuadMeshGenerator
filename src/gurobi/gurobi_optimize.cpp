#include "gurobi_c++.h"
#include "gurobi_optimize.h"
#include "../mesh/components.h"

// #include "gurobi_c++.h"
using namespace quadro;

#define PI 3.1415926535897932384

GurobiSolver::GurobiSolver(Mesh* mesh) : MeshUserOperator(mesh) {
    vectorField = mesh->createHalfedgeAttribute<glm::vec3>(glm::vec3(1,0,0));
}

void GurobiSolver::prepare() {

}

float GurobiSolver::getArea(const Face* face) {
    glm::vec3 v1 = glm::vec3(face->getHalfedge()->getNext()->getTarget()->getPosition()
        - face->getHalfedge()->getNext()->getSource()->getPosition());
    glm::vec3 v2 = glm::vec3(face->getHalfedge()->getTarget()->getPosition()
        - face->getHalfedge()->getSource()->getPosition());
    float area = glm::length(glm::cross(v1, -v2)) / 2.0f;
    return area;
}

float GurobiSolver::weight(const Halfedge* he) {
    float edgeLength = glm::length(glm::vec3(he->getTarget()->getPosition() - he->getSource()->getPosition()));
    return 3.0 * edgeLength * edgeLength / getArea(he->getFace()) / getArea(he->getSym()->getFace());
}

void GurobiSolver::optimize() {
    //using namespace std;
    //try {
    //    unique_ptr<MeshAttribute<GRBVar>> uPtrMeshGrbVarAttr 
    //        = mesh->createFaceAttribute<GRBVar>();
    //    MeshAttribute<GRBVar> meshGrbVarAttr = *uPtrMeshGrbVarAttr;
    //    unique_ptr<MeshAttribute<GRBVar>> uPtrTransistionFuncAttr
    //        = mesh->createHalfedgeAttribute<GRBVar>();
    //    MeshAttribute<GRBVar> transitionFuncAttr = *uPtrTransistionFuncAttr;
    //    // Create an environment
    //    GRBEnv env = GRBEnv(true);
    //    env.set("LogFile", "mesh_gurobi_optimize.log");
    //    env.start();

    //    // Create an empty model
    //    GRBModel model = GRBModel(env);

    //    vector<GRBVar> vars;
    //    for (auto& idFace : mesh->getFaces()) {
    //        const Face* f = &idFace.second;
    //        meshGrbVarAttr[f->getMutable()] = 
    //            model.addVar(-PI, PI, 0.0, GRB_CONTINUOUS, string("angle") + to_string(f->getId()));
    //    }
    //    GRBQuadExpr obj;
    //    for (auto& idHalfedge : mesh->getHalfedges()) {
    //        const Halfedge* he = &idHalfedge.second;
    //        if (he->getTarget()->getId() < he->getSource()->getId()) {
    //            continue;
    //        }
    //        transitionFuncAttr[he->getMutable()] = 
    //            model.addVar(0.0, 4.0, 0.0, GRB_INTEGER, string("transition") + to_string(he->getId()));
    //        obj = obj + weight(he) * 
    //    }

    //    // Set objective
    //    model.setObjective(obj, GRB_MINIMIZE);
    //    // Optimize model
    //    model.optimize();

    //    cout << x.get(GRB_StringAttr_VarName) << " "
    //        << x.get(GRB_DoubleAttr_X) << endl;
    //    cout << y.get(GRB_StringAttr_VarName) << " "
    //        << y.get(GRB_DoubleAttr_X) << endl;
    //    cout << z.get(GRB_StringAttr_VarName) << " "
    //        << z.get(GRB_DoubleAttr_X) << endl;

    //    cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

    //}
    //catch (GRBException e) {
    //    cout << "Error code = " << e.getErrorCode() << endl;
    //    cout << e.getMessage() << endl;
    //}
    //catch (...) {
    //    cout << "Exception during optimization" << endl;
    //}

    //return;
}

GurobiSolver::~GurobiSolver() {

}

void GurobiSolver::operator()() {
    display->displayMeshFieldOnEdge(vectorField.get());
}