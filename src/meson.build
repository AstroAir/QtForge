# QtForge Source Directory Build Configuration

# Core library sources (only include files that actually exist)
qtforge_core_sources = [
  'qtplugin.cpp',
  'utils/version.cpp',
  'utils/error_handling.cpp'
]

# Add conditional sources based on what actually exists
conditional_sources = [
  'core/plugin_interface.cpp',
  'core/plugin_manager.cpp',  # Re-enabled - should work with other build systems
  # 'core/plugin_manager_minimal.cpp',  # Temporarily disabled - incomplete type issues
  # 'core/plugin_manager_stub.cpp',  # Stub no longer needed
  # 'core/plugin_manager_moc.cpp',  # Removed - MOC implementation too complex
  'core/plugin_manager_factory_stub.cpp',  # Temporary stub for factory function
  'core/plugin_loader.cpp',
  'core/plugin_registry.cpp',
  'core/plugin_dependency_resolver.cpp',
  'core/plugin_lifecycle_manager.cpp',
  'monitoring/plugin_hot_reload_manager.cpp',
  'monitoring/plugin_metrics_collector.cpp',
  'communication/message_bus.cpp',
  'communication/request_response_system.cpp',
  'communication/factory.cpp',
  'communication/message_publisher.cpp',
  'communication/plugin_service_contracts.cpp',
  'communication/plugin_service_discovery.cpp',
  'communication/typed_event_system.cpp',

  'managers/configuration_manager.cpp',
  'managers/components/configuration_storage.cpp',
  'managers/components/configuration_validator.cpp',
  'managers/components/configuration_merger.cpp',
  'managers/components/configuration_watcher.cpp',
  'managers/logging_manager.cpp',
  'managers/plugin_version_manager.cpp',  # Re-enabled - should work with other build systems
  # 'managers/plugin_version_manager_stub.cpp',  # Stub no longer needed
  'managers/resource_manager.cpp',
  'managers/resource_lifecycle.cpp',
  'managers/components/resource_pool.cpp',
  'managers/components/resource_allocator.cpp',
  'managers/resource_monitor.cpp',
  # Advanced plugin system sources (v3.1.0)
  'communication/plugin_service_contracts.cpp',
  'core/advanced_plugin_interface.cpp',
  # Workflow module sources (moved from orchestration/composition)
  'workflow/error_recovery.cpp',
  'workflow/rollback_manager.cpp',
  'workflow/state_persistence.cpp',
  'workflow/orchestration.cpp',
  'workflow/progress_tracking.cpp',
  'workflow/transactions.cpp',
  'workflow/composition.cpp',
  'workflow/integration.cpp',
  'workflow/workflow_manager.cpp',
  'workflow/progress_monitoring.cpp',
  'workflow/progress_message_bus.cpp',
  'workflow/transaction_error_handler.cpp',
  'workflow/workflow_validator.cpp',
  'transactions/plugin_transaction_manager.cpp',
  # Version management sources (v3.1.0)
  'managers/plugin_version_manager.cpp',
  # Dynamic plugin system sources (v3.2.0)
  'core/dynamic_plugin_interface.cpp',
  'core/plugin_capability_discovery.cpp',
  'core/plugin_property_system.cpp',
  # Multi-language plugin bridges (v3.2.0)
  'bridges/python_plugin_bridge.cpp',
  'bridges/lua_plugin_bridge.cpp',
  # Remote plugin system
  'remote/http_plugin_loader.cpp',
  'remote/plugin_download_manager.cpp',
  'remote/remote_plugin_configuration.cpp',
  'remote/remote_plugin_discovery.cpp',
  'remote/remote_plugin_loader.cpp',
  'remote/remote_plugin_manager.cpp',
  'remote/remote_plugin_manager_extension.cpp',
  'remote/remote_plugin_registry_extension.cpp',
  'remote/remote_plugin_source.cpp',
  'remote/remote_plugin_validator.cpp',
  'remote/unified_plugin_manager.cpp',
  # Security components
  'security/components/permission_manager.cpp',
  'security/components/security_policy_engine.cpp',
  'security/components/security_validator.cpp',
  'security/components/signature_verifier.cpp'

]

# Only add sources that actually exist
foreach src : conditional_sources
  if fs.exists(src)
    qtforge_core_sources += [src]
  endif
endforeach

# Conditionally add Lua-related sources if Lua bindings are enabled
# Temporarily disabled due to compilation issues with sol2 integration
# if get_option('lua_bindings') and lua_dep.found() and sol2_dep.found()
#   lua_sources = [
#     'bridges/lua_plugin_bridge.cpp',
#     'core/lua_plugin_loader.cpp'
#   ]
#
#   foreach src : lua_sources
#     if fs.exists(src)
#       qtforge_core_sources += [src]
#       message('QtForge: Including Lua plugin source: @0@'.format(src))
#     endif
#   endforeach
#   message('QtForge: Including Lua plugin sources in core library')
# else
  message('QtForge: Excluding Lua plugin sources from core library (temporarily disabled)')
# endif

# Core library headers that need MOC processing (only headers with actual Q_OBJECT classes)
qtforge_core_moc_headers = [
  # Start with a minimal set to test the build
  '../include/qtplugin/communication/message_bus.hpp',
  '../include/qtplugin/communication/request_response_system.hpp',
  '../include/qtplugin/managers/configuration_manager_impl.hpp',
  '../include/qtplugin/managers/logging_manager_impl.hpp',
  '../include/qtplugin/managers/resource_manager_impl.hpp',
  '../include/qtplugin/managers/resource_lifecycle_impl.hpp',
  '../include/qtplugin/managers/resource_monitor_impl.hpp',
  '../include/qtplugin/managers/components/configuration_storage.hpp',
  '../include/qtplugin/managers/components/configuration_merger.hpp',
  '../include/qtplugin/managers/components/configuration_watcher.hpp',
  '../include/qtplugin/managers/components/resource_allocator.hpp',
  '../include/qtplugin/managers/components/resource_pool.hpp',  # Added for ResourcePoolBase
  '../include/qtplugin/monitoring/plugin_hot_reload_manager.hpp',
  '../include/qtplugin/monitoring/plugin_metrics_collector.hpp',

  # Add core headers back - we've fixed the signal emission issues
  '../include/qtplugin/core/plugin_registry.hpp',
  '../include/qtplugin/core/plugin_dependency_resolver.hpp',
  '../include/qtplugin/core/plugin_lifecycle_manager.hpp',
  # Add remaining headers needed for vtables
  '../include/qtplugin/managers/components/configuration_validator.hpp',
  # '../include/qtplugin/managers/plugin_version_manager.hpp',  # Removed due to MOC issues
  # '../include/qtplugin/core/plugin_manager.hpp',  # Disabled again - MOC has incomplete type issues
]

# Filter MOC headers to only include those that actually exist
qtforge_existing_moc_headers = []
foreach header : qtforge_core_moc_headers
  if fs.exists(header)
    qtforge_existing_moc_headers += [header]
  endif
endforeach

# All core library headers (combine MOC headers with other headers)
qtforge_core_headers = qtforge_existing_moc_headers + [
  '../include/qtplugin/interfaces/core/plugin_interface.hpp',
  '../include/qtplugin/interfaces/core/service_plugin_interface.hpp',
  '../include/qtplugin/utils/version.hpp',
  '../include/qtplugin/utils/error_handling.hpp',

  '../include/qtplugin/components.hpp'
]

# Add conditional headers that exist
conditional_headers = [
  '../include/qtplugin/communication/message_types.hpp',
  '../include/qtplugin/communication/plugin_service_contracts.hpp',
  '../include/qtplugin/managers/plugin_version_manager.hpp',
  '../include/qtplugin/managers/resource_pools.hpp',
  '../include/qtplugin/managers/components/configuration_validator.hpp',
  # Advanced plugin system headers (v3.1.0)
  '../include/qtplugin/interfaces/core/advanced_plugin_interface.hpp',
  '../include/qtplugin/orchestration/plugin_orchestrator.hpp',
  '../include/qtplugin/composition/plugin_composition.hpp',
  '../include/qtplugin/workflow/transactions.hpp',
  # Dynamic plugin system headers (v3.2.0)
  '../include/qtplugin/interfaces/core/dynamic_plugin_interface.hpp',
  # Multi-language plugin bridge headers (v3.2.0)
  '../include/qtplugin/bridges/python_plugin_bridge.hpp',

]

foreach header : conditional_headers
  if fs.exists(header)
    qtforge_core_headers += [header]
  endif
endforeach

# Conditionally add Lua-related headers if Lua bindings are enabled
if get_option('lua_bindings') and lua_dep.found() and sol2_dep.found()
  lua_headers = [
    '../include/qtplugin/bridges/lua_plugin_bridge.hpp',
    '../include/qtplugin/core/lua_plugin_loader.hpp'
  ]

  foreach header : lua_headers
    if fs.exists(header)
      qtforge_core_headers += [header]
    endif
  endforeach
endif

# Qt preprocessing for MOC files (only if we have headers to process)
qtforge_core_moc_files = []
if qtforge_existing_moc_headers.length() > 0
  qtforge_core_moc_files = qt6.compile_moc(
    headers: qtforge_existing_moc_headers,
    dependencies: [qt6_core_dep],
    include_directories: [inc_dir, src_inc_dir]
  )
endif

# Collect Qt dependencies for core library
qtforge_core_qt_deps = [qt6_core_dep]
if qt6_network_dep.found()
  qtforge_core_qt_deps += [qt6_network_dep]
endif
if qt6_widgets_dep.found()
  qtforge_core_qt_deps += [qt6_widgets_dep]
endif
if qt6_sql_dep.found()
  qtforge_core_qt_deps += [qt6_sql_dep]
endif
if qt6_concurrent_dep.found()
  qtforge_core_qt_deps += [qt6_concurrent_dep]
endif
if qt6_statemachine_dep.found()
  qtforge_core_qt_deps += [qt6_statemachine_dep]
endif

# Core library compile definitions
qtforge_core_definitions = []
if get_option('default_library') == 'shared'
  qtforge_core_definitions += ['-DQTFORGE_CORE_EXPORTS']
endif

# Feature-based conditional compilation
if get_option('enable_hot_reload')
  qtforge_core_definitions += ['-DQTFORGE_ENABLE_HOT_RELOAD']
endif
if get_option('enable_metrics')
  qtforge_core_definitions += ['-DQTFORGE_ENABLE_METRICS']
endif
if get_option('enable_transactions')
  qtforge_core_definitions += ['-DQTFORGE_ENABLE_TRANSACTIONS']
endif
if get_option('enable_orchestration')
  qtforge_core_definitions += ['-DQTFORGE_ENABLE_ORCHESTRATION']
endif
if get_option('enable_composition')
  qtforge_core_definitions += ['-DQTFORGE_ENABLE_COMPOSITION']
endif


# Create QtForgeCore library
qtforge_core_lib = library('qtforge-core',
  qtforge_core_sources,
  qtforge_core_moc_files,
  include_directories: [inc_dir, src_inc_dir],
  dependencies: qtforge_core_qt_deps,
  cpp_args: qtforge_core_definitions,
  version: meson.project_version(),
  soversion: version_major,
  install: true,
  install_dir: get_option('libdir')
)

# Create dependency object for QtForgeCore
qtforge_core_dep = declare_dependency(
  link_with: qtforge_core_lib,
  include_directories: inc_dir,
  dependencies: qtforge_core_qt_deps
)

# Security library removed

# Generate pkg-config files
pkgconfig.generate(qtforge_core_lib,
  name: 'QtForgeCore',
  description: 'QtForge Core Library - Modern C++ Plugin System for Qt Applications',
  url: project_url,
  version: meson.project_version(),
  requires: ['Qt6Core'],
  requires_private: qt6_network_dep.found() ? ['Qt6Network'] : [],
  subdirs: ['qtplugin']
)



# Python bindings (temporarily disabled due to binding issues)
# if get_option('python_bindings') and python3_dep.found() and pybind11_dep.found()
#   subdir('python')
# endif

# Lua bindings (if enabled)
if get_option('lua_bindings') and lua_dep.found() and sol2_dep.found()
  subdir('lua')
endif
