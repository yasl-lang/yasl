echo 'static const char *outputs[] = {' > test2/outputs.inl
for f in $(find test/inputs -name '*.yasl'); do
    echo "  \"$f\"," >> test2/outputs.inl;
done;
echo '};' >> test2/outputs.inl

echo 'static const char *assert_errors[] = {' > test2/assert_errors.inl
for f in $(find test/errors/assert -name '*.yasl'); do
    echo "  \"$f\"," >> test2/assert_errors.inl;
done;
echo '};' >> test2/assert_errors.inl
