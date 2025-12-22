#ifndef PG_CARBON_OPTIMIZER_TASKS_EXPL_GRP_H
#define PG_CARBON_OPTIMIZER_TASKS_EXPL_GRP_H

#include "scheduler.h"

namespace pg_carbon {

// 2. ExplGrp (Explore Group)
class ExplGrp : public Task {
public:
    ExplGrp(Group *group, Context *context) : group_(group), context_(context) {}
    void perform(TaskScheduler *scheduler) override;

private:
    Group *group_;
    Context *context_;
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_EXPL_GRP_H
