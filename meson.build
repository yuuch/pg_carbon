project('pg_carbon', 'c', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++17', 'warning_level=3'])

pg_config = find_program('pg_config', required: true)
incdir = run_command(pg_config, '--includedir', check: true).stdout().strip()
incdir_server = run_command(pg_config, '--includedir-server', check: true).stdout().strip()
pkglibdir = run_command(pg_config, '--pkglibdir', check: true).stdout().strip()
sharedir = run_command(pg_config, '--sharedir', check: true).stdout().strip()

inc = include_directories('.', 'src', incdir, incdir_server)

sources = files(
  'src/bridge/lib.c',
  'src/optimizer/optimizer.cpp',
  'src/optimizer/memo.cpp',
  'src/optimizer/scheduler.cpp',
  'src/optimizer/translator.cpp',
  'src/rules/rules.cpp',
  'src/operators/operators.cpp',
  'src/optimizer/preprocess.cpp',
)

# Shared module for the extension
item = shared_module('pg_carbon',
  sources,
  include_directories: inc,
  install: true,
  install_dir: pkglibdir,
  name_prefix: '',
  link_args: ['-Wl,-undefined,dynamic_lookup']
)

# Install control file
install_data('pg_carbon.control',
  install_dir: sharedir + '/extension'
)

# Install SQL file
install_data('pg_carbon--1.0.sql',
  install_dir: sharedir + '/extension'
)
