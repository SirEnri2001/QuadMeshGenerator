#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <glm.hpp>


class Smoother
{
	Mesh* mesh;
public:
	//glm::vec3 getDelC(VertexHandle Ni, HalfedgeHandle ife, HalfedgeHandle ofe);
	//void boundaryEdgeSmooth(HalfedgeHandle ofe);
	//void Smoother::boundaryEdgeSmooth(HalfedgeHandle ife, HalfedgeHandle ofe);
	void triangleInteriorSmooth(const Vertex* vertex, bool); // should eliminate overlap triangle around the vertex
	//void quadriInteriorSmooth(VertexHandle);
	//void doTriangleSmooth(int epoch = 3);
	//int doBoundarySmooth(HalfedgeHandle bhe, int epoch = 1);
	Smoother(Mesh* mesh);
};

