#pragma once
#include "components.h"

namespace quadro {
	/**
	* MeshDisplay is an interface for MeshOperator to display mesh & components for debugging purpose. 
	* It gives the data for OpenGL buffers.
	*/
	class MeshDisplay {
	protected:
		/**
		* Mesh to be displayed
		*/
		Mesh* mesh;
		std::unique_ptr<MeshAttribute<glm::vec4>> vertexColor;
		std::unique_ptr<MeshAttribute<std::pair<bool, glm::vec4>>> vertexMark;
	public:
		std::vector<glm::vec4> vertexBuffer;
		std::vector<ID> indices;
		std::unordered_map<ID, int> vertexBufferOffset;

		std::vector<glm::vec4> frameVertexBuffer;
		std::vector<ID> frameIndices;

		std::vector<glm::vec4> heSelect;
		std::vector<ID> heSelectIndices;
		std::vector<glm::vec4> pointScatter;
		std::vector<ID> pointIndices;

		int markCount = 0;
		float vertexSize = 5.0f;
		float edgeSize = 3.0f;
		glm::vec3 calculateSurfaceNormal(const Face* face);
		glm::vec3 calculateVertexNormal(const Vertex* vertex);
		glm::mat4 modelMat;
		float getLinePointDistance(glm::vec4 point, glm::vec4 lineDirection, glm::vec4 pointOnLine);

		MeshDisplay(Mesh* mesh);
		void create();
		void createFrame();
		void markHalfedge(const Halfedge* he,
			glm::vec4 sourceColor = glm::vec4(0, 0, 1, 1),
			glm::vec4 targetColor = glm::vec4(1, 0, 0, 1));
		void markPoint(glm::vec4 pos, glm::vec4 normal = glm::vec4(0, 1, 0, 0));
		void markFace(const Face* face, glm::vec4 color = glm::vec4(1, 0.5, 0.5, 1));
		void markBoundaries();
		void markVertex(const Vertex* vertex);
		void markHalfedgeCycle(const Halfedge* he);
		const Vertex* selectVertex(glm::vec4 eye, glm::vec4 ray);
		const Halfedge* selectHalfedge(glm::vec4 eye, glm::vec4 ray);
		float getNormalizedScale();

		void setMarked(Vertex* v, bool mark);
		void setMarkColor(Vertex* v, glm::vec4 color);
		void setColor(Vertex* v, glm::vec4 color);

		bool getMarked(const Vertex* v) const;
		glm::vec4 getMarkColor(const Vertex* v) const;
		glm::vec4 getColor(const Vertex* v) const;

		void drawVertexAttribute(const MeshAttribute<glm::vec4>* attribute);
		void markVertexAttribute(const MeshAttribute<std::pair<bool, glm::vec4>>* attribute);
		void markVertexAttribute(const MeshAttribute<bool>* attribute);
	};
}
