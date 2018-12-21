#ifndef CG_SEM5_COURSEWORK_SCENENODE_H
#define CG_SEM5_COURSEWORK_SCENENODE_H

#include <memory>
#include <string>
#include <string_view>

class SceneGraphVisitor;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
    SceneNode();
    SceneNode(std::string_view id);
    virtual ~SceneNode();

    virtual void accept_up(SceneGraphVisitor &);
    virtual void accept_down(SceneGraphVisitor &);

    const std::string &get_id() const;
    void set_id(std::string_view id);

    bool is_changed() const;
    void set_changed(bool);
    void mark_changed();
    void mark_unchanged();
    
protected:
    std::string id;

    bool changed;
};

#endif // CG_SEM5_COURSEWORK_SCENENODE_H