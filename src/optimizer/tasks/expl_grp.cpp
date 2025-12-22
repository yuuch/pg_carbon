#include "expl_grp.h"
#include "expl_expr.h"

namespace pg_carbon {

// 2. ExplGrp (Explore Group)
void ExplGrp::perform(TaskScheduler *scheduler) {
    // 10: grp.Explored <- true
    group_->SetExplored(true);

    const auto &logical_exprs = group_->GetLogicalExpressions();
    // 11: for expr in grp.Expressions do
    // 12: tasks.Push(ExplExpr(expr, limit))
    // Reverse iteration to push tasks in correct order (stack)
    for (int i = logical_exprs.size() - 1; i >= 0; --i) {
        auto *expr = logical_exprs[i];
        scheduler->ScheduleTask(new ExplExpr(expr, context_));
    }
}

} // namespace pg_carbon
