#include "recorder.h"
#include "components.h"
using namespace quadro;

//class MeshHistory {
//public:
//	struct ComponentHistory
//	{
//		Vertex vertex;
//		Halfedge halfedge;
//		Face face;
//	} componentHistory;
//	enum ComponentType {
//		VERTEX, HALFEDGE, FACE
//	} componentType;
//	MeshHistory* lastHistory;
//	std::unique_ptr<MeshHistory> nextHistory;
//};
//
//MeshRecorder::MeshRecorder(Mesh* mesh) :mesh(mesh) {
//	rootHistory = std::make_unique<MeshHistory>();
//	rootHistory->lastHistory = nullptr;
//	currentHistory = rootHistory.get();
//}
//
//void MeshRecorder::trackComponent(const Component* component) {
//	std::unique_ptr<MeshHistory> nextHistory = std::make_unique<MeshHistory>();
//	if (static_cast<const Vertex*>(component)) {
//		nextHistory->componentHistory.vertex = *static_cast<const Vertex*>(component);
//		nextHistory->componentType = MeshHistory::VERTEX;
//	}
//	if (static_cast<const Face*>(component)) {
//		nextHistory->componentHistory.face = *static_cast<const Face*>(component);
//		nextHistory->componentType = MeshHistory::FACE;
//	}
//	if (static_cast<const Halfedge*>(component)) {
//		nextHistory->componentHistory.halfedge = *static_cast<const Halfedge*>(component);
//		nextHistory->componentType = MeshHistory::HALFEDGE;
//	}
//	nextHistory->lastHistory = currentHistory;
//	currentHistory->nextHistory = std::move(nextHistory);
//	currentHistory = currentHistory->nextHistory.get();
//}
//
//void MeshRecorder::undo() {
//	switch (currentHistory->componentType)
//	{
//	case MeshHistory::VERTEX:
//		break;
//	case MeshHistory::FACE:
//		break;
//	case MeshHistory::HALFEDGE:
//		break;
//	default:
//		break;
//	}
//}