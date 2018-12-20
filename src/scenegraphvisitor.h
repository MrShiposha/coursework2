#ifndef CG_SEM5_SCENEGRAPHVISITOR_H
#define CG_SEM5_SCENEGRAPHVISITOR_H

class SceneNode;

class SceneGraphVisitor 
{
public:
    virtual ~SceneGraphVisitor() {}
    virtual void visit_up(SceneNode &)   = 0;
    virtual void visit_down(SceneNode &) = 0;
};

#endif // CG_SEM5_SCENEGRAPHVISITOR_H