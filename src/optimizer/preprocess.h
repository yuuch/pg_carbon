#ifndef PG_CARBON_OPTIMIZER_PREPROCESS_H
#define PG_CARBON_OPTIMIZER_PREPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "postgres.h"
struct _dummy;
#include "nodes/parsenodes.h"
#include "nodes/pathnodes.h"
// #include "nodes/pg_list.h" // Removed to match optimizer.h pattern,
// theoretically pulled by others

#ifdef __cplusplus
}
#endif

namespace pg_carbon {

struct PreprocessingContext {
  Query *parse;
};

class Preprocess {
public:
  static void PreprocessTargetList(Query *parse);
};

} // namespace pg_carbon

#endif // PG_CARBON_OPTIMIZER_PREPROCESS_H
