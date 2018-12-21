#ifndef CG_SEM5_SCENEGRAPH_H
#define CG_SEM5_SCENEGRAPH_H

#include <vector>
#include <memory>

#include "scenenode.h"

class SceneGraph : public SceneNode
{
public:
    SceneGraph(std::string_view id);

    virtual void accept_up(SceneGraphVisitor &) override;
    virtual void accept_down(SceneGraphVisitor &) override;

    void add_node(std::shared_ptr<SceneNode>);

private:
    std::vector<std::shared_ptr<SceneNode>> nodes;
};

#endif // CG_SEM5_SCENEGRAPH_H