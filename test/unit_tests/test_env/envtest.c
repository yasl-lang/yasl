#include "env.h"
#include "yats.h"

SETUP_YATS();

/*
fn f(a) {
    fn g(b) {
        # echo "#{a}, #{b}"
        return a + b;
    }
    echo g(a)
    return g
}

let add2 = f(2);
let add1 = f(1);
echo add1(6)
echo add2(3)
 */
static void test_two(void) {
	struct Env *outer = env_new(NULL);
	struct Env *inner = env_new(outer);

	scope_decl_var(outer->scope, "a");
	scope_decl_var(outer->scope, "g");
	scope_decl_var(inner->scope, "b");

	int64_t a = env_resolve_upval_index(inner, "a");
	ASSERT_EQ(a, 0);
	env_del(inner);
}

/*
fn f(x) {
    const a = 2 * x
    const b = 3 * x
    fn g(y) {
        return y + a + b;
    }
    echo g(1)
    return g
}

let tmp = f(2);
echo tmp(1)
echo tmp(3)
 */
static void test_multi(void) {
	struct Env *outer = env_new(NULL);
	struct Env *inner = env_new(outer);

	scope_decl_var(outer->scope, "x");
	scope_decl_var(outer->scope, "a");
	scope_decl_var(outer->scope, "b");
	scope_decl_var(outer->scope, "g");
	scope_decl_var(inner->scope, "y");

	int64_t a = env_resolve_upval_index(inner, "a");
	int64_t b = env_resolve_upval_index(inner, "b");

	ASSERT_EQ(a, 0);
	ASSERT_EQ(b, 1);
}

/*
fn f(x) {
    const a = 2 * x
    const b = 3 * x
    fn g(y) {
        return y + a + b;
    }
    echo g(1)
    return g
}

let tmp = f(2);
echo tmp(1)
echo tmp(3)
 */
static void test_multi_reversed(void) {
	struct Env *outer = env_new(NULL);
	struct Env *inner = env_new(outer);

	scope_decl_var(outer->scope, "x");
	scope_decl_var(outer->scope, "a");
	scope_decl_var(outer->scope, "b");
	scope_decl_var(outer->scope, "g");
	scope_decl_var(inner->scope, "y");

	int64_t b = env_resolve_upval_index(inner, "b");
	int64_t a = env_resolve_upval_index(inner, "a");
	int64_t a_again = env_resolve_upval_index(inner, "a");

	ASSERT_EQ(a, 1);
	ASSERT_EQ(a_again, 1);
	ASSERT_EQ(b, 0);
	
	int64_t a_value = env_resolve_upval_value(inner, "a");
	int64_t b_value = env_resolve_upval_value(inner, "b");
	ASSERT_EQ(a_value, 1);
	ASSERT_EQ(b_value, 2);
}

static void test_simple_nested(void) {

}

int envtest(void) {
	test_two();
	test_multi();
	test_multi_reversed();
	return __YASL_TESTS_FAILED__;
}