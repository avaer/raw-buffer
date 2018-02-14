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
    }
  ]
}
