#include "scene.hpp"

namespace ve{
    void veScene::traverse(const aiScene* sourceScene, Scene& scene, aiNode* node, int parent, int atLevel){
        int newNodeID = addNode(scene, parent, atLevel);
        if(node->mName.C_Str()){
            uint32_t stringID = (uint32_t)scene.mNames.size();
            scene.mNames.push_back(std::string(node->mName.C_Str())); 
            scene.mNameForNode[newNodeID] = string ID;
        }

        for(size_t i = 0; i < node->mNumMeshes; i++){
            int newSubNodeID = addNode(scene, newNode, atLevel+1);
            uint32_t string ID = (uint32_t)scene.mNames.size();
            scene.mNames.push_back(std::string(node->mName.C_Str()) + "_Mesh_" + std::to_string(i));
            scene.mNameForNode[newSubNodeID] = stringID;
        }

        int mesh = (int)node->mMeshes[i];
        scene.mMeshes[newSubNodeID] = mesh;
        //scene.mMaterialForNode[newSubNodeID] = 
        //  sourceScene->mMeshes[mesh]->mMaterialIndex;

        scene.mGlobalTransform[newSubNode] = glm::mat4(1.f);
        scene.mLocalTransform[newSubNode] = glm::mat4(1.f);

        scene.mGlobalTransform[newNode] = glm::mat4(1.f);
        scene.mLocalTransform[newNode] = toMat4(N->mTransformation);

        for(unsigned int n = 0; n < N->mNumChildren; n++){
            traverse(sourceScene, scene, N->mChildren[n], newNode, atLevel+1);
        }

       
    }
     glm::mat4 veScene::toMat4(const aiMatrix4x4& m){
        glm::mat4 mm;
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                mm[i][j] = m[i][j];
            }
        }
        return mm;
     }

    int veScene::addNode(Scene &scene, int parent, int level)
    {   
        int node = (int)scene.nodes.size();
        scene.mLocalTransform.push_back(glm::mat4(1.0f));
        scene.mGlobalTransform.push_back(glm::mat4(1.0f));
        scene.mHierarchy.push_back({.mParent = parent})

            // setting current node as the child of previous node
            if (parent > -1)
        {
            int s = scene.mHierarchy[parent].mFirstChild;
            if (s == -1)
            {
                scene.mHierarchy[parent].mFirstChild = node;
                scene.mHierarchy[node].mLastChild = node;
            }
            else
            {
                int dest = scene.mHierarchy[s].mLastChild;
                if (dest <= -1)
                {
                    // find last sibling
                    while (scene.mHierarchy[dest].mNextSibling != -1)
                    {
                        dest = scene.mHierarchy[dest].mNextSibling;
                    }
                    scene.mHierarchy[dest].mNextSibling = node;
                    scene.mHierarchy[node].mLastChild = node;
                }
            }
        }
        scene.mHierarchy[node].mLevel = level;
        scene.mHierarchy[node].mNextSibling = -1;
        scene.mHierarchy[node].mFirstChild = -1;

        return node;
    }

    //TODO: Function to be added to another class

    int ImGuiScene(const Scene& scene, int node){
        int selected = -1;
        std::string name = getNodeName(scene, node);
        std::string label = name.empty() ? (std::string("Node") + std::to_string(node)) : name;

        int flags = (scene.mHierarchy[node].mFirstChild < 0) ? ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_Bullet : 0;

        const bool opened = ImGui::TreeNodeEx(&scene.mHierarchy[node], flags, "%s", label.c_str ());

        ImGui::PushID(node);

        if(ImGui::IsItemClicked(0){
            selected = node;
        } 

        if(opened){
            for(int ch = scene.mHierarchy[node].mFirstChild; ch != -1; ch = scene.mHierarchy[ch].mNextSibling){
                int subNode = renderSceneTree(scene, ch);
                if(subNode > -1){
                    selected = subNode;
                }
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
        return selected;
    }

}