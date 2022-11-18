#pragma once

#include <unordered_map>
#include <vector>

namespace ve
{
    class veScene{
        public:
            struct SceneNode{
                int mMesh;
                int mParent;
                int mFirstChild;
                int mRightSibling;
                //int mMaterial;
            };

            struct Scene{
                std::vector<SceneNode> mNodes;
                std::vector<mat4> mLocalTransforms;
                std::vector<mat4> mGlobalTransforms;
                std::vector<Hierarchy> mHierarchy;
                std::unordered_map<uint32_t, uint32_t> mMeshes;
                //std::unordered_map<uint32_t, uint32_t> mMaterialForNode;
                std::unordered_map<uint32_t, uint32_t> mNameForNode;
                std::vector<std::string> mNames;
                //std::vector<std::string> mMaterialNames;
            };

            struct Hierarchy{
                int mParent;
                int mFirstChild;
                int mNextSibling;
                int mLastSibling;
                int mLevel;
            };
        void traverse(const aiScene*, Scene&, aiNode*, int, int);

        glm::mat4 toMat4(const aiMatrix4x4&);

        int addNode(Scene&, int, int);
    }
}