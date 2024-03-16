#include "meshattribute.h"
#include "meshcomponents.h"

BaseMeshAttribute::BaseMeshAttribute(Mesh* mesh, AttributeType type) 
	: mesh(mesh), type(type) { }

BaseMeshAttribute::~BaseMeshAttribute() {}