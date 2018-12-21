#include "scenegraph.h"

SceneGraph::SceneGraph(std::string_view id)
: SceneNode(id)
{}

void SceneGraph::accept_up(SceneGraphVisitor &)
{}

void SceneGraph::accept_down(SceneGraphVisitor &visitor)
{
    for(auto &&node : nodes)
    {
        node->accept_down(visitor);
        node->accept_up(visitor);
    }
}

void SceneGraph::add_node(std::shared_ptr<SceneNode> node)
{
    nodes.push_back(node);
}