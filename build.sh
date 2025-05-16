program_name="base_layer_test"
build_dir="./build"

compiler="clang"

sources="./src/main.cpp"
common_opts="-Wall -std=c++20 -nostdinc++ -nostdlib++"
debug_opts="-O0 -g -DDEBUG"
release_opts="-O2 -DRELEASE"

if [ "$1" == "debug" ]; then
    echo "--- Building in debug mode ---"
    mode_opts=$debug_opts
elif [ "$1" == "release" ]; then
    echo "--- Building in release mode ---"
    mode_opts=$release_opts
elif [ "$1" == "run" ]; then
    echo "--- Running $program_name ---"
    $build_dir/$program_name
    exit $?
else
    echo "Invalid command. Use 'debug', 'release' or 'run'"
    exit -1
fi

if [ ! -d "$build_dir" ]; then
    mkdir $build_dir
fi

$compiler $common_opts $mode_opts -o "$build_dir/$program_name" $sources

exit $?