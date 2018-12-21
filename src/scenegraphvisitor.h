#ifndef CG_SEM5_SCENEGRAPHVISITOR_H
#define CG_SEM5_SCENEGRAPHVISITOR_H

#include <memory>

class SceneNode;

class SceneGraphVisitor 
{
public:
    virtual ~SceneGraphVisitor() {}
    virtual void visit_up(std::shared_ptr<SceneNode>)   = 0;
    virtual void visit_down(std::shared_ptr<SceneNode>) = 0;
};

#endif // CG_SEM5_SCENEGRAPHVISITOR_H