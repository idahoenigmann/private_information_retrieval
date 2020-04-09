project('private_information_retrieval',
        'cpp',
        version: '0.0.1',
        meson_version: '>0.46',
        default_options : ['cpp_std=c++2a', 'warning_level=3', 'werror=true'])

inc_dir = include_directories(['include', get_option('asio'), get_option('spdlog'), get_option('cli')])
src_client = ['src/client/main.cpp']

src_server = ['src/server/main.cpp']

thread_dep = dependency('threads')

executable('client',
           sources : src_client,
           include_directories : inc_dir,
           dependencies: thread_dep)

executable('server',
           sources : src_server,
           include_directories : inc_dir,
           dependencies: thread_dep)