#pragma once
#include "components.h"
#include <string>
namespace quadro {
	/**
	* MeshIO reads and writes mesh files, supported format M & OBJ
	*/
	class MeshIO {
		Mesh* mesh;
	public:
		/**
		* Constructor.
		*/
		MeshIO(Mesh* mesh);
		/**
		* Destructor.
		*/
		~MeshIO();
		/**
		* Load obj file from given filename.
		* @param filename Relevant or absolute directory
		* @return Successfully loaded obj file
		*/
		bool loadObj(const std::string& filename);
		/**
		* Load m file from given filename.
		* @param filename Relevant or absolute directory
		* @return Successfully loaded m file
		*/
		bool loadM(const std::string& filename);
		/**
		* Write m file from given filename.
		* @param filename Relevant or absolute directory
		* @return Successfully wrote m file
		*/
		bool writeM(const std::string& filename);
		/**
		* Write obj file from given filename.
		* @param filename Relevant or absolute directory
		* @return Successfully wrote obj file
		*/
		bool writeObj(const std::string& filename);
	};
}