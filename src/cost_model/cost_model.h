#ifndef PG_CARBON_COST_MODEL_H
#define PG_CARBON_COST_MODEL_H

#include "../metadata/metadata.h"

namespace pg_carbon {

class CostModel {
public:
    CostModel(MetadataAccessor *metadata) : metadata_(metadata) {}
    ~CostModel() = default;

    double CostSeqScan(double rows, double width, double pages);
    double CostIndexScan(double rows, double width, double pages, double index_pages);
    double CostFilter(double rows, double input_cost);
    double CostProjection(double rows, double input_cost);
    double CostLimit(double input_cost);
    double CostSort(double rows, double width, double input_cost);
    double CostAggregate(double rows, double input_cost);
    double CostNestedLoopJoin(double outer_rows, double outer_cost, double inner_rows,
                              double inner_cost);

private:
    MetadataAccessor *metadata_;
};

} // namespace pg_carbon

#endif // PG_CARBON_COST_MODEL_H
