#ifndef PG_CARBON_OPTIMIZER_TASKS_EXPL_EXPR_H
#define PG_CARBON_OPTIMIZER_TASKS_EXPL_EXPR_H

#include "scheduler.h"

namespace pg_carbon {

// 4. ExplExpr (Explore Expression)
class ExplExpr : public Task {
public:
    ExplExpr(GroupExpression *expr, Context *context) : expr_(expr), context_(context) {}
    void perform(TaskScheduler *scheduler) override;

private:
    GroupExpression *expr_;
    Context *context_;
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_TASKS_EXPL_EXPR_H
