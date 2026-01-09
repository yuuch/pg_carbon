LOAD 'pg_carbon';
SET pg_carbon.enable = on;
SET client_min_messages = warning;

-- Setup
DROP TABLE IF EXISTS scan_test;
CREATE TABLE scan_test (id int, val int);
INSERT INTO scan_test SELECT i, i % 100 FROM generate_series(1, 100000) i;
ANALYZE scan_test;

-- Test 1: Seq Scan
-- Without index, it must be Seq Scan
EXPLAIN SELECT * FROM scan_test WHERE val = 50;
SELECT count(*) FROM scan_test WHERE val = 50;

-- Test 2: Index Scan
CREATE INDEX idx_scan_test_val ON scan_test(val);
ANALYZE scan_test;

-- Usually forcing index scan is safer for testing specific path if data is small
SET enable_seqscan = on;
EXPLAIN SELECT * FROM scan_test WHERE val = 50;
SET enable_seqscan = off;
EXPLAIN SELECT * FROM scan_test WHERE val = 50;
SELECT count(*) FROM scan_test WHERE val = 50;
SET enable_seqscan = on;

-- Clean up
DROP TABLE scan_test;
