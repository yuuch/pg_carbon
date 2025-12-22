#include "opt_grp.h"
#include "expl_grp.h"
#include "opt_expr.h"

namespace pg_carbon {

// 1. OptGrp (Optimize Group)
void OptGrp::perform(TaskScheduler *scheduler) {
    if (group_->IsExplored()) {
        const auto &logical_exprs = group_->GetLogicalExpressions();
        // Reverse iteration to push tasks in correct order (stack)
        for (int i = logical_exprs.size() - 1; i >= 0; --i) {
            auto *expr = logical_exprs[i];
            // Schedule OptExpr for each logical expression
            scheduler->ScheduleTask(new OptExpr(expr, context_));
        }
    } else {
        // If not explored, schedule exploration first.
        // 4: tasks.Push(OptGrp(grp, limit)) -> Will be executed second
        scheduler->ScheduleTask(new OptGrp(group_, context_));

        // 5: tasks.Push(ExplGrp(grp, limit)) -> Will be executed first
        scheduler->ScheduleTask(new ExplGrp(group_, context_));
    }
}

} // namespace pg_carbon
