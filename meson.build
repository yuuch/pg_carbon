project('pg_carbon', 'c',
  version: '0.0.1',
  license: 'PostgreSQL',
  meson_version: '>=0.57.0',
)

pg_config = find_program('pg_config', required: false)
if not pg_config.found()
  error('pg_config not found. Please install PostgreSQL or set the PATH to include pg_config.')
endif

pg_includedir = run_command(pg_config, '--includedir', check: true).stdout().strip()
pg_pkglibdir = run_command(pg_config, '--pkglibdir', check: true).stdout().strip()
pg_server_includedir = run_command(pg_config, '--includedir-server', check: true).stdout().strip()

include_directories = include_directories('.', pg_server_includedir)

pg_carbon_sources = files(
  'lib.c',
)

shared_module('pg_carbon',
  pg_carbon_sources,
  include_directories: include_directories,
  install: true,
  install_dir: pg_pkglibdir,
  name_prefix: '',
)

install_data('pg_carbon.control', 'pg_carbon--0.0.1.sql',
  install_dir: join_paths(run_command(pg_config, '--sharedir', check: true).stdout().strip(), 'extension')
)
