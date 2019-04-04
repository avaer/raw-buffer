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
        ['"<!(echo $ANDROID)"=="1"', {
          # "cflags_cc":["-fPIC"],
          'defines': ['ANDROID'],
        }],
      ],
    }
  ]
}
