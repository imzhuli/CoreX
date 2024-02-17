alias rebuild='python3 _script/build_corex.py'
alias build_all='python3 _script/build_dependency.py; python3 _script/build_corex.py'
alias build_all_release='CMAKE_BUILD_TYPE=Release python3 _script/build_dependency.py; CMAKE_BUILD_TYPE=Release python3 _script/build_corex.py'
alias qb='cmake --build _corex_build -- install;'
