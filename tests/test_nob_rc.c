#include <assert.h>
#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
// #define NOB_RC_LOG
#define NOB_RC_TRACK_STATS
#define NOB_RC_IMPLEMENTATION
#include "nob_utils.h"

typedef enum {
    EXPR_NIL,
    EXPR_SYMBOL,
    EXPR_INTEGER,
    EXPR_PAIR,
    __count_Expr_Kind,
} Expr_Kind;

typedef struct Expr Expr;

struct Expr {
    Expr_Kind kind;
    static_assert(__count_Expr_Kind == 4, "Add parameters for new Expr_Kind");
    union {
        const char *symbol;
        int integer;
        struct {
            Expr *left;
            Expr *right;
        } pair;
    };
};

void destroy_expr(void *data);

RC_Allocator expr_allocator = {.destroy=destroy_expr};

void destroy_expr(void *data) {
    Expr *expr = data;
    switch (expr->kind) {
    case EXPR_PAIR: {
        rc_release(&expr_allocator, expr->pair.left);
        rc_release(&expr_allocator, expr->pair.right);
    } break;
    case EXPR_NIL:
    case EXPR_INTEGER:
    case EXPR_SYMBOL:
        break;
    case __count_Expr_Kind:
    default:
        UNREACHABLE("Expr_Kind");
    }
}

Expr *alloc_expr(Expr_Kind kind) {
    Expr *ret = rc_alloc(&expr_allocator, sizeof(Expr));
    ret->kind = kind;
    return ret;
}

Expr *make_nil(void) {
    return alloc_expr(EXPR_NIL);
}

Expr *make_integer(int integer) {
    Expr *ret = alloc_expr(EXPR_INTEGER);
    ret->integer = integer;
    return ret;
}

Expr *make_symbol(const char *symbol) {
    Expr *ret = alloc_expr(EXPR_SYMBOL);
    ret->symbol = symbol;
    return ret;
}

Expr *make_pair(Expr *left, Expr *right) {
    Expr *ret = alloc_expr(EXPR_PAIR);
    ret->pair.left  = rc_acquire(&expr_allocator, left);
    ret->pair.right = rc_acquire(&expr_allocator, right);
    return ret;
}

static_assert(__count_Expr_Kind == 4, "Add make_* for new Expr_Kind");

Expr *copy_expr(Expr *expr) {
    Expr *ret = alloc_expr(expr->kind);
    memcpy(ret, expr, sizeof(Expr));
    return ret;
}

Expr *args_to_list(va_list args) {
    Expr *arg = va_arg(args, Expr *);
    if (arg->kind == EXPR_NIL) return arg;
    return make_pair(arg, args_to_list(args));
}

#define make_list(...) make_list_impl(NULL, __VA_ARGS__, make_nil())
Expr *make_list_impl(void *first, ...) {
    va_list args;
    va_start(args, first);
    Expr *list = args_to_list(args);
    va_end(args);
    return list;
}

#define dump_expr(expr) dump_expr_impl(expr, 0)
void dump_expr_impl(Expr *expr, int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    switch (expr->kind) {
    case EXPR_NIL: {
        printf("NIL(%zu) %p\n", rc_count(expr), expr);
    } break;
    case EXPR_SYMBOL: {
        printf("Symbol(%zu) %p: %s\n", rc_count(expr), expr, expr->symbol);
    } break;
    case EXPR_INTEGER: {
        printf("Integer(%zu) %p: %d\n", rc_count(expr), expr, expr->integer);
    } break;
    case EXPR_PAIR: {
        printf("Pair(%zu) %p:\n", rc_count(expr), expr);
        dump_expr_impl(expr->pair.left, level + 1);
        dump_expr_impl(expr->pair.right, level + 1);
    } break;
    case __count_Expr_Kind:
    default: UNREACHABLE("Expr_Kind");
    }
}

Expr *eval_expr(Expr *expr) {
    if (expr->kind == EXPR_PAIR) {
        assert(expr->pair.left->kind == EXPR_SYMBOL);
        if (strcmp(expr->pair.left->symbol, "+") == 0) {
            int res = 0;
            expr = expr->pair.right;
            while (expr->kind != EXPR_NIL) {
                assert(expr->kind == EXPR_PAIR);
                Expr *left_evaled = eval_expr(expr->pair.left);
                assert(left_evaled->kind == EXPR_INTEGER);
                res += left_evaled->integer;
                if (left_evaled != expr->pair.left)
                    rc_release(&expr_allocator, left_evaled);
                expr = expr->pair.right;
            }
            return make_integer(res);
        } else if (strcmp(expr->pair.left->symbol, "swap") == 0) {
            Expr *args = expr->pair.right;
            assert(args->kind == EXPR_PAIR);
            assert(args->pair.left->kind  == EXPR_PAIR);
            assert(args->pair.right->kind == EXPR_NIL);
            args = args->pair.left;
            return make_pair(args->pair.right, args->pair.left);
        } else {
            UNREACHABLE(temp_sprintf("Unknown Symbol to Evaluate `%s`", expr->pair.left->symbol));
        }
    }

    return expr;
}

int main(void) {
    {
        Expr *expr1 = rc_acquire(&expr_allocator, make_list(
            make_symbol("+"),
            make_integer(1),
            make_integer(2),
            make_integer(3),
            make_integer(4)
        ));
        printf("Expr1:\n");
        dump_expr(expr1);

        printf("--------------------------------------------\n");
        Expr *expr1_evaled = rc_acquire(&expr_allocator, eval_expr(expr1));
        printf("Expr1 (Evaled):\n");
        dump_expr(expr1_evaled);

        {
            printf("--------------------------------------------\n");
            Expr *expr2 = rc_acquire(&expr_allocator, make_list(
                make_symbol("+"),
                expr1,
                expr1,
                expr1_evaled,
                make_integer(5),
                make_integer(6),
                expr1,
                expr1_evaled,
                make_list(
                    make_symbol("+"),
                    make_integer(1),
                    make_integer(2)
                )
            ));
            printf("Expr2: \n");
            dump_expr(expr2);
            rc_release(&expr_allocator, expr1);
            rc_release(&expr_allocator, expr1_evaled);

            printf("--------------------------------------------\n");
            Expr *expr2_evaled = rc_acquire(&expr_allocator, eval_expr(expr2));
            rc_release(&expr_allocator, expr2);
            printf("Expr2 (Evaled):\n");
            dump_expr(expr2_evaled);
            rc_release(&expr_allocator, expr2_evaled);
            rc_release(&expr_allocator, expr2_evaled);
        }
        rc_release(&expr_allocator, expr1);
        rc_release(&expr_allocator, expr1_evaled);
    }

    {
        printf("--------------------------------------------\n");
        Expr *expr3 = rc_acquire(&expr_allocator, make_list(
            make_symbol("swap"),
            make_pair(
                make_integer(1),
                make_symbol("+"))));
        printf("Expr3: \n");
        dump_expr(expr3);

        printf("--------------------------------------------\n");
        Expr *expr3_evaled = rc_acquire(&expr_allocator, eval_expr(expr3));
        rc_release(&expr_allocator, expr3);
        printf("Expr3 (Evaled):\n");
        dump_expr(expr3_evaled);
        rc_release(&expr_allocator, expr3_evaled);
    }

    rc_print_stats(expr_allocator);

    return 0;
}

