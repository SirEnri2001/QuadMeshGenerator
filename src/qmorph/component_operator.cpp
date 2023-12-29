#include "component_operator.h"
#include "../mesh/meshcomponents.h"

//             v1
//            /  \
//           /    \
//          /      \
//         /        \
//--bhe1->va-oldEdge->vb--bhe2->
//         \        /
//          \      /
//           \    /
//            \  /
//             v2
// Only allowed in triangular mesh
const Vertex* ComponentOperator::splitEdge(const Halfedge* coldHe, glm::vec4 pos)
{
	Halfedge* oldHe = const_cast<Halfedge*>(coldHe);
	Halfedge* oldEdge = oldHe;
	Vertex* newVertex = mesh->createVertex();
	newVertex->setPosition(pos);
	const Vertex* va = oldHe->getSource();
	const Vertex* vb = oldHe->getTarget();
	const Vertex* v1 = oldHe->getNext()->getTarget();
	const Vertex* v2 = oldHe->getSym()->getNext()->getTarget();
	bool isBoundary0 = oldHe->isBoundary();
	bool isBoundary1 = oldHe->getSym()->isBoundary();
	Halfedge* bhe1 = NULL, * bhe2 = NULL;

	if (isBoundary0) {
		bhe1 = oldHe->getPrev();
		bhe2 = oldHe->getNext();
	}
	if (isBoundary1) {
		bhe1 = oldHe->getSym()->getNext()->getSym();
		bhe2 = oldHe->getSym()->getPrev()->getSym();
	}

	if (!isBoundary0) {
		mesh->deleteFace(oldHe->getFace());
	}
	if (!isBoundary1) {
		mesh->deleteFace(oldHe->getSym()->getFace());
	}
	return nullptr;
	//mesh->setFace(oldHe, NULL);
	//setFace(halfedgeSym(oldHe), NULL);
	//unsetHalfedge(halfedgeVertex(oldHe), oldHe);
	//unsetHalfedge(halfedgeVertex(halfedgeSym(oldHe)), halfedgeSym(oldHe));
	//disconnect(oldEdge);
	//deleteEdge(oldEdge);
	//createEdge(va, newVertex);
	//createEdge(newVertex, vb);
	//if (!isBoundary0) {
	//	createFace(newVertex, vb, v1);
	//	createFace(va, newVertex, v1);
	//}
	//if (!isBoundary1) {
	//	createFace(newVertex, va, v2);
	//	createFace(vb, newVertex, v2);
	//}

	//if (isBoundary0) {
	//	setNextHalfedge(bhe1, vertexHalfedge(va, newVertex));
	//	setNextHalfedge(vertexHalfedge(va, newVertex), vertexHalfedge(newVertex, vb));
	//	setNextHalfedge(vertexHalfedge(newVertex, vb), bhe2);
	//}

	//if (isBoundary1) {
	//	setPrevHalfedge(halfedgeSym(bhe1), vertexHalfedge(newVertex, va));
	//	setPrevHalfedge(vertexHalfedge(newVertex, va), vertexHalfedge(vb, newVertex));
	//	setPrevHalfedge(vertexHalfedge(vb, newVertex), halfedgeSym(bhe2));
	//}
	//return newVertex;
}



//EdgeHandle CToolMesh::swapEdge(EdgeHandle oldEdge)
//{
//	bool debug = false;
//	topology_assert(!isBoundary(oldEdge), { edgeHalfedge(oldEdge, 0) });
//	HalfedgeHandle he1 = halfedge_handle(oldEdge, 0);
//	HalfedgeHandle he2 = halfedge_handle(oldEdge, 1);
//
//	VertexHandle va = halfedgeSource(he1);
//	VertexHandle vb = halfedgeTarget(he1);
//	VertexHandle v1 = halfedgeTarget(halfedgeNext(he1));
//	VertexHandle v2 = halfedgeTarget(halfedgeNext(he2));
//	deleteFace(edgeFace1(oldEdge));
//	deleteFace(edgeFace2(oldEdge));
//	setFace(edgeHalfedge(oldEdge, 0), NULL);
//	setFace(edgeHalfedge(oldEdge, 1), NULL);
//	unsetHalfedge(halfedgeVertex(edgeHalfedge(oldEdge, 0)), edgeHalfedge(oldEdge, 0));
//	unsetHalfedge(halfedgeVertex(edgeHalfedge(oldEdge, 1)), edgeHalfedge(oldEdge, 1));
//	disconnect(oldEdge);
//	deleteEdge(oldEdge);
//	createEdge(v1, v2);
//	createFace(v1, va, v2);
//	FaceHandle face = createFace(v2, vb, v1);
//	he1 = halfedge_handle(face);
//	if (halfedgeSource(he1) == v2)
//		he1 = halfedgePrev(he1);
//	if (halfedgeTarget(he1) == v1)
//		he1 = halfedgeNext(he1);
//	return halfedgeEdge(he1);
//}