#include "expl_expr.h"
#include "apply_rule.h"
#include "expl_grp.h"
#include <typeinfo>

namespace pg_carbon {

// 4. ExplExpr (Explore Expression)
void ExplExpr::perform(TaskScheduler *scheduler) {
    // 13: function ExplExpr(expr, limit)
    // 14: moves <- empty
    PgVector<ApplyRule *> moves;

    const auto &rules = scheduler->GetRules();
    for (auto *rule : rules) {
        // 16: if !expr.IsApplied(rule) and rule.CheckPattern(expr)
        // 16: Check if rule is applicable (Top-level operator match)
        Operator *pattern_op = rule->GetPattern()->GetOperator();
        bool is_applicable = false;

        if (dynamic_cast<PatternLeaf *>(pattern_op)) {
            is_applicable = true;
        } else if (typeid(*pattern_op) == typeid(*expr_->GetOperator())) {
            is_applicable = true;
        }

        if (!expr_->HasAppliedRule(rule) && is_applicable) {
            // 17: moves.Add(ApplyRule(...))
            // exploring=true
            moves.push_back(new ApplyRule(rule, expr_, context_, true));
        }
    }

    // 19: for m in moves do tasks.Push(m)
    for (auto *m : moves) {
        scheduler->ScheduleTask(m);
    }

    // 21: for childExpr in inputs (children groups actually)
    const auto &children = expr_->GetChildren();
    for (int i = children.size() - 1; i >= 0; --i) {
        auto *child_group = children[i];
        // 22: grp <- GetGroup
        // 23: if !grp.Explored
        if (!child_group->IsExplored()) {
            // 24: tasks.Push(ExplGrp(grp))
            scheduler->ScheduleTask(new ExplGrp(child_group, context_));
        }
    }
}

} // namespace pg_carbon
