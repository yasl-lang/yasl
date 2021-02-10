#include "env.h"
#include "yats.h"

SETUP_YATS();

#define ENTER_SCOPE(env) do { (env)->scope = scope_new((env)->scope); } while(0)

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

	ENTER_SCOPE(outer);
	ENTER_SCOPE(inner);

	scope_decl_var(outer->scope, "a");
	scope_decl_var(outer->scope, "g");
	scope_decl_var(inner->scope, "b");

	int64_t a = env_resolve_upval_index(inner, NULL, "a");
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

	ENTER_SCOPE(outer);
	ENTER_SCOPE(inner);

	scope_decl_var(outer->scope, "x");
	scope_decl_var(outer->scope, "a");
	scope_decl_var(outer->scope, "b");
	scope_decl_var(outer->scope, "g");
	scope_decl_var(inner->scope, "y");

	int64_t a = env_resolve_upval_index(inner, NULL, "a");
	int64_t b = env_resolve_upval_index(inner, NULL, "b");

	ASSERT_EQ(a, 0);
	ASSERT_EQ(b, 1);

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
static void test_multi_reversed(void) {
	struct Env *outer = env_new(NULL);
	struct Env *inner = env_new(outer);

	ENTER_SCOPE(outer);
	ENTER_SCOPE(inner);

	scope_decl_var(outer->scope, "x");
	scope_decl_var(outer->scope, "a");
	scope_decl_var(outer->scope, "b");
	scope_decl_var(outer->scope, "g");
	scope_decl_var(inner->scope, "y");

	int64_t b = env_resolve_upval_index(inner, NULL, "b");
	int64_t a = env_resolve_upval_index(inner, NULL, "a");
	int64_t a_again = env_resolve_upval_index(inner, NULL, "a");

	ASSERT_EQ(a, 1);
	ASSERT_EQ(a_again, 1);
	ASSERT_EQ(b, 0);

	int64_t a_value = env_resolve_upval_value(inner, "a");
	int64_t b_value = env_resolve_upval_value(inner, "b");
	ASSERT_EQ(a_value, 1);
	ASSERT_EQ(b_value, 2);

	env_del(inner);
}

/*
fn outer() {
  let x = 1;
  fn middle() {
    fn inner() {
      echo x;
    }
    return inner
  }
  let i = middle()
  return i
}

let inside = outer()
inside()
 */
static void test_deep(void) {
	struct Env *outer = env_new(NULL);
	struct Env *middle = env_new(outer);
	struct Env *inner = env_new(middle);

	ENTER_SCOPE(outer);
	ENTER_SCOPE(middle);
	ENTER_SCOPE(inner);

	scope_decl_var(outer->scope, "x");
	scope_decl_var(outer->scope, "middle");
	scope_decl_var(middle->scope, "inner");
	scope_decl_var(outer->scope, "i");

	ASSERT_EQ(env_resolve_upval_index(inner, NULL, "x"), 0);
	ASSERT_EQ(env_resolve_upval_value(inner, "x"), ~0);

	ASSERT_EQ(env_resolve_upval_index(middle, NULL, "x"), 0);
	ASSERT_EQ(env_resolve_upval_value(middle, "x"), 0);

	env_del(inner);
}

static void test_deep_many_vars(void) {
	struct Env *outer = env_new(NULL);
	struct Env *middle = env_new(outer);
	struct Env *inner = env_new(middle);

	ENTER_SCOPE(outer);
	ENTER_SCOPE(middle);
	ENTER_SCOPE(inner);

	scope_decl_var(outer->scope, "x");
	scope_decl_var(outer->scope, "y");
	scope_decl_var(outer->scope, "z");
	scope_decl_var(outer->scope, "middle");
	scope_decl_var(middle->scope, "inner");
	scope_decl_var(outer->scope, "i");

	ASSERT_EQ(env_resolve_upval_index(middle, NULL, "y"), 0);
	ASSERT_EQ(env_resolve_upval_value(middle, "y"), 1);

	ASSERT_EQ(env_resolve_upval_index(inner, NULL, "z"), 0);
	ASSERT_EQ(env_resolve_upval_value(inner, "z"), ~1);

	ASSERT_EQ(env_resolve_upval_index(middle, NULL, "z"), 1);
	ASSERT_EQ(env_resolve_upval_value(middle, "z"), 2);

	env_del(inner);
}

int envtest(void) {
	test_two();
	test_multi();
	test_multi_reversed();
	test_deep();
	test_deep_many_vars();
	return NUM_FAILED;
}
