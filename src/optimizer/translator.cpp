#include "translator.h"
#include "../operators/operators.h"
#include <iostream>

extern "C" {
#include "catalog/pg_type.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "nodes/primnodes.h"
#include "parser/parse_collate.h"
#include "utils/lsyscache.h"
}

namespace pg_carbon {

// Helper walker to change Vars to OUTER_VAR in place

Operator *Translator::TranslateQueryToCarbon(Query *pg_query) {
  // 1. Translation of the FROM clause (Join Tree)
  // For now, we still assume a simple relation scan or single table.
  // In the future, this should handle joins recursively.
  Operator *current_op = nullptr;

  if (pg_query->rtable && pg_query->jointree && pg_query->jointree->fromlist) {
    List *fromlist = pg_query->jointree->fromlist;
    if (list_length(fromlist) == 1) {
      Node *node = (Node *)linitial(fromlist);
      if (IsA(node, RangeTblRef)) {
        RangeTblRef *rtr = (RangeTblRef *)node;
        RangeTblEntry *rte =
            (RangeTblEntry *)list_nth(pg_query->rtable, rtr->rtindex - 1);

        if (rte->rtekind == RTE_RELATION) {
          current_op = new LogicalGet(rte->relid, rtr->rtindex);
        }
      }
    }
  }

  if (!current_op) {
    return nullptr;
  }

  // 2. Filter (WHERE clause)
  if (pg_query->jointree->quals) {
    auto filter = new LogicalFilter(pg_query->jointree->quals);
    filter->AddInput(current_op);
    current_op = filter;
  }

  // 3. Aggregation (GROUP BY / HAVING / Aggs) -> UNSUPPORTED
  if (pg_query->groupClause || pg_query->hasAggs || pg_query->havingQual) {
    return nullptr;
  }

  // 4. Sort (ORDER BY)
  if (pg_query->sortClause) {
    auto sort = new LogicalSort(pg_query->sortClause);
    sort->AddInput(current_op);
    current_op = sort;
  }

  // 5. Limit (LIMIT / OFFSET)
  if (pg_query->limitOffset || pg_query->limitCount) {
    auto limit = new LogicalLimit(pg_query->limitOffset, pg_query->limitCount);
    limit->AddInput(current_op);
    current_op = limit;
  }

  // 6. Projection (TargetList)
  // We always add a projection node at the top to represent the final output
  // targets.
  if (pg_query->targetList) {
    auto projection = new LogicalProjection(pg_query->targetList);
    projection->AddInput(current_op);
    current_op = projection;
  }

  return current_op;
}

Plan *Translator::TranslatePlanToPG(Memo *memo,
                                    GroupExpression *best_physical_plan,
                                    Query *pg_query) {
  if (!best_physical_plan)
    return nullptr;

  Operator *op = best_physical_plan->GetOperator();
  if (!op->IsPhysical()) {
    return nullptr;
  }

  // Helper to get child plan
  auto GetChildPlan = [&](int index) -> Plan * {
    if (index >= best_physical_plan->GetChildren().size())
      return nullptr;
    Group *child_group = best_physical_plan->GetChildren()[index];
    // In a real system, we'd pick the best expression from child group
    // based on cost/properties. Here we just take the one marked best.
    GroupExpression *child_expr = child_group->GetBestExpression();
    return TranslatePlanToPG(memo, child_expr, pg_query);
  };

  if (auto scan = dynamic_cast<PhysicalTableScan *>(op)) {
    SeqScan *node = makeNode(SeqScan);
    node->scan.scanrelid = scan->GetRtIndex();
    // In real system, targetlist and quals would be properly set
    // Construct TargetList from Logical Properties
    node->scan.plan.targetlist = NIL;
    if (best_physical_plan->GetGroup() &&
        best_physical_plan->GetGroup()->GetLogicalProperties()) {
      node->scan.plan.targetlist = BuildTargetList(
          memo, best_physical_plan->GetGroup()->GetLogicalProperties());
    }
    return (Plan *)node;
  }

  if (auto filter = dynamic_cast<PhysicalFilter *>(op)) {
    Plan *child_plan = GetChildPlan(0);
    // In PG, quals are often attached to the node itself (e.g. Scan)
    // or we use a Result node.
    // Simplifying: If child is Scan, attach qual. Else create Result.
    if (child_plan && IsA(child_plan, SeqScan)) {
      // qual is a List* in Plan, but Node* in our Operator.
      // We must wrap the expression in a List.
      Node *qual_copy = (Node *)copyObjectImpl(filter->GetQual());
      ((SeqScan *)child_plan)->scan.plan.qual = list_make1(qual_copy);
      return child_plan;
    } else {
      // Create a Result node acting as filter
      // Result *node = makeNode(Result);
      // node->plan.lefttree = child_plan;
      // node->plan.qual = (List *)copyObjectImpl(filter->GetQual());
      // node->plan.targetlist = child_plan ? child_plan->targetlist : NIL;
      // return (Plan *)node;

      // Fallback: Attach to child plan's qual if possible?
      // No, let's strictly return child_plan for now because we haven't handled
      // creating Result nodes properly with all fields.
      // Or just Warning and return child.
      elog(WARNING, "Translator: PhysicalFilter text not fully implemented, "
                    "attaching to child if Scan.");
      return child_plan;
    }
  }

  if (auto sort = dynamic_cast<PhysicalSort *>(op)) {
    Plan *child_plan = GetChildPlan(0);
    Sort *node = makeNode(Sort);
    node->plan.lefttree = child_plan;
    node->plan.targetlist = child_plan->targetlist; // Pass through tlist

    int numCols = list_length(sort->GetSortClause());
    node->numCols = numCols;
    node->sortColIdx = (AttrNumber *)palloc(numCols * sizeof(AttrNumber));
    node->sortOperators = (Oid *)palloc(numCols * sizeof(Oid));
    node->collations = (Oid *)palloc(numCols * sizeof(Oid));
    node->nullsFirst = (bool *)palloc(numCols * sizeof(bool));

    int i = 0;
    ListCell *lc;
    foreach (lc, sort->GetSortClause()) {
      SortGroupClause *sgc = (SortGroupClause *)lfirst(lc);
      TargetEntry *tle = nullptr;

      // Find TLE with matching ressortgroupref
      ListCell *l;
      foreach (l, pg_query->targetList) {
        TargetEntry *candidate = (TargetEntry *)lfirst(l);
        if (candidate->ressortgroupref == sgc->tleSortGroupRef) {
          tle = candidate;
          break;
        }
      }

      if (tle) {
        node->sortColIdx[i] = tle->resno;
        node->sortOperators[i] = sgc->sortop;
        node->collations[i] = exprCollation((Node *)tle->expr);
        node->nullsFirst[i] = sgc->nulls_first;
      } else {
        elog(WARNING, "Translator: Could not find TLE for SortGroupRef %u",
             sgc->tleSortGroupRef);
        node->sortColIdx[i] = 0; // Likely to crash if executed
      }
      i++;
    }

    return (Plan *)node;
  }

  if (auto limit = dynamic_cast<PhysicalLimit *>(op)) {
    Plan *child_plan = GetChildPlan(0);
    Limit *node = makeNode(Limit);
    node->plan.lefttree = child_plan;
    node->plan.targetlist = child_plan->targetlist;
    node->limitOffset = limit->GetLimitOffset();
    node->limitCount = limit->GetLimitCount();
    return (Plan *)node;
  }

  if (auto proj = dynamic_cast<PhysicalProjection *>(op)) {
    Plan *child_plan = GetChildPlan(0);
    // If child is already a plan, we can just update its targetlist?
    // Or create a Result node (Projection)
    // For now, let's just update the child plan's target list if it exists.
    if (child_plan) {
      child_plan->targetlist = (List *)copyObjectImpl(proj->GetTargetList());
      return child_plan;
    }
    // If no child (e.g. Result with constant), create Result.
    // Result *node = makeNode(Result);
    // node->plan.targetlist = (List *)copyObjectImpl(proj->GetTargetList());
    // return (Plan *)node;
  }

  return nullptr;
}

List *Translator::BuildTargetList(Memo *memo, const LogicalProperties *props) {
  List *target_list = NIL;
  const ColSet &output_cols = props->GetOutputColumns();

  // Iterate over columns.
  // Since ColSet doesn't have an iterator for enabled bits easily exposed in
  // this quick implementation, we might need to iterate up to a max (which is
  // inefficient) or improve ColSet. For now, let's assume we iterate up to
  // memo->columns_.size() and check IsSet.

  // TODO: Improve ColSet iteration.
  // TEMPORARY HACK: Iterate all Memo columns and check if in Output Cols.
  for (int i = 0; i < 1000; ++i) { // Limit to avoid infinite loop if bug
    // This is obviously not ideal.
    // Better: ColSet should expose a way to iterate.
    // Let's rely on checking common IDs if we knew them, but we don't.

    // Let's modify ColSet in future. For now, we will iterate all columns in
    // Memo.
    CarbonColumn *col = memo->GetColumn(i);
    if (!col)
      break; // End of registered columns

    // Check if this column is in the output set
    // We need ColSet::Has(int)
    // Since we don't have it exposed in the snippet, we added it in thought but
    // maybe not in code? Re-checking memo.h content... ColSet has Add, Union,
    // IsSubset. Missing IsMember/Has. I will add a IsMember to ColSet in memo.h
    // after this or assume it exists/add it now. Wait, I can't modify memo.h
    // here. I will add IsMember to memo.h in a separate step if needed.
    // Actually I can access bits_ if protected/public? It is private.

    // STOPGAP: I will add IsMember to ColSet in next step.
    // For this step, I'll write the code assuming IsMember exists, then fix
    // memo.h. Or I can use IsSubset with a singleton.

    ColSet singleton = ColSet::MakeSingleton(i);
    if (singleton.IsSubset(output_cols)) {
      TargetEntry *tle = nullptr;
      if (col->GetType() == CarbonColumnType::TABLE_COLUMN) {
        auto *tc = static_cast<TableColumn *>(col);
        Var *var = makeVar(tc->GetRtIndex(), tc->GetAttrNum(), INT4OID, -1,
                           InvalidOid, 0); // Mock types for now
        tle = makeTargetEntry((Expr *)var,
                              (AttrNumber)list_length(target_list) + 1,
                              pstrdup("col"), false);
      } else if (col->GetType() == CarbonColumnType::EXPR_COLUMN) {
        auto *ec = static_cast<ExprColumn *>(col);
        tle = makeTargetEntry((Expr *)copyObjectImpl(ec->GetExpr()),
                              (AttrNumber)list_length(target_list) + 1,
                              pstrdup("expr"), false);
      }

      if (tle) {
        target_list = lappend(target_list, tle);
      }
    }
  }

  return target_list;
}

} // namespace pg_carbon
