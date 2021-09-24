#include <glm/glm.hpp>

#include <fstream>
#include <sstream>

#include "ImportedModel.h"

using namespace std;

ImportedModel::ImportedModel() {}

ImportedModel::ImportedModel(const char* filePath) {
	ModelImporter modelImporter = ModelImporter();
	modelImporter.parseOBJ(filePath);
	numVertices = modelImporter.getNumVertices();
	std::vector<float> verts = modelImporter.getVertices();
	std::vector<float> tcs = modelImporter.getTextureCoordinates();
	std::vector<float> normals = modelImporter.getNormals();

	for (int i = 0; i < numVertices; i++) {
		vertices.push_back(glm::vec3(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]));
		texCoords.push_back(glm::vec2(tcs[i * 2], tcs[i * 2 + 1]));
		normalVecs.push_back(glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]));
	}
}

int ImportedModel::getNumVertices() { return numVertices; }
std::vector<glm::vec3> ImportedModel::getVertices() { return vertices; }
std::vector<glm::vec2> ImportedModel::getTextureCoords() { return texCoords; }
std::vector<glm::vec3> ImportedModel::getNormals() { return normalVecs; }

// ---------------------------------------------------------------

ModelImporter::ModelImporter() {}

void ModelImporter::parseOBJ(const char* filePath) {
    float x, y, z;
    string content;
    ifstream fileStream(filePath, ios::in);
    string line = "";
    while (!fileStream.eof()) {
        getline(fileStream, line);
        if (line.compare(0, 2, "v ") == 0) {
            stringstream ss(line.erase(0, 1));
            ss >> x; ss >> y; ss >> z;
            vertVals.push_back(x);
            vertVals.push_back(y);
            vertVals.push_back(z);
        }
        if (line.compare(0, 2, "vt") == 0) {
            stringstream ss(line.erase(0, 2));
            ss >> x; ss >> y;
            stVals.push_back(x);
            stVals.push_back(y);
        }
        if (line.compare(0, 2, "vn") == 0) {
            stringstream ss(line.erase(0, 2));
            ss >> x; ss >> y; ss >> z;
            normVals.push_back(x);
            normVals.push_back(y);
            normVals.push_back(z);
        }
        if (line.compare(0, 2, "f ") == 0) {
            string oneCorner, v, t, n;
            stringstream ss(line.erase(0, 2));
            int vi[4],ti[4],ni[4];
            for (int i = 0; i < 4; i++) {
                //int vi, ti, ni;
                vi[i] = ti[i] = ni[i] = 0;
                while (ss.peek() == ' ') // skip spaces
                    ss.get();

                getline(ss, oneCorner, ' ');

                if (!ss)
                    break;
                if (sscanf(oneCorner.c_str(), "%i/%i/%i", &vi[i], &ti[i], &ni[i]) != 3) {
                    sscanf(oneCorner.c_str(), "%i//%i", &vi[i], &ni[i]);
                }

                if (ti[i] < 1) {
                    if (stVals.size() / 2 >= vi[i])
                        ti[i] = vi[i];
                }

                if (ni[i] < 1) {
                    if (normVals.size() / 3 >= vi[i])
                        ni[i] = vi[i];
                }

                
                if (i > 2){
                    for (int j = 1; j < 3; j++){
                        
                        int vertRef = (vi[j] - 1) * 3;
                        int tcRef = (ti[j] - 1) * 2;
                        int normRef = (ni[j] - 1) * 3;
                        
                        if (vertRef > -1) {
                            triangleVerts.push_back(vertVals[vertRef]);
                            triangleVerts.push_back(vertVals[vertRef + 1]);
                            triangleVerts.push_back(vertVals[vertRef + 2]);
                        }
                        else {
                            triangleVerts.push_back(0.0f);
                            triangleVerts.push_back(0.0f);
                            triangleVerts.push_back(0.0f);
                        }

                        if (tcRef > -1) {
                            textureCoords.push_back(stVals[tcRef]);
                            textureCoords.push_back(stVals[tcRef + 1]);
                        }
                        else {
                            textureCoords.push_back(0.0f);
                            textureCoords.push_back(0.0f);
                        }

                        if (normRef > -1) {
                            normals.push_back(normVals[normRef]);
                            normals.push_back(normVals[normRef + 1]);
                            normals.push_back(normVals[normRef + 2]);
                        }
                        else {
                            normals.push_back(0.0f);
                            normals.push_back(0.0f);
                            normals.push_back(0.0f);
                        }
                    }
                }

                int vertRef = (vi[i] - 1) * 3;
                int tcRef = (ti[i] - 1) * 2;
                int normRef = (ni[i] - 1) * 3;
                
                if (vertRef > -1) {
                    triangleVerts.push_back(vertVals[vertRef]);
                    triangleVerts.push_back(vertVals[vertRef + 1]);
                    triangleVerts.push_back(vertVals[vertRef + 2]);
                }
                else {
                    triangleVerts.push_back(0.0f);
                    triangleVerts.push_back(0.0f);
                    triangleVerts.push_back(0.0f);
                }

                if (tcRef > -1) {
                    textureCoords.push_back(stVals[tcRef]);
                    textureCoords.push_back(stVals[tcRef + 1]);
                }
                else {
                    textureCoords.push_back(0.0f);
                    textureCoords.push_back(0.0f);
                }

                if (normRef > -1) {
                    normals.push_back(normVals[normRef]);
                    normals.push_back(normVals[normRef + 1]);
                    normals.push_back(normVals[normRef + 2]);
                }
                else {
                    normals.push_back(0.0f);
                    normals.push_back(0.0f);
                    normals.push_back(0.0f);
                }
            }
        }
    }
}
int ModelImporter::getNumVertices() { return (triangleVerts.size() / 3); }
std::vector<float> ModelImporter::getVertices() { return triangleVerts; }
std::vector<float> ModelImporter::getTextureCoordinates() { return textureCoords; }
std::vector<float> ModelImporter::getNormals() { return normals; }
