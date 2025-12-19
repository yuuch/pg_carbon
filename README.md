# pg_carbon

A Cascades-style Cost-Based Optimizer (CBO) extension for PostgreSQL. `pg_carbon` is designed to provide an alternative query optimization path using the Cascades framework, allowing for more extensible and potentially efficient query planning.

![Architecture](img/carbon_arch.png)


## Prerequisites

Before building `pg_carbon`, ensure you have the following installed:

*   **PostgreSQL**: (Development headers and libraries required)
*   **Meson**: (>= 0.57.0)
*   **Ninja**: Build system

## Installation

`pg_carbon` uses the Meson build system. Follow these steps to compile and install the extension:

1.  **Setup the build directory:**

    ```bash
    meson setup build
    ```

    If `pg_config` is not in your PATH, you may need to specify it:

    ```bash
    meson setup build -Dpg_config=/path/to/pg_config
    ```

2.  **Compile the extension:**

    ```bash
    cd build
    ninja
    ```

3.  **Install:**

    ```bash
    ninja install
    ```

## Usage

### Configuration

To enable `pg_carbon`, you need to load it as a shared library.

1.  Add `pg_carbon` to `shared_preload_libraries` in your `postgresql.conf`:

    ```ini
    shared_preload_libraries = 'pg_carbon'
    ```

2.  Restart your PostgreSQL server.

### Loading the Extension

Connect to your database and create the extension:

```sql
CREATE EXTENSION pg_carbon;
```

### Options

`pg_carbon` provides GUC (Grand Unified Configuration) variables to control its behavior:

*   `pg_carbon.enable` (boolean): Enables or disables the `pg_carbon` optimizer. Default is `on`.

    ```sql
    -- Disable pg_carbon for the current session
    SET pg_carbon.enable = off;
    ```

## How it works

When enabled, `pg_carbon` hooks into the PostgreSQL planner. It attempts to optimize the query using its Cascades-style engine. If `pg_carbon` cannot handle a specific query or if optimization fails (currently, the optimizer is a stub that always returns `NULL`), it gracefully falls back to the standard PostgreSQL planner, ensuring query execution functionality is preserved.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

