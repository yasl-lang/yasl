static const char *inputs[] = {
  "test/inputs/clear.yasl",
  "test/inputs/lambdas/ambiguous.yasl",
  "test/inputs/lambdas/emptyreturn.yasl",
  "test/inputs/lambdas/nested.yasl",
  "test/inputs/lambdas/basic.yasl",
  "test/inputs/builtin-types/bool/operators.yasl",
  "test/inputs/builtin-types/bool/tostr.yasl",
  "test/inputs/builtin-types/table/clear.yasl",
  "test/inputs/builtin-types/table/remove.yasl",
  "test/inputs/builtin-types/table/__len.yasl",
  "test/inputs/builtin-types/table/__eq.yasl",
  "test/inputs/builtin-types/table/copy.yasl",
  "test/inputs/builtin-types/table/__bor.yasl",
  "test/inputs/builtin-types/table/tostr.yasl",
  "test/inputs/builtin-types/table/operator_overloading_right.yasl",
  "test/inputs/builtin-types/table/__set.yasl",
  "test/inputs/builtin-types/table/operator_overloading.yasl",
  "test/inputs/builtin-types/table/values.yasl",
  "test/inputs/builtin-types/table/keys.yasl",
  "test/inputs/builtin-types/table/memberfn.yasl",
  "test/inputs/builtin-types/table/comp_op_overloading.yasl",
  "test/inputs/builtin-types/table/iadd.yasl",
  "test/inputs/builtin-types/table/comp_op_overloading_right.yasl",
  "test/inputs/builtin-types/table/tables.yasl",
  "test/inputs/builtin-types/int/operators.yasl",
  "test/inputs/builtin-types/int/literals.yasl",
  "test/inputs/builtin-types/int/concat_3.yasl",
  "test/inputs/builtin-types/int/decimal.yasl",
  "test/inputs/builtin-types/int/conversions.yasl",
  "test/inputs/builtin-types/int/hex.yasl",
  "test/inputs/builtin-types/int/tostr-format.yasl",
  "test/inputs/builtin-types/int/binary.yasl",
  "test/inputs/builtin-types/int/smallints.yasl",
  "test/inputs/builtin-types/str/operators.yasl",
  "test/inputs/builtin-types/str/literals.yasl",
  "test/inputs/builtin-types/str/startswith.yasl",
  "test/inputs/builtin-types/str/count.yasl",
  "test/inputs/builtin-types/str/ltrim.yasl",
  "test/inputs/builtin-types/str/slice.yasl",
  "test/inputs/builtin-types/str/split.yasl",
  "test/inputs/builtin-types/str/rep.yasl",
  "test/inputs/builtin-types/str/withnull.yasl",
  "test/inputs/builtin-types/str/replace.yasl",
  "test/inputs/builtin-types/str/slice_mt.yasl",
  "test/inputs/builtin-types/str/dot.yasl",
  "test/inputs/builtin-types/str/rtrim.yasl",
  "test/inputs/builtin-types/str/spread.yasl",
  "test/inputs/builtin-types/str/interpolation.yasl",
  "test/inputs/builtin-types/str/__get.yasl",
  "test/inputs/builtin-types/str/tostr.yasl",
  "test/inputs/builtin-types/str/is.yasl",
  "test/inputs/builtin-types/str/partition.yasl",
  "test/inputs/builtin-types/str/trim.yasl",
  "test/inputs/builtin-types/str/concat.yasl",
  "test/inputs/builtin-types/str/search.yasl",
  "test/inputs/builtin-types/str/strings.yasl",
  "test/inputs/builtin-types/str/endswith.yasl",
  "test/inputs/builtin-types/str/to.yasl",
  "test/inputs/builtin-types/list/clear.yasl",
  "test/inputs/builtin-types/list/insert.yasl",
  "test/inputs/builtin-types/list/pop.yasl",
  "test/inputs/builtin-types/list/count.yasl",
  "test/inputs/builtin-types/list/remove.yasl",
  "test/inputs/builtin-types/list/sort.yasl",
  "test/inputs/builtin-types/list/slice.yasl",
  "test/inputs/builtin-types/list/__len.yasl",
  "test/inputs/builtin-types/list/__eq.yasl",
  "test/inputs/builtin-types/list/copy.yasl",
  "test/inputs/builtin-types/list/reverse.yasl",
  "test/inputs/builtin-types/list/__get.yasl",
  "test/inputs/builtin-types/list/__add.yasl",
  "test/inputs/builtin-types/list/tostr.yasl",
  "test/inputs/builtin-types/list/push.yasl",
  "test/inputs/builtin-types/list/join.yasl",
  "test/inputs/builtin-types/list/__set.yasl",
  "test/inputs/builtin-types/list/search.yasl",
  "test/inputs/builtin-types/list/extend.yasl",
  "test/inputs/builtin-types/float/operators.yasl",
  "test/inputs/builtin-types/float/toint.yasl",
  "test/inputs/builtin-types/float/tobool.yasl",
  "test/inputs/builtin-types/float/tostr.yasl",
  "test/inputs/builtin-types/float/floats.yasl",
  "test/inputs/builtin-types/float/float.yasl",
  "test/inputs/fn/cvarfn.yasl",
  "test/inputs/fn/implicit_return.yasl",
  "test/inputs/fn/spread.yasl",
  "test/inputs/fn/rest.yasl",
  "test/inputs/fn/nested.yasl",
  "test/inputs/fn/empty.yasl",
  "test/inputs/fn/functional2.yasl",
  "test/inputs/fn/forward.yasl",
  "test/inputs/fn/functional.yasl",
  "test/inputs/fn/add.yasl",
  "test/inputs/binops.yasl",
  "test/inputs/syntax/augmented_assign.yasl",
  "test/inputs/syntax/multi_assign.yasl",
  "test/inputs/syntax/foreach_str.yasl",
  "test/inputs/syntax/complicated_comprehension.yasl",
  "test/inputs/syntax/while_break.yasl",
  "test/inputs/syntax/ifdef.yasl",
  "test/inputs/syntax/newline_list_table.yasl",
  "test/inputs/syntax/for_continue.yasl",
  "test/inputs/syntax/multi_assign_set_list.yasl",
  "test/inputs/syntax/foreach_break.yasl",
  "test/inputs/syntax/multi_assign_swap.yasl",
  "test/inputs/syntax/for.yasl",
  "test/inputs/syntax/fn_with_for.yasl",
  "test/inputs/syntax/foreach_continue.yasl",
  "test/inputs/syntax/if.yasl",
  "test/inputs/syntax/foreach.yasl",
  "test/inputs/syntax/while.yasl",
  "test/inputs/syntax/continue_locals.yasl",
  "test/inputs/syntax/table_comprehensions.yasl",
  "test/inputs/syntax/while_continue.yasl",
  "test/inputs/syntax/list_comprehensions.yasl",
  "test/inputs/syntax/for_break.yasl",
  "test/inputs/syntax/foreach_listcomp.yasl",
  "test/inputs/syntax/multi_assign_set_table.yasl",
  "test/inputs/syntax/echo.yasl",
  "test/inputs/syntax/ifndef.yasl",
  "test/inputs/match/guard/last_guard.yasl",
  "test/inputs/match/guard/list_guard.yasl",
  "test/inputs/match/guard/bind_guard.yasl",
  "test/inputs/match/guard/basic_guard.yasl",
  "test/inputs/match/bind/let.yasl",
  "test/inputs/match/bind/const.yasl",
  "test/inputs/match/table/vtable_basic.yasl",
  "test/inputs/match/table/table.yasl",
  "test/inputs/match/table/vtable.yasl",
  "test/inputs/match/not/not.yasl",
  "test/inputs/match/str/str_single.yasl",
  "test/inputs/match/str/str_back.yasl",
  "test/inputs/match/str/str_dot.yasl",
  "test/inputs/match/types.yasl",
  "test/inputs/match/alt/alt_multi.yasl",
  "test/inputs/match/alt/alt_nested_right.yasl",
  "test/inputs/match/alt/alt_nested_left.yasl",
  "test/inputs/match/alt/alt.yasl",
  "test/inputs/match/fallthrough.yasl",
  "test/inputs/match/basic_1.yasl",
  "test/inputs/match/bool.yasl",
  "test/inputs/match/list/const_vlist.yasl",
  "test/inputs/match/list/vlist.yasl",
  "test/inputs/match/list/list.yasl",
  "test/inputs/match/list/list_let.yasl",
  "test/inputs/match/undef.yasl",
  "test/inputs/match/empty.yasl",
  "test/inputs/match/basic_smallint.yasl",
  "test/inputs/match/float_smallint.yasl",
  "test/inputs/match/float.yasl",
  "test/inputs/match/parens.yasl",
  "test/inputs/varpack/method_0_args_and_val.yasl",
  "test/inputs/varpack/list_set.yasl",
  "test/inputs/varpack/nested_calls.yasl",
  "test/inputs/varpack/closure_multi_return.yasl",
  "test/inputs/varpack/method_0_args.yasl",
  "test/inputs/varpack/function_with_args.yasl",
  "test/inputs/varpack/too_few.yasl",
  "test/inputs/varpack/method_using_fields.yasl",
  "test/inputs/varpack/c_call.yasl",
  "test/inputs/varpack/closure_simple.yasl",
  "test/inputs/varpack/function_0_args.yasl",
  "test/inputs/varpack/nested_method.yasl",
  "test/inputs/varpack/function_0_args_and_val.yasl",
  "test/inputs/varpack/collect.yasl",
  "test/inputs/varpack/method_call.yasl",
  "test/inputs/varpack/index_call.yasl",
  "test/inputs/varpack/function_call.yasl",
  "test/inputs/std/collections/set.yasl",
  "test/inputs/std/collections/contains.yasl",
  "test/inputs/std/collections/set_fromlist.yasl",
  "test/inputs/std/collections/set_cartesian.yasl",
  "test/inputs/std/collections/set_iter.yasl",
  "test/inputs/std/collections/table.yasl",
  "test/inputs/std/collections/set_comp.yasl",
  "test/inputs/std/collections/list.yasl",
  "test/inputs/std/io_setformat.yasl",
  "test/inputs/std/io.yasl",
  "test/inputs/std/math/min-max.yasl",
  "test/inputs/std/math/math.yasl",
  "test/inputs/std/require/ifdef.yasl",
  "test/inputs/std/require/require.yasl",
  "test/inputs/std/mt/setmt.yasl",
  "test/inputs/std/mt/mt.yasl",
  "test/inputs/std/try.yasl",
  "test/inputs/__call.yasl",
  "test/inputs/yasl_iter.yasl",
  "test/inputs/empty.yasl",
  "test/inputs/vargs/fold.yasl",
  "test/inputs/vargs/closure.yasl",
  "test/inputs/vargs/prepend.yasl",
  "test/inputs/vargs/first_rest.yasl",
  "test/inputs/vargs/simple.yasl",
  "test/inputs/vargs/filter.yasl",
  "test/inputs/vargs/empty.yasl",
  "test/inputs/vargs/map.yasl",
  "test/inputs/vargs/forward.yasl",
  "test/inputs/vargs/isempty.yasl",
  "test/inputs/multiset.yasl",
  "test/inputs/refcount.yasl",
  "test/inputs/method.yasl",
  "test/inputs/unary.yasl",
  "test/inputs/dead_code_elimination.yasl",
  "test/inputs/assert.yasl",
  "test/inputs/scripts/AOC2024-6.yasl",
  "test/inputs/scripts/quine.yasl",
  "test/inputs/scripts/factorial.yasl",
  "test/inputs/scripts/CTCI1-1.yasl",
  "test/inputs/scripts/fib_iter.yasl",
  "test/inputs/scripts/LC-5.yasl",
  "test/inputs/scripts/AOC2022-3.yasl",
  "test/inputs/scripts/LC-3.yasl",
  "test/inputs/scripts/CTCI1-4.yasl",
  "test/inputs/scripts/fib.yasl",
  "test/inputs/scripts/twosum.yasl",
  "test/inputs/scripts/LC-1.yasl",
  "test/inputs/scripts/CTCI1-9.yasl",
  "test/inputs/scripts/heap.yasl",
  "test/inputs/scripts/CTCI1-6.yasl",
  "test/inputs/scripts/CTCI5-4.yasl",
  "test/inputs/scripts/box.yasl",
  "test/inputs/scripts/LC-2.yasl",
  "test/inputs/scripts/fib_match.yasl",
  "test/inputs/scripts/LC-4.yasl",
  "test/inputs/scripts/str_to_json.yasl",
  "test/inputs/ternary.yasl",
  "test/inputs/version.yasl",
  "test/inputs/closures/two.yasl",
  "test/inputs/closures/multi.yasl",
  "test/inputs/closures/deep.yasl",
  "test/inputs/closures/location_closed.yasl",
  "test/inputs/closures/deep_assign.yasl",
  "test/inputs/closures/simple.yasl",
  "test/inputs/closures/location_open.yasl",
  "test/inputs/closures/deep_param.yasl",
  "test/inputs/closures/memoize.yasl",
  "test/inputs/closures/loop.yasl",
  "test/inputs/closures/local.yasl",
  "test/inputs/closures/assign.yasl",
};
