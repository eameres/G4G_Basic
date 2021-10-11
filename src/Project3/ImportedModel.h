#include <vector>

struct objMesh;

class ImportedModel
{
private:
	int numVertices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normalVecs;
public:
	ImportedModel();
	ImportedModel(const char *filePath);
	int getNumVertices();
	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTextureCoords();
	std::vector<glm::vec3> getNormals();
};

struct vertIndices {
	int vi, ti, ni;
};

class ModelImporter
{
private:
	std::vector<glm::vec3> vertVals;
	std::vector<glm::vec3> triangleVerts;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::vec2> stVals;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> normVals;
	std::vector<objMesh> meshes;
	std::map<std::string, unsigned int> textures;
	std::vector<vertIndices> vertIndexList;
public:
	ModelImporter();
	void parseOBJ(const char* filePath);
	void parseMTL(const char* filePath);
	int getNumVertices();
	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTextureCoordinates();
	std::vector<glm::vec3> getNormals();
	std::vector<objMesh> getMeshes();
};