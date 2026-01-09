#include "optimizer.h"
#include "../translator/translator.h"
#include "memo/memo.h"
#include "tasks/opt_grp.h"
#include "tasks/scheduler.h"

extern "C" {
#include "access/htup_details.h"
#include "catalog/pg_aggregate.h"
#include "catalog/pg_type.h"
#include "nodes/nodeFuncs.h"
#include "parser/parse_agg.h"
#include "utils/lsyscache.h"
#include "utils/syscache.h"
}

#include "../metadata/metadata.h"
#include "../operators/logical_filter.h"

namespace pg_carbon {

// Optimize: Carbon Operator Tree (Root) -> Best Carbon Physical Plan
GroupExpression *Optimizer::Optimize(Expression *root_expr, Query *pg_query,
                                     MetadataAccessor *metadata) {
    // 1. Initialize Memo with the expression tree
    memo_.SetMetadataAccessor(metadata);
    metadata->SetQuery(pg_query);

    // Copy Initial Plan to Memo
    Group *root_group = memo_.InitMemo(root_expr);

    // 2. Initialize Scheduler
    TaskScheduler scheduler;

    // 3. Schedule optimization of the root group
    Context *context = new Context(&memo_);
    scheduler.ScheduleTask(new OptGrp(root_group, context));

    // 4. Run Scheduler
    scheduler.Run();

    // 5. Extract best plan
    auto best_expr = root_group->GetBestExpression();

    return best_expr;
}

} // namespace pg_carbon

extern "C" {
Plan *pg_carbon_optimize_query(Query *parse, int cursorOptions, ParamListInfo boundParams) {
    // We ignore cursorOptions and boundParams for this skeleton

    // 1. Translate PG Query -> Carbon Operator Tree
    pg_carbon::Translator translator;
    pg_carbon::Expression *root_expr = translator.TranslateQueryToCarbon(parse);

    if (!root_expr) {
        // Fallback or error
        return nullptr;
    }

    // 2. Optimization
    pg_carbon::Optimizer optimizer;
    pg_carbon::MetadataAccessor metadata; // Instantiate on stack
    pg_carbon::GroupExpression *best_plan = optimizer.Optimize(root_expr, parse, &metadata);

    if (!best_plan) {
        return nullptr;
    }

    // 3. Translate Carbon Plan -> PG Plan
    Plan *plan = translator.TranslatePlanToPG(optimizer.GetMemo(), best_plan, parse);

    return plan;
}
}
