#include "memo.h"
#include "../rules/rules.h"
#include "expression.h"

namespace pg_carbon {

void GroupExpression::AddAppliedRule(Rule *rule) {
    int id = rule->GetId();
    if (id < 0)
        return;

    if (static_cast<size_t>(id) >= applied_rules_.size()) {
        applied_rules_.resize(id + 1, false);
    }
    applied_rules_[id] = true;
}

bool GroupExpression::HasAppliedRule(Rule *rule) const {
    int id = rule->GetId();
    if (id < 0 || static_cast<size_t>(id) >= applied_rules_.size()) {
        return false;
    }
    return applied_rules_[id];
}

void Group::AddExpression(GroupExpression *expr) {
    expr->SetGroup(this);
    if (expr->GetOperator()->IsLogical()) {
        logical_exprs_.push_back(expr);
    } else {
        physical_exprs_.push_back(expr);
    }
}

Group *Memo::InsertExpression(GroupExpression *expr) {
    // In a real implementation, check if expr already exists in the memo.
    // For this skeleton, we assume we are building the initial tree or adding new
    // expressions.

    LogicalProperties *props = nullptr;
    if (expr->GetOperator()->IsLogical()) {
        auto *log_op = static_cast<LogicalOperator *>(expr->GetOperator());
        // Step 1: Derive properties (Bottom-Up)
        props = log_op->DeriveLogicalProps(this, expr->GetChildren());
    }

    // If the expression doesn't have a group, create one.
    // Note: This is a simplification. Usually, we look up if the expression
    // exists. If we are inserting a new expression into an existing group, the
    // caller handles that. Here we assume we are creating a new group for a new
    // logical expression.

    // Step 2: Create new group with calculated properties
    Group *group = NewGroup(props);
    group->AddExpression(expr);
    return group;
}

Group *Memo::InitMemo(Operator *root_op) {
    if (!root_op)
        return nullptr;

    PgVector<Group *> child_groups;
    for (auto *input : root_op->GetInputs()) {
        child_groups.push_back(InitMemo(input));
    }

    auto expr = new GroupExpression(root_op, child_groups);
    return InsertExpression(expr);
}

Group *Memo::NewGroup(LogicalProperties *props) {
    Group *group = new Group();
    if (props) {
        group->SetLogicalProperties(props);
    }
    groups_.push_back(group);
    return group;
}

Group *Memo::CopyIn(Expression *expr) {
    if (expr->GetGroup()) {
        return expr->GetGroup();
    }

    PgVector<Group *> child_groups;
    for (auto *child : expr->GetChildren()) {
        child_groups.push_back(CopyIn(child));
    }

    auto *group_expr = new GroupExpression(expr->GetOperator(), child_groups);
    return InsertExpression(group_expr);
}

} // namespace pg_carbon
