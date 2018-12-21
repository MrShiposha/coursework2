#ifndef CG_SEM5_COURSEWORK_SCENENODE_H
#define CG_SEM5_COURSEWORK_SCENENODE_H

#include <string>
#include <string_view>

class SceneGraphVisitor;

class SceneNode
{
public:
    SceneNode();
    SceneNode(std::string_view id);
    virtual ~SceneNode();

    virtual void accept_up(SceneGraphVisitor &);
    virtual void accept_down(SceneGraphVisitor &);

    const std::string &get_id() const;
    void set_id(std::string_view id);

protected:
    std::string id;
};

#endif // CG_SEM5_COURSEWORK_SCENENODE_H