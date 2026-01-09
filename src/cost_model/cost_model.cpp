#include "cost_model.h"
#include <cmath>

namespace pg_carbon {

// Default costs consistent with PostgreSQL
#define PG_CARBON_SEQ_PAGE_COST        1.0
#define PG_CARBON_RANDOM_PAGE_COST     4.0
#define PG_CARBON_CPU_TUPLE_COST       0.01
#define PG_CARBON_CPU_INDEX_TUPLE_COST 0.005
#define PG_CARBON_CPU_OPERATOR_COST    0.0025

} // namespace pg_carbon

// External GUCs from Postgres
extern bool enable_seqscan;
extern bool enable_indexscan;

namespace pg_carbon {

double CostModel::CostSeqScan(double rows, double /*width*/, double pages) {
    // Basic PG Cost:
    // run_cost = cpu_tuple_cost * rows + seq_page_cost * pages
    // startup_cost = 0.0 (usually)
    double cpu_tuple_cost = PG_CARBON_CPU_TUPLE_COST;
    double seq_page_cost = PG_CARBON_SEQ_PAGE_COST;

    double cost = (cpu_tuple_cost * rows) + (seq_page_cost * pages);
    if (!enable_seqscan) {
        cost += 1.0e10;
    }
    return cost;
}

double CostModel::CostIndexScan(double rows, double /*width*/, double /*pages*/,
                                double index_pages) {
    // Very simplified Index Scan cost
    // PG is much more complex (startup cost based on tree height, selective blocks, etc.)
    // run_cost = cpu_index_tuple_cost * rows + random_page_cost * pages_fetched

    // We assume selective access touches a fraction of pages.
    // For Carbon demo, we'll try to follow:
    // Cost = cpu_index_tuple_cost * rows + random_page_cost * (index_pages_touched +
    // data_pages_touched)

    // Simplification:
    double cpu_index_tuple_cost = PG_CARBON_CPU_INDEX_TUPLE_COST;
    double random_page_cost = PG_CARBON_RANDOM_PAGE_COST;
    // Assume we touch 1 data page per row for random access (worst case)
    // and some index pages. This is loose.
    double cost = (cpu_index_tuple_cost * rows) + (random_page_cost * (index_pages + rows));
    if (!enable_indexscan) {
        cost += 1.0e10;
    }
    return cost;
}

double CostModel::CostFilter(double rows, double input_cost) {
    // Filter adds cpu_operator_cost per row
    double cpu_operator_cost = PG_CARBON_CPU_OPERATOR_COST;
    return input_cost + (cpu_operator_cost * rows);
}

double CostModel::CostProjection(double rows, double input_cost) {
    // Projection is usually cheap
    double cpu_operator_cost = PG_CARBON_CPU_OPERATOR_COST;
    return input_cost + (cpu_operator_cost * rows);
}

double CostModel::CostLimit(double input_cost) {
    // Limit is cheap, mostly about stopping early (which is handled by lower logical rows usually)
    // Structure overhead
    return input_cost + 1.0;
}

double CostModel::CostSort(double rows, double /*width*/, double input_cost) {
    // Sort : N log N * comparison_cost
    if (rows < 2.0)
        return input_cost;

    double cpu_operator_cost = PG_CARBON_CPU_OPERATOR_COST;
    double sort_cost = rows * std::log2(rows) * cpu_operator_cost;
    return input_cost + sort_cost;
}

double CostModel::CostAggregate(double rows, double input_cost) {
    // Agg: linear scan
    double cpu_operator_cost = PG_CARBON_CPU_OPERATOR_COST;
    return input_cost + (rows * cpu_operator_cost * 1.5);
}

double CostModel::CostNestedLoopJoin(double outer_rows, double outer_cost, double /*inner_rows*/,
                                     double inner_cost) {
    // NLJ: outer_cost + (outer_rows * inner_cost)
    double cost = outer_cost + (outer_rows * inner_cost);
    return cost;
}

} // namespace pg_carbon
