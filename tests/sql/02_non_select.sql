LOAD 'pg_carbon';
SET pg_carbon.enable = on;
SET client_min_messages = warning;

-- This is a non-SELECT query, should NOT trigger pg_carbon warning/logic
CREATE TEMP TABLE t_non_select (a int);

-- This is also non-SELECT (CMD_INSERT)
INSERT INTO t_non_select VALUES (1);

-- Verify data (This IS a SELECT, so it might trigger pg_carbon if enabled)
SELECT * FROM t_non_select;
