#ifndef CG_SEM5_DETAIL_MESHESCONTAINER_H
#define CG_SEM5_DETAIL_MESHESCONTAINER_H

#include <cstdint>
#include <cstddef>

#include "../scenegraphvisitor.h"
#include "../staticmesh.h"

class StaticMeshesContainer : public SceneGraphVisitor
{
public:
    const std::vector<std::shared_ptr<StaticMesh>> &get_meshes() const;

    virtual void visit_up(std::shared_ptr<SceneNode>) override;
    virtual void visit_down(std::shared_ptr<SceneNode>) override;

private:
    std::vector<std::shared_ptr<StaticMesh>> meshes;
};

#endif // CG_SEM5_DETAIL_MESHESCONTAINER_H
