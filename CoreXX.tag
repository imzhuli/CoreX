alias rebuild='python3 _script/build_corex.py'
alias build_all='python3 _script/build_dependency.py; python3 _script/build_corex.py'
alias build_all_release='python3 _script/build_dependency.py -r;python3 _script/build_corex.py -r'
alias qb='cmake --build _corex_build'
alias qi='python3 _script/clean_install.py; cmake --install _corex_build'
alias rb='rebuild -j 24'

# windows:
# function rb3() { python3 _script/build_dependency.py }
# function rb()  { python3 _script/clean_install.py; python3 _script/build_corex.py }
# function qbd() { python3 _script/clean_install.py; cmake --build _corex_build --config=Debug; cmake --install _corex_build --config=Debug }
# function qbr() { python3 _script/clean_install.py; cmake --build _corex_build --config=Release; cmake --install _corex_build --config=Release }
