dofile 'build-aux.lua'

solution 'rusalka'
configurations {
  'Debug',
  'Release',
}


static_library {
  'rusalka',
  files = { '**.h', '**.inl', '**.cpp' },
  excludes = { '**_test.cpp', '**_test.h', '**_test.inl' },
}


console_app {
  'vm_test',
  files = { 'vm_test.cpp' },
  links = { 'rusalka' },
}
