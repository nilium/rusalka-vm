-- Common functions for declaring projects in Premake4.
local __release_options = {
  defines = { 'NDEBUG' },
  flags = { 'Optimize' },
}

local __debug_options = {
  defines = { 'DEBUG' },
  flags = { 'Symbols' },
}

local __default_configurations = {
  default = {
    flags = { 'ExtraWarnings' },
  },

  macosx = {
    buildoptions = { '-std=gnu++11', '-stdlib=libc++', '-fno-lto' },
    linkoptions = { '-fno-lto' },
    links = { 'c++' },
  },

  Debug = __debug_options,
  Release = __release_options,
  ['Debug-*'] = __debug_options,
  ['Release-*'] = __release_options,
}

local __skipped_keys = { use_defaults = true, [1] = true }

local _options = {
  files = files, excludes = excludes, buildoptions = buildoptions,
  flags = flags, includedirs = includedirs, defines = defines,
  libdirs = libdirs, links = links, linkoptions = linkoptions,
  targetname = targetname, targetdir = targetdir, targetprefix = targetprefix,
  targetsuffix = targetsuffix, targetextension = targetextension,
}


function __load_opt(name, value, func)
  if type(value) == 'function' then
    value()
    return
  elseif type(name) ~= 'string' then
    return
  elseif __skipped_keys[name] then
    return
  end

  if func == nil then
    func = _options[name] or _G[name]
  end

  if func ~= nil then
    func(value)
  else
    print("Undefined option " .. name .. " (" .. tostring(value) .. ")")
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
  for k,v in pairs(opts) do
    if k ~= "configs" then
      __load_opt(k, v)
    end
  end
end


function __declare_project(opts, _kind)
  project((opts[1] or opts.name) or error('no project name given'))
  if _kind ~= nil then
    kind(_kind)
  end
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

function library(opts)
  __declare_project(opts, nil)
end
