#ifndef CG_SEM5_DETAIL_MATERIALCOUNTERVISITOR_H
#define CG_SEM5_DETAIL_MATERIALCOUNTERVISITOR_H

#include <cstdint>
#include <cstddef>

#include "../scenegraphvisitor.h"

class MaterialCounterVisitor : public SceneGraphVisitor
{
public:
    MaterialCounterVisitor();

    uint32_t get_materials_count() const;

    virtual void visit_up(std::shared_ptr<SceneNode>) override;
    virtual void visit_down(std::shared_ptr<SceneNode>) override;

private:
    uint32_t materials_count;
};

#endif // CG_SEM5_DETAIL_MATERIALCOUNTERVISITOR_H
