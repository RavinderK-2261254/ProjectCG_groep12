#include <iostream>
#include <string> 
#include <vector>
#include "Texture.h" 
#include "Mesh.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace std;
class Model {
public:
	Model(char* path);
	void Draw(Shader& Shader);
private:
	vector <Mesh> meshes;
	std::string directory;
	void loadModel(std::string const& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
		string typeName);
};