program_name="base_layer_test"
build_dir="./build"

sources="./src/*.cpp"
debug_opts="-O0 -g -DDEBUG"
release_opts="-O2 -DRELEASE"
comp_opts="-Wall -Wextra -std=c++20 -fno-threadsafe-statics -fno-exceptions -fno-rtti -march=x86-64-v3"
link_opts="-nostdinc++ -nostdlib++ -lc -ldl -lm -lpthread"

if [ "$1" == "debug" ]; then
    echo "--- Building in debug mode ---"
    comp_opts="$comp_opts $debug_opts"
elif [ "$1" == "release" ]; then
    echo "--- Building in release mode ---"
    comp_opts="$comp_opts $release_opts"
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

echo "--- Building $program_name ---"
echo "$comp_opts $sources $link_opts"
clang $comp_opts $sources $link_opts -o "$build_dir/$program_name"

exit $?
