dofile 'build-aux.lua'

solution 'rusalka'
configurations {
  'Debug-Static',
  'Debug-Shared',
  'Release-Static',
  'Release-Shared',
}

library {
  'rusalka',
  files = { '**.cpp' },
  excludes = { '**_test.*' },
  configs = {
    ['*-Static'] = { kind = 'StaticLib' },
    ['*-Shared'] = { kind = 'SharedLib' },
  },
}


console_app {
  'vm_test',
  files = { 'vm_test.cpp' },
  links = { 'rusalka' },
}
