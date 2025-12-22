#ifndef PG_CARBON_OPTIMIZER_TASKS_OPT_GRP_H
#define PG_CARBON_OPTIMIZER_TASKS_OPT_GRP_H

#include "scheduler.h"

namespace pg_carbon {

// 1. OptGrp (Optimize Group)
class OptGrp : public Task {
public:
    OptGrp(Group *group, Context *context) : group_(group), context_(context) {}
    void perform(TaskScheduler *scheduler) override;

private:
    Group *group_;
    Context *context_;
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_OPT_GRP_H
