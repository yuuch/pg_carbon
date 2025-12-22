#include "opt_inputs.h"
#include "opt_grp.h"

namespace pg_carbon {

// 6. OptInputs
void OptInputs::perform(TaskScheduler *scheduler) {
    // 48: childExpr <- expr.GetNextInput() (Simulated by current_input_index_)
    const auto &children = expr_->GetChildren();

    // 49: if childExpr is null then (All inputs optimized)
    if (static_cast<size_t>(current_input_index_) >= children.size()) {
        // 50: memo.UpdateBestPlan(expr)
        // In our simplified model, we track the best expression in the group.
        // Real implementation would calculate cost and compare.
        expr_->GetGroup()->SetBestExpression(expr_);

        // 51: return
        return;
    }

    // 52: tasks.Push(OptInputs(expr, limit))
    // Add a task to optimize the next input
    auto *next_step = new OptInputs(expr_, context_);
    next_step->current_input_index_ = current_input_index_ + 1;
    scheduler->ScheduleTask(next_step);

    // 53: limit <- UpdateCostLimit(expr, limit) (omitted)

    // 54: tasks.Push(OptGrp(GetGroup(childExpr), limit))
    // Optimize the current input
    auto *child_group = children[current_input_index_];
    scheduler->ScheduleTask(new OptGrp(child_group, context_));
}

} // namespace pg_carbon
