{
  'targets': [
    {
      'target_name': 'raw_buffer',
      'sources': [
        'main.cpp',
      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ],
      "conditions" : [
        ['"<!(echo $LUMIN)"=="1"', {
          'defines': ['LUMIN'],
        }],
        ['OS=="android"', {
          "cflags_cc":["-fPIC"],
        }],
      ],
    }
  ]
}
