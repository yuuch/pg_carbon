#include "optimizer.h"
#include "memo.h"
#include "preprocess.h"
#include "scheduler.h"
#include "translator.h"

extern "C" {
#include "access/htup_details.h"
#include "catalog/pg_aggregate.h"
#include "catalog/pg_type.h"
#include "nodes/nodeFuncs.h"
#include "parser/parse_agg.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"
}

namespace pg_carbon {

// Optimize: Carbon Operator Tree (Root) -> Best Carbon Physical Plan
GroupExpression *Optimizer::Optimize(Operator *root_op) {
  // 1. Initialize Memo with the operator tree
  Group *root_group = memo_.InitMemo(root_op);

  // 2. Initialize Scheduler
  TaskScheduler scheduler;

  // 3. Schedule optimization of the root group
  // In a real system, we would pass required properties (e.g., sort order).
  scheduler.ScheduleTask(new O_Group(root_group, nullptr));

  // 4. Run Scheduler
  scheduler.Run();

  // 5. Extract best plan
  // In a real system, we extract based on required properties.
  // Here we just take the best expression stored in the group.
  auto best_expr = root_group->GetBestExpression();

  return best_expr;
}

} // namespace pg_carbon

extern "C" {
Plan *pg_carbon_optimize_query(Query *parse, int cursorOptions,
                               ParamListInfo boundParams) {
  // We ignore cursorOptions and boundParams for this skeleton

  // 0. Preprocess TargetList and Aggregates
  pg_carbon::Preprocess::PreprocessTargetList(parse);

  // 1. Translate PG Query -> Carbon Operator Tree
  pg_carbon::Translator translator;
  pg_carbon::Operator *root_op = translator.TranslateQueryToCarbon(parse);

  if (!root_op) {
    // Fallback or error
    return nullptr;
  }

  // 2. Optimization
  pg_carbon::Optimizer optimizer;
  pg_carbon::GroupExpression *best_plan = optimizer.Optimize(root_op);

  if (!best_plan) {
    return nullptr;
  }

  // 3. Translate Carbon Plan -> PG Plan
  Plan *plan =
      translator.TranslatePlanToPG(optimizer.GetMemo(), best_plan, parse);

  // Fix Aggref variables in the final plan
  // We need to traverse the plan to find Agg nodes, or just fix the root if we
  // know it's Agg. For now, let's fix the root's targetlist and qual if it has
  // them. Actually, FixVarsToOuter expects minimal Node* (TargetList or
  // expression). TranslatePlanToPG already called it on the specific Agg node
  // targetlist. Since we saw targetlist pointer change, we need to apply it to
  // the CURRENT targetlist of the plan.

  return plan;
}
}
