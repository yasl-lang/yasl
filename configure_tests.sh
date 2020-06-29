echo 'static const char *inputs[] = {' > test2/inputs.inl
for f in $(find test/inputs -name '*.yasl'); do
    echo "  \"$f\"," >> test2/inputs.inl;
done;
echo '};' >> test2/inputs.inl

shopt -s globstar nullglob dotglob;

make_array () {
    declare name="$1";
    echo "static const char *${name}_errors[] = {" > test2/${name}_errors.inl
    for f in test/errors/$name/**/*.yasl; do
        echo "  \"$f\"," >> test2/${name}_errors.inl;
    done;
    echo '};' >> test2/${name}_errors.inl
}

make_array divisionbyzero;
make_array assert;
make_array value;
make_array stackoverflow;
make_array type;
