if test -f ./exec; then
    rm ./exec
fi

./build.sh

if test -f ./exec; then
    ./exec
else
    echo "File not built successfully"
fi