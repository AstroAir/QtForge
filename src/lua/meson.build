# QtForge Lua Bindings Build Configuration

# Lua bindings are only built if explicitly requested and dependencies are available
if not get_option('lua_bindings') or not lua_dep.found() or not sol2_dep.found()
  subdir_done()
endif

message('Building QtForge Lua bindings')

# Lua binding sources - minimal components only (matching CMake configuration)
lua_binding_sources = [
  'qtforge_lua.cpp',
  # 'qt_conversions.cpp',  # Temporarily disabled due to Qt dependencies
  'core/core_bindings.cpp',
  # 'core/metadata_bindings.cpp',  # Temporarily disabled due to Qt dependencies
  'utils/utils_bindings.cpp',
  # 'utils/error_handling_bindings.cpp'  # Temporarily disabled due to Qt dependencies
]

# Add optional binding sources based on available Qt components
# Temporarily disabled to focus on core bindings
# lua_binding_sources += [
#   'security/security_bindings.cpp',
#   'managers/managers_bindings.cpp',
#   'orchestration/orchestration_bindings.cpp',
#   'monitoring/monitoring_bindings.cpp',
#   'transactions/transaction_bindings.cpp',
#   'composition/composition_bindings.cpp',
#   'marketplace/marketplace_bindings.cpp',
#   'threading/threading_bindings.cpp'
# ]

# Communication bindings (requires Qt6::Network for some features)
# Temporarily disabled to focus on core bindings
# if qt6_network_dep.found()
#   lua_binding_sources += ['communication/communication_bindings.cpp']
#   message('QtForge: Including Lua communication bindings')
# else
#   message('QtForge: Skipping Lua communication bindings (Qt6::Network not found)')
# endif

# Lua binding dependencies
lua_binding_deps = [
  lua_dep
]

# Ultra-minimal bindings - only link Lua libraries for now
# qtforge_core_dep,
# qt6_core_dep

# Add optional Qt dependencies if available
# if qt6_network_dep.found()
#   lua_binding_deps += [qt6_network_dep]
# endif
# if qt6_widgets_dep.found()
#   lua_binding_deps += [qt6_widgets_dep]
# endif
# if qt6_sql_dep.found()
#   lua_binding_deps += [qt6_sql_dep]
# endif

# Lua binding compile definitions
lua_binding_definitions = [
  '-DQTPLUGIN_VERSION_MAJOR=@0@'.format(version_major),
  '-DQTPLUGIN_VERSION_MINOR=@0@'.format(version_minor),
  '-DQTPLUGIN_VERSION_PATCH=@0@'.format(version_patch),
  '-DQTFORGE_LUA_BINDINGS'
]

# Add sandboxing support if enabled
if get_option('lua_enable_sandbox')
  lua_binding_definitions += ['-DQTFORGE_LUA_SANDBOX_ENABLED']
endif

# Add feature-based definitions
# if qt6_network_dep.found()
#   lua_binding_definitions += ['-DQTFORGE_NETWORK_AVAILABLE']
# endif
# if qt6_widgets_dep.found()
#   lua_binding_definitions += ['-DQTFORGE_WIDGETS_AVAILABLE']
# endif
# if qt6_sql_dep.found()
#   lua_binding_definitions += ['-DQTFORGE_SQL_AVAILABLE']
# endif

# Include directories
lua_include_dirs = [inc_dir, src_inc_dir, include_directories('.')]

# Add sol2 include directory if found manually
if sol2_dep.found()
  # sol2 include directory is handled by the dependency
else
  # Try to add manual sol2 include directory
  sol2_include_dir = get_option('prefix') / 'include'
  lua_include_dirs += [include_directories(sol2_include_dir)]
endif

# Create Lua module
qtforge_lua_lib = shared_library('qtforge_lua',
  lua_binding_sources,
  include_directories: lua_include_dirs,
  dependencies: lua_binding_deps,
  cpp_args: lua_binding_definitions,
  version: meson.project_version(),
  soversion: version_major,
  install: get_option('lua_bindings_install'),
  install_dir: get_option('libdir')
)

# Create dependency for Lua bindings
qtforge_lua_dep = declare_dependency(
  link_with: qtforge_lua_lib,
  include_directories: inc_dir,
  dependencies: lua_binding_deps
)

message('QtForge: Lua bindings configured successfully')
