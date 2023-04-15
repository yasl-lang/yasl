static const char *inputs[] = {
  "test/inputs/clear.yasl",
  "test/inputs/lambdas/ambiguous.yasl",
  "test/inputs/lambdas/emptyreturn.yasl",
  "test/inputs/lambdas/nested.yasl",
  "test/inputs/lambdas/basic.yasl",
  "test/inputs/bool/operators.yasl",
  "test/inputs/bool/tobool.yasl",
  "test/inputs/bool/tostr.yasl",
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
  "test/inputs/table/clear.yasl",
  "test/inputs/table/remove.yasl",
  "test/inputs/table/__len.yasl",
  "test/inputs/table/__eq.yasl",
  "test/inputs/table/copy.yasl",
  "test/inputs/table/__bor.yasl",
  "test/inputs/table/tostr.yasl",
  "test/inputs/table/operator_overloading_right.yasl",
  "test/inputs/table/__set.yasl",
  "test/inputs/table/operator_overloading.yasl",
  "test/inputs/table/values.yasl",
  "test/inputs/table/keys.yasl",
  "test/inputs/table/memberfn.yasl",
  "test/inputs/table/comp_op_overloading.yasl",
  "test/inputs/table/iadd.yasl",
  "test/inputs/table/comp_op_overloading_right.yasl",
  "test/inputs/table/tables.yasl",
  "test/inputs/int/operators.yasl",
  "test/inputs/int/literals.yasl",
  "test/inputs/int/concat_3.yasl",
  "test/inputs/int/decimal.yasl",
  "test/inputs/int/tobool.yasl",
  "test/inputs/int/hex.yasl",
  "test/inputs/int/tostr.yasl",
  "test/inputs/int/binary.yasl",
  "test/inputs/int/tofloat.yasl",
  "test/inputs/int/smallints.yasl",
  "test/inputs/int/tochar.yasl",
  "test/inputs/binops.yasl",
  "test/inputs/collections/set.yasl",
  "test/inputs/collections/contains.yasl",
  "test/inputs/collections/set_fromlist.yasl",
  "test/inputs/collections/set_cartesian.yasl",
  "test/inputs/collections/set_iter.yasl",
  "test/inputs/collections/table.yasl",
  "test/inputs/collections/set_comp.yasl",
  "test/inputs/collections/list.yasl",
  "test/inputs/str/operators.yasl",
  "test/inputs/str/literals.yasl",
  "test/inputs/str/startswith.yasl",
  "test/inputs/str/tolower.yasl",
  "test/inputs/str/count.yasl",
  "test/inputs/str/tolist2.yasl",
  "test/inputs/str/ltrim.yasl",
  "test/inputs/str/slice.yasl",
  "test/inputs/str/split.yasl",
  "test/inputs/str/rep.yasl",
  "test/inputs/str/withnull.yasl",
  "test/inputs/str/tobyte.yasl",
  "test/inputs/str/replace.yasl",
  "test/inputs/str/tobool.yasl",
  "test/inputs/str/slice_mt.yasl",
  "test/inputs/str/dot.yasl",
  "test/inputs/str/isnum.yasl",
  "test/inputs/str/isspace.yasl",
  "test/inputs/str/rtrim.yasl",
  "test/inputs/str/tolist.yasl",
  "test/inputs/str/spread.yasl",
  "test/inputs/str/isal.yasl",
  "test/inputs/str/interpolation.yasl",
  "test/inputs/str/__get.yasl",
  "test/inputs/str/tostr.yasl",
  "test/inputs/str/trim.yasl",
  "test/inputs/str/concat.yasl",
  "test/inputs/str/search.yasl",
  "test/inputs/str/toupper.yasl",
  "test/inputs/str/isalnum.yasl",
  "test/inputs/str/strings.yasl",
  "test/inputs/str/endswith.yasl",
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
  "test/inputs/match/str/str_single.yasl",
  "test/inputs/match/str/str_back.yasl",
  "test/inputs/match/str/str_dot.yasl",
  "test/inputs/match/types.yasl",
  "test/inputs/match/alt/alt_multi.yasl",
  "test/inputs/match/alt/alt_nested_right.yasl",
  "test/inputs/match/alt/alt_nested_left.yasl",
  "test/inputs/match/alt/alt.yasl",
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
  "test/inputs/__call.yasl",
  "test/inputs/yasl_iter.yasl",
  "test/inputs/math/min-max.yasl",
  "test/inputs/math/math.yasl",
  "test/inputs/list/clear.yasl",
  "test/inputs/list/insert.yasl",
  "test/inputs/list/pop.yasl",
  "test/inputs/list/count.yasl",
  "test/inputs/list/remove.yasl",
  "test/inputs/list/sort.yasl",
  "test/inputs/list/join_empty.yasl",
  "test/inputs/list/slice.yasl",
  "test/inputs/list/__len.yasl",
  "test/inputs/list/__eq.yasl",
  "test/inputs/list/copy.yasl",
  "test/inputs/list/reverse.yasl",
  "test/inputs/list/__set2.yasl",
  "test/inputs/list/__get.yasl",
  "test/inputs/list/__add.yasl",
  "test/inputs/list/tostr.yasl",
  "test/inputs/list/push.yasl",
  "test/inputs/list/join.yasl",
  "test/inputs/list/__set.yasl",
  "test/inputs/list/search.yasl",
  "test/inputs/list/join_default.yasl",
  "test/inputs/list/extend.yasl",
  "test/inputs/float/operators.yasl",
  "test/inputs/float/toint.yasl",
  "test/inputs/float/tobool.yasl",
  "test/inputs/float/tostr.yasl",
  "test/inputs/float/floats.yasl",
  "test/inputs/float/float.yasl",
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
  "test/inputs/require/ifdef.yasl",
  "test/inputs/require/require.yasl",
  "test/inputs/multiset.yasl",
  "test/inputs/mt/setmt.yasl",
  "test/inputs/mt/mt.yasl",
  "test/inputs/refcount.yasl",
  "test/inputs/method.yasl",
  "test/inputs/unary.yasl",
  "test/inputs/dead_code_elimination.yasl",
  "test/inputs/assert.yasl",
  "test/inputs/scripts/factorial.yasl",
  "test/inputs/scripts/CTCI1-1.yasl",
  "test/inputs/scripts/fib_iter.yasl",
  "test/inputs/scripts/LC-5.yasl",
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
  "test/inputs/closures/deep_assign.yasl",
  "test/inputs/closures/simple.yasl",
  "test/inputs/closures/deep_param.yasl",
  "test/inputs/closures/memoize.yasl",
  "test/inputs/closures/loop.yasl",
  "test/inputs/closures/local.yasl",
  "test/inputs/closures/location.yasl",
  "test/inputs/closures/assign.yasl",
};
