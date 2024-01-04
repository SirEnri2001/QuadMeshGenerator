#include "mesh.h"
#include <memory>
class MeshHistory;
class MeshRecorder {
	Mesh* mesh;
public:
	MeshRecorder(Mesh* mesh);
	std::unique_ptr<MeshHistory> rootHistory;
	MeshHistory* currentHistory;
	void trackComponent(const Component* component);
	void undo();
	void redo();
	void rollbackComponent(const Component* component);
};