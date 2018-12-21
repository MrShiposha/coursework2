#include "scenenode.h"
#include "scenegraphvisitor.h"

static size_t default_id = 0;

SceneNode::SceneNode()
: id("node" + std::to_string(default_id++))
{}

SceneNode::SceneNode(std::string_view id)
: id(id.data())
{}

SceneNode::~SceneNode()
{}

void SceneNode::accept_up(SceneGraphVisitor &visitor)
{
    visitor.visit_up(shared_from_this());
}

void SceneNode::accept_down(SceneGraphVisitor &visitor)
{
    visitor.visit_down(shared_from_this());
}

const std::string &SceneNode::get_id() const
{
    return id;
}

void SceneNode::set_id(std::string_view id)
{
    this->id = id.data();
}
