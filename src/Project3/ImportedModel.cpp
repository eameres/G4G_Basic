#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_opengl3.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <filesystem>

#include "renderer.h"
#include "ImportedModel.h"

using namespace std;

ImportedModel::ImportedModel() {}

ImportedModel::ImportedModel(const char* filePath) {

}

int ImportedModel::getNumVertices() { return numVertices; }
std::vector<glm::vec3> ImportedModel::getVertices() { return vertices; }
std::vector<glm::vec2> ImportedModel::getTextureCoords() { return texCoords; }
std::vector<glm::vec3> ImportedModel::getNormals() { return normalVecs; }

// ---------------------------------------------------------------

ModelImporter::ModelImporter() {}

ObjModel::ObjModel(const char* filePath, Material* material, glm::mat4 m)
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    modelMatrix = m;

    myMaterial = material;

    glGenVertexArrays(1, &VAO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glGenBuffers(numVBOs = 2, VBO);
    glGenBuffers(1, &EBO);

    ModelImporter modelImporter = ModelImporter();
    Renderer::name = filePath;
    modelImporter.parseOBJ(filePath);

    std::vector<float> verts = modelImporter.getVertices();
    std::vector<float> tcs = modelImporter.getTextureCoordinates();
    std::vector<float> normals = modelImporter.getNormals();

    std::vector<float> vbovalues;
    std::vector<int> indices;

    for (int i = 0; i < modelImporter.getNumVertices(); i++) {
        vbovalues.push_back(verts[i * 3]);
        vbovalues.push_back(verts[i * 3 + 1]);
        vbovalues.push_back(verts[i * 3 + 2]);

        vbovalues.push_back(normals[i * 3]);
        vbovalues.push_back(normals[i * 3 + 1]);
        vbovalues.push_back(normals[i * 3 + 2]);

        vbovalues.push_back(tcs[i * 2]);
        vbovalues.push_back(tcs[i * 2 + 1]);

        indices.push_back(i);
    }

    for (objMesh temp : modelImporter.getMeshes())
        meshes.push_back(temp);

    indexCount = indices.size();

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vbovalues.size() * 4, &vbovalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * 4, &indices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal vector attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // skip color for now since its in another VBO
    // 
    // texture coord attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    setupColorAttrib();

    glBindVertexArray(0);
};

void ObjModel::render(glm::mat4 treeMat, glm::mat4 vpMat, double deltaTime, SceneGraph* sg) {

    glm::mat4 mvp;
    unsigned int shaderID;

    if (!enabled) return;

    //assert(myMaterial != NULL);

    if (myMaterial == NULL)
        myMaterial = Material::materials["green"];

    glm::vec3 lightLoc = sg->light.position;
    glm::vec3 cameraLoc = sg->camera.position;

    shaderID = myMaterial->use(sg->renderPass);

    glUniform1f(glGetUniformLocation(shaderID, "myTime"), elapsedTime += (float)deltaTime);

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "v"), 1, GL_FALSE, glm::value_ptr(treeMat));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "p"), 1, GL_FALSE, glm::value_ptr(vpMat));

    // because it is only once per model, another approach might be just to pre-multiply model, view and perspective 
    mvp = vpMat * treeMat * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    glUniform3fv(glGetUniformLocation(shaderID, "cPos"), 1, glm::value_ptr(cameraLoc));
    glUniform3fv(glGetUniformLocation(shaderID, "lPos"), 1, glm::value_ptr(lightLoc));

    glm::mat4 lightViewProjection = sg->light.projection() * glm::lookAt(sg->light.position, sg->light.target, sg->light.up);

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightViewProjection));

    glBindVertexArray(VAO);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (meshes.size() == 0) {
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instances);
    } else {
        for (int i = 0; i < meshes.size(); i++) {

            unsigned int shaderID;

            shaderID = Material::materials[meshes[i].myName]->use(sg->renderPass);

            glm::mat4 mvp;

            if (!enabled) return;


            glm::vec3 lightLoc = sg->light.position;
            glm::vec3 cameraLoc = sg->camera.position;

            glUniform1f(glGetUniformLocation(shaderID, "myTime"), elapsedTime += (float)deltaTime);

            glUniformMatrix4fv(glGetUniformLocation(shaderID, "m"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "v"), 1, GL_FALSE, glm::value_ptr(treeMat));
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "p"), 1, GL_FALSE, glm::value_ptr(vpMat));

            // because it is only once per model, another approach might be just to pre-multiply model, view and perspective 
            mvp = vpMat * treeMat * modelMatrix;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

            glUniform3fv(glGetUniformLocation(shaderID, "cPos"), 1, glm::value_ptr(cameraLoc));
            glUniform3fv(glGetUniformLocation(shaderID, "lPos"), 1, glm::value_ptr(lightLoc));

            glm::mat4 lightViewProjection = sg->light.projection() * glm::lookAt(sg->light.position, sg->light.target, sg->light.up);

            glUniformMatrix4fv(glGetUniformLocation(shaderID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightViewProjection));

            glBindVertexArray(VAO);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);

            unsigned int endingVert = indexCount;

            if ((i+1) < meshes.size())
                endingVert = meshes[i + 1].startingVert;

            glDrawElementsInstanced(GL_TRIANGLES, endingVert - meshes[i].startingVert, GL_UNSIGNED_INT, (void*)(meshes[i].startingVert * sizeof(unsigned int)), instances);
        }
    }
}

#include "textures.h"

using std::string;

static string getPathName(const string& s) {

   char sep = '/';

   size_t i = s.rfind(sep, s.length());
   if (i != string::npos) {
      return(s.substr(0, i) + "/");
   }

   sep = '\\';

   i = s.rfind(sep, s.length());
   if (i != string::npos) {
       return(s.substr(0, i) + "/");
   }

   return("");
}

void ModelImporter::parseMTL(const char* filePath) {
    int count = 0;
    ifstream fileStream(filePath, ios::in);
    string line = "";
    bool activeMtl = false;
    string newmtl, mtlName;
    string attrib,val,texName;
    unsigned int tNum = 0;
    bool textured = false;
    glm::vec4 mtlColor;

    if (fileStream.good()) {
        while (!fileStream.eof()) {
            getline(fileStream, line);
            if (line.compare(0, 6, "newmtl") == 0){
                if (activeMtl) {
                    if (textured)
                        new Material(Shader::shaders["textured"], mtlName, tNum, 4, true);
                    else
                        new Material(Shader::shaders["base"], mtlName, -1, mtlColor);
                }
                std::istringstream some_stream(line);
                some_stream >> newmtl >> mtlName;
                activeMtl = true;
                textured = false;
            }
            else {
                std::istringstream some_stream(line);
                some_stream >> attrib;
                if ((attrib == "map_Kd") || (attrib == "map_Ka")){
                    some_stream >> val;
                    texName = val;
                    string tName = getPathName(filePath) + val;
                    tNum = loadTexture(tName.c_str());
                    textured = true;
                }else if (attrib == "Kd") {
                    float r, g, b;
                    some_stream >> r >> g >> b;
                    mtlColor = glm::vec4(r,g,b,1.0);
                }
            }
        }
        if (textured)
            new Material(Shader::shaders["textured"], mtlName, tNum, 4, true);
        else
            new Material(Shader::shaders["base"], mtlName, -1, mtlColor);
    }
}

void ModelImporter::parseOBJ(const char* filePath) {
    float x, y, z;
    string content;
    ifstream fileStream(filePath, ios::in);
    string line = "";
    while (!fileStream.eof()) {
        getline(fileStream, line);
        if (line.compare(0, 6, "mtllib") == 0){
            string attrib,fname;
            std::istringstream some_stream(line);
            some_stream >> attrib >> fname;
            fname = getPathName(filePath) + fname;
            parseMTL(fname.c_str());
        }
        if (line.compare(0, 2, "us") == 0) {
            objMesh temp;
            string action;
            std::istringstream some_stream(line);
            some_stream >> action >> temp.myName;

            std::cout << action << " " << temp.myName << " " << triangleVerts.size() <<  "\n";
            temp.startingVert = triangleVerts.size()/3; // number of vertices (coordinate triples), not the individual float count!!
            meshes.push_back(temp);

        }
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
std::vector<objMesh> ModelImporter::getMeshes() { return meshes; }
