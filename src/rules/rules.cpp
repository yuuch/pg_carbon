#include "rules.h"
#include "../operators/operators.h"

namespace pg_carbon {

// --- RuleGetToScan ---

bool RuleGetToScan::Matches(GroupExpression *expr) const {
  return expr->GetOperator()->IsLogical() &&
         dynamic_cast<LogicalGet *>(expr->GetOperator());
}

PgVector<GroupExpression *>
RuleGetToScan::Transform(GroupExpression *expr) const {
  auto logical_get = dynamic_cast<LogicalGet *>(expr->GetOperator());
  auto physical_scan = new PhysicalTableScan(logical_get->GetTableOid(),
                                             logical_get->GetRtIndex());
  auto group_expr = new GroupExpression(physical_scan, {});

  PgVector<GroupExpression *> result;
  result.push_back(group_expr);
  return result;
}

// --- RuleFilterToPhysical ---

bool RuleFilterToPhysical::Matches(GroupExpression *expr) const {
  return expr->GetOperator()->IsLogical() &&
         dynamic_cast<LogicalFilter *>(expr->GetOperator());
}

PgVector<GroupExpression *>
RuleFilterToPhysical::Transform(GroupExpression *expr) const {
  auto logical = dynamic_cast<LogicalFilter *>(expr->GetOperator());
  auto physical = new PhysicalFilter(logical->GetQual());
  auto group_expr = new GroupExpression(physical, expr->GetChildren());

  PgVector<GroupExpression *> result;
  result.push_back(group_expr);
  return result;
}

// --- RuleProjectionToPhysical ---

bool RuleProjectionToPhysical::Matches(GroupExpression *expr) const {
  return expr->GetOperator()->IsLogical() &&
         dynamic_cast<LogicalProjection *>(expr->GetOperator());
}

PgVector<GroupExpression *>
RuleProjectionToPhysical::Transform(GroupExpression *expr) const {
  auto logical = dynamic_cast<LogicalProjection *>(expr->GetOperator());
  auto physical = new PhysicalProjection(logical->GetTargetList());
  auto group_expr = new GroupExpression(physical, expr->GetChildren());

  PgVector<GroupExpression *> result;
  result.push_back(group_expr);
  return result;
}

// --- RuleSortToPhysical ---

bool RuleSortToPhysical::Matches(GroupExpression *expr) const {
  return expr->GetOperator()->IsLogical() &&
         dynamic_cast<LogicalSort *>(expr->GetOperator());
}

PgVector<GroupExpression *>
RuleSortToPhysical::Transform(GroupExpression *expr) const {
  auto logical = dynamic_cast<LogicalSort *>(expr->GetOperator());
  auto physical = new PhysicalSort(logical->GetSortClause());
  auto group_expr = new GroupExpression(physical, expr->GetChildren());

  PgVector<GroupExpression *> result;
  result.push_back(group_expr);
  return result;
}

// --- RuleLimitToPhysical ---

bool RuleLimitToPhysical::Matches(GroupExpression *expr) const {
  return expr->GetOperator()->IsLogical() &&
         dynamic_cast<LogicalLimit *>(expr->GetOperator());
}

PgVector<GroupExpression *>
RuleLimitToPhysical::Transform(GroupExpression *expr) const {
  auto logical = dynamic_cast<LogicalLimit *>(expr->GetOperator());
  auto physical =
      new PhysicalLimit(logical->GetLimitOffset(), logical->GetLimitCount());
  auto group_expr = new GroupExpression(physical, expr->GetChildren());

  PgVector<GroupExpression *> result;
  result.push_back(group_expr);
  return result;
}

} // namespace pg_carbon
