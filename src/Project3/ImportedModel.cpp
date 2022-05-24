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
using namespace glm;

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

    const std::vector<vec3>& verts = modelImporter.getVertices();
    const std::vector<vec2>& tcs = modelImporter.getTextureCoordinates();
    const std::vector<vec3>& normals = modelImporter.getNormals();

    std::vector<float> vbovalues;
    std::vector<int> indices;

    for (int i = 0; i < modelImporter.getNumVertices(); i++) {
        vbovalues.push_back(verts[i].x);
        vbovalues.push_back(verts[i].y);
        vbovalues.push_back(verts[i].z);

        vbovalues.push_back(normals[i].x);
        vbovalues.push_back(normals[i].y);
        vbovalues.push_back(normals[i].z);

        vbovalues.push_back(tcs[i].x);
        vbovalues.push_back(tcs[i].y);

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
    string mtlName;
    string attrib,val;
    unsigned int tNum = 0;
    bool textured = false;
    glm::vec4 mtlColor;

    if (fileStream.good()) {
        std::cout << "loading material descriptions from " << filePath << "\n";
        while (!fileStream.eof()) {
            getline(fileStream, line);
            std::istringstream nStream;
            nStream.clear();
            nStream.str(line);
            nStream >> attrib;

            if (attrib == "newmtl"){
                if (activeMtl) {
                    if (textured)
                        new Material(Shader::shaders["textured"], mtlName, tNum, 4, true);
                    else
                        new Material(Shader::shaders["PhongShadowed"], mtlName, -1, mtlColor);
                }
                nStream >> mtlName;
                activeMtl = true;
                textured = false;
            }else {
                if ((attrib == "map_Kd") || (attrib == "map_Ka")){
                    nStream >> val;
                    if (val.compare( 0, 1, "-") == 0) {
                        nStream >> val;
                        nStream >> val;
                    }
                    string tName = getPathName(filePath) + val;

                    if (textures[val] == NULL) {
                        std::cout << "loading " << attrib << " texture : " << tName << "\n";
                        tNum = loadTexture(tName.c_str());
                        textures[val] = tNum;
                    }
                    else {
                        std::cout << "already loaded " << attrib << " texture : " << tName << "\n";
                        tNum = textures[val];
                    }
                    textured = true;
                }else if (attrib == "Kd") {
                    float r, g, b;
                    nStream >> r >> g >> b;
                    mtlColor = glm::vec4(r,g,b,1.0);
                }
            }
        }
        if (textured)
            new Material(Shader::shaders["textured"], mtlName, tNum, 4, true);
        else
            new Material(Shader::shaders["PhongShadowed"], mtlName, -1, mtlColor);
    }
}

void ModelImporter::parseOBJ(const char* filePath) {
    float x, y, z;
    string content;
    ifstream fileStream(filePath, ios::in);
    string line = "";

    string lType;
    std::istringstream objStream(line);
    vec3 cNormal;

    while (!fileStream.eof()) {
        getline(fileStream, line);
        objStream.clear();
        objStream.str(line);

        objStream >> lType;

        if (objStream.fail())
            continue;

        if (lType == "v") { // vertex coordinates
            objStream >> x >> y >> z;
            vertVals.push_back(vec3(x,y,z));
            continue;
        }else if (lType == "vt") { // texture coordinates
            objStream >>  x >> y;
            stVals.push_back(vec2(x,y));
            continue;
        }else if (lType == "vn") { // vertex normals
            objStream >> x >> y >> z;
            normVals.push_back(vec3(x,y,z));
            continue;
        }else if (lType == "f") { // faces
            string oneCorner;
            std::vector<vertIndices> viList;

            for (int i = 0; i < 4; i++) {
                objStream >> oneCorner;
                if (objStream.fail())
                    break;

                vertIndices temp;

                temp.ti = temp.ni = temp.vi = 0;
                
                // parse each face's indices
                if (sscanf(oneCorner.c_str(), "%i/%i/%i", &temp.vi, &temp.ti, &temp.ni) != 3) {
                    temp.ti = temp.ni = temp.vi = 0;
                    if (sscanf(oneCorner.c_str(), "%i//%i", &temp.vi, &temp.ni) != 2) {
                        temp.ti = temp.ni = temp.vi = 0;
                        if (sscanf(oneCorner.c_str(), "%i/%i", &temp.vi, &temp.ti) != 2){
                            temp.ti = temp.ni = temp.vi = 0;
                            if (sscanf(oneCorner.c_str(), "%i", &temp.vi) != 1)
                                std::cout << "unimplemented face format\n";
                        }
                    }
                }
                // account for relative indexing, adjust by number of preceding attributes
                if (temp.vi < 0) temp.vi += vertVals.size() + 1;
                if (temp.ni < 0) temp.ni += normVals.size() + 1;
                if (temp.ti < 0) temp.ti += stVals.size() + 1;

                viList.push_back(temp);

                if (i > 2) { 
                    // if more than 3 verts in face, it's a quad, so re-emit 2 verts to cause a tri
                    viList.push_back(viList[viList.size() - 4]);
                    viList.push_back(viList[viList.size() - 3]);
                }
            }

            if (viList[0].ni == 0) {
                // if no normal, calculate a face formal from face edges
                vec3 ab, bc;

                ab = vertVals[viList[2].vi - 1] - vertVals[viList[1].vi - 1];
                bc = vertVals[viList[0].vi - 1] - vertVals[viList[1].vi - 1];

                cNormal = cross(ab, bc);
            }
            else
                cNormal = vec3(0);

            for (vertIndices temp : viList) {
                int vertRef = temp.vi;
                int tcRef = temp.ti;
                int normRef = temp.ni;

                // references start at 1, not 0, so adjust
                vertRef--;
                tcRef--;
                normRef--;

                if (vertRef > -1) {
                    triangleVerts.push_back(vertVals[vertRef]);
                }
                else {
                    triangleVerts.push_back(vec3(0.0f));
                }

                if (tcRef > -1) {
                    textureCoords.push_back(stVals[tcRef]);
                }
                else {
                    textureCoords.push_back(vec2(0.0f));
                }

                if (normRef > -1) {
                    normals.push_back(normVals[normRef]);
                }
                else {
                    normals.push_back(cNormal);
                }
            }
            continue;
        }else if (lType == "mtllib") { // uses a material library file
            string fname;
            objStream >> fname;
            fname = getPathName(filePath) + fname;
            parseMTL(fname.c_str());
            continue;
        }
        else if (lType == "usemtl") { // use this material on the next faces, unitl next material
            objMesh temp;
            objStream >> temp.myName;

            if (meshes.empty() || (temp.myName != meshes.back().myName)) // only create mesh when there is a material change
            {
                std::cout << temp.myName << " " << getNumVertices() << "\n";
                temp.startingVert = getNumVertices();
                meshes.push_back(temp);
            }
            continue;
        }
    }
}
int ModelImporter::getNumVertices() { return (triangleVerts.size()); }
std::vector<vec3> ModelImporter::getVertices() { return triangleVerts; }
std::vector<vec2> ModelImporter::getTextureCoordinates() { return textureCoords; }
std::vector<vec3> ModelImporter::getNormals() { return normals; }
std::vector<objMesh> ModelImporter::getMeshes() { return meshes; }
