#ifndef CG_SEM5_ACTORSCONTAINER_H
#define CG_SEM5_ACTORSCONTAINER_H

#include <vector>
#include <memory>

#include "../scenegraphvisitor.h"
#include "../actor.h"

class ActorsContainer : public SceneGraphVisitor
{
public:
    virtual void visit_up(std::shared_ptr<SceneNode>) override;
    virtual void visit_down(std::shared_ptr<SceneNode>) override;

    const std::vector<std::shared_ptr<Actor>> &get_actors() const;

private:
    std::vector<std::shared_ptr<Actor>> actors;
};

#endif // CG_SEM5_ACTORSCONTAINER_H