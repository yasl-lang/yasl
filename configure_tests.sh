echo 'static const char *inputs[] = {' > test/inputs.inl
for f in $(find test/inputs -name '*.yasl'); do
    echo "  \"$f\"," >> test/inputs.inl;
done;
echo '};' >> test/inputs.inl

shopt -s globstar nullglob dotglob;

make_array () {
    declare name="$1";
    echo "static const char *${name}_errors[] = {" > test/${name}_errors.inl
    for f in test/errors/$name/**/*.yasl; do
        echo "  \"$f\"," >> test/${name}_errors.inl;
    done;
    echo '};' >> test/${name}_errors.inl
}

make_array divisionbyzero;
make_array assert;
make_array value;
make_array stackoverflow;
make_array type;
make_array syntax;
