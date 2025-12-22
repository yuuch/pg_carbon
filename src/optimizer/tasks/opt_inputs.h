#ifndef PG_CARBON_OPTIMIZER_TASKS_OPT_INPUTS_H
#define PG_CARBON_OPTIMIZER_TASKS_OPT_INPUTS_H

#include "scheduler.h"

namespace pg_carbon {

// 6. OptInputs
class OptInputs : public Task {
public:
    OptInputs(GroupExpression *expr, Context *context) : expr_(expr), context_(context) {}
    void perform(TaskScheduler *scheduler) override;

private:
    GroupExpression *expr_;
    Context *context_;
    int current_input_index_ = 0;
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_OPT_INPUTS_H
