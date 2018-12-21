#ifndef CG_SEM5_MESHSELECTORVISITOR_H
#define CG_SEM5_MESHSELECTORVISITOR_H

#include "../scenegraphvisitor.h"
#include "../abstractmesh.h"

class MeshSelectorVisitor : public SceneGraphVisitor
{
public:
    MeshSelectorVisitor();

    virtual void visit_up(std::shared_ptr<SceneNode>) override;
    virtual void visit_down(std::shared_ptr<SceneNode>) override;

    void reset_current_mesh();
    std::shared_ptr<AbstractMesh> get_next_mesh();

private:
    size_t current_mesh;
    std::vector<std::shared_ptr<AbstractMesh>> meshes;
};

#endif // CG_SEM5_MESHSELECTORVISITOR_H