dofile 'build-aux.lua'

solution 'rusalka'
configurations {
  'Debug',
  'Release',
}


static_library {
  'rusalka',
  files = { '**.cpp' },
  excludes = { '**_test.*' },
}


console_app {
  'vm_test',
  files = { 'vm_test.cpp' },
  links = { 'rusalka' },
}
