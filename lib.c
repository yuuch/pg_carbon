#include "postgres.h"

#include "fmgr.h"
#include "optimizer/planner.h"
#include "utils/guc.h"

PG_MODULE_MAGIC;

/* GUC variables */
static bool pg_carbon_enable = true;

/* Saved hook */
static planner_hook_type prev_planner_hook = NULL;

/* Function declarations */
void _PG_init(void);
static PlannedStmt *pg_carbon_planner(Query *parse, const char *query_string,
                                      int cursorOptions,
                                      ParamListInfo boundParams);
static PlannedStmt *pg_carbon_optimize(Query *parse, const char *query_string,
                                       int cursorOptions,
                                       ParamListInfo boundParams);

/*
 * _PG_init
 * Entry point for the extension.
 */
void _PG_init(void) {
  ereport(WARNING, (errmsg("pg_carbon: _PG_init called")));
  /* Define generic GUCs */
  DefineCustomBoolVariable("pg_carbon.enable", "Enable pg_carbon optimizer.",
                           NULL, &pg_carbon_enable, true, PGC_USERSET, 0, NULL,
                           NULL, NULL);

  /* Install the hook */
  prev_planner_hook = planner_hook;
  planner_hook = pg_carbon_planner;
}

/*
 * pg_carbon_optimize
 * Stub for the Cascades optimizer entry point.
 * Currently returns NULL to simulate "not supported" or failure.
 */
static PlannedStmt *pg_carbon_optimize(Query *parse, const char *query_string,
                                       int cursorOptions,
                                       ParamListInfo boundParams) {
  /*
   * TODO: Implement the actual optimizer logic here.
   * For now, we just return NULL to force fallback.
   */
  return NULL;
}

/*
 * pg_carbon_planner
 * The main hook function. Tries pg_carbon, falls back to standard planner.
 */
static PlannedStmt *pg_carbon_planner(Query *parse, const char *query_string,
                                      int cursorOptions,
                                      ParamListInfo boundParams) {
  PlannedStmt *result = NULL;
  ereport(WARNING, (errmsg("pg_carbon: planner hook called")));

  if (pg_carbon_enable) {
    result =
        pg_carbon_optimize(parse, query_string, cursorOptions, boundParams);

    if (result == NULL) {
      /* Log that we tried but are falling back */
      ereport(WARNING, (errmsg("Cascades optimizer triggered! Falling back to "
                               "standard planner.")));
    }
  }

  if (result == NULL) {
    if (prev_planner_hook)
      return prev_planner_hook(parse, query_string, cursorOptions, boundParams);
    else
      return standard_planner(parse, query_string, cursorOptions, boundParams);
  }

  return result;
}
