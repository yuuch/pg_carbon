#ifndef PG_CARBON_OPTIMIZER_TASKS_OPT_EXPR_H
#define PG_CARBON_OPTIMIZER_TASKS_OPT_EXPR_H

#include "scheduler.h"

namespace pg_carbon {

// 3. OptExpr (Optimize Expression)
class OptExpr : public Task {
public:
    OptExpr(GroupExpression *expr, Context *context) : expr_(expr), context_(context) {}
    void perform(TaskScheduler *scheduler) override;

private:
    GroupExpression *expr_;
    Context *context_;
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_OPT_EXPR_H
