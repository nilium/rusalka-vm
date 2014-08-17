-- Common functions for declaring projects in Premake4.
local __default_configurations = {
  default = {
    flags = { 'ExtraWarnings' },
  },

  macosx = {
    buildoptions = { '-std=gnu++11', '-stdlib=libc++' },
    links = { 'c++' },
  },

  Debug = {
    defines = { 'DEBUG' },
    flags = { 'Symbols' },
  },

  Release = {
    defines = { 'NDEBUG' },
    flags = { 'Optimize' },
  },
}

local _options = {
  files = files, excludes = excludes, buildoptions = buildoptions,
  flags = flags, includedirs = includedirs, defines = defines,
  libdirs = libdirs, links = links, linkoptions = linkoptions,
  targetname = targetname, targetdir = targetdir, targetprefix = targetprefix,
  targetsuffix = targetsuffix, targetextension = targetextension,
}


function __load_opt(opts, name, func)
  func = func or _G[name]
  value = opts[name]
  if value ~= nil then
    func(value)
  end
end


function load_configs(configs)
  if not configs then return end
  for k,v in pairs(configs) do
    if k == 'default' then k = {} end
    configuration(k)
    load_opts(v)
  end
end


function load_opts(opts)
  for k,v in pairs(_options) do
    __load_opt(opts, k, v)
  end
end


function __declare_project(opts, _kind)
  project((opts[1] or opts.name) or error('no project name given'))
  kind(_kind or error('no project kind given'))
  language(opts.language or 'C++')
  configuration {}
  load_opts(opts)
  load_configs(opts.configs)
  if opts.use_defaults == nil or opts.use_defaults then
    load_configs(__default_configurations)
  end
end


function gui_app(opts)
  __declare_project(opts, 'WindowedApp')
end


function console_app(opts)
  __declare_project(opts, 'ConsoleApp')
end


function shared_library(opts)
  __declare_project(opts, 'SharedLib')
end


function static_library(opts)
  __declare_project(opts, 'StaticLib')
end
