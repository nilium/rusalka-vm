solution 'rusalka'
configurations {'Debug', 'Release'}

project 'vm_test'
do
  kind 'ConsoleApp'
  language 'C++'

  files { '**.h', '**.inl', '**.cpp' }

  flags { 'ExtraWarnings' }

  configuration 'macosx'
  do
    buildoptions { '-std=gnu++11', '-stdlib=libc++' }
    links { 'c++' }
  end

  configuration 'Debug'
  do
    defines { 'DEBUG' }
    flags { 'Symbols' }
  end

  configuration 'Release'
  do
    defines { 'NDEBUG' }
    flags { 'Optimize' }
  end
end
