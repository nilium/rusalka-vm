solution 'rusalka'
configurations {'Debug', 'Release'}

project 'rusalka'
do
  kind 'StaticLib'
  language 'C++'
  flags { 'ExtraWarnings' }

  files { '**.h', '**.inl', '**.cpp' }
  excludes { '**_test.cpp', '**_test.h', '**_test.inl' }

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


project 'vm_test'
do
  kind 'ConsoleApp'
  language 'C++'

  files { 'vm_test.cpp' }
  links { 'rusalka' }

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
