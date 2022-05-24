#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "libgccjit.h"

#include "harness.h"

void
create_code (gcc_jit_context *ctxt, void *user_data)
{
  /* Let's try to inject the equivalent of:

     int test_ptr(const int* value)
     {
      return *foo;
     }

     int main (void)
     {
       int value = 10;
       const int *ptr = &value;
       int res = test_ptr (ptr);
       return res;
     }
  */
  gcc_jit_type *int_type =
    gcc_jit_context_get_type (ctxt, GCC_JIT_TYPE_INT);
  gcc_jit_type *int_ptr_type =
    gcc_jit_type_get_pointer (int_type);
  gcc_jit_type *const_ptr_type =
    gcc_jit_type_get_const (int_ptr_type);

  /* Build the test_ptr.  */
  gcc_jit_param *param =
    gcc_jit_context_new_param (ctxt, NULL, const_ptr_type, "value");
  gcc_jit_function *test_ptr =
    gcc_jit_context_new_function (ctxt, NULL,
				  GCC_JIT_FUNCTION_EXPORTED,
				  int_type,
				  "test_ptr",
				  1, &param,
				  0);
  gcc_jit_block *block = gcc_jit_function_new_block (test_ptr, NULL);
  gcc_jit_block_end_with_return (block,
    NULL,
    gcc_jit_lvalue_as_rvalue (
      gcc_jit_rvalue_dereference (gcc_jit_param_as_rvalue (param), NULL)));

  /* Build main.  */
  gcc_jit_function *main =
    gcc_jit_context_new_function (ctxt, NULL,
				  GCC_JIT_FUNCTION_EXPORTED,
				  int_type,
				  "main",
				  0, NULL,
				  0);
  gcc_jit_block *main_block = gcc_jit_function_new_block (main, NULL);
  gcc_jit_lvalue *variable =
    gcc_jit_function_new_local (main, NULL, int_type, "value");
  gcc_jit_lvalue *pointer =
    gcc_jit_function_new_local (main, NULL, const_ptr_type, "ptr");
  gcc_jit_block_add_assignment (main_block, NULL, pointer,
    gcc_jit_lvalue_get_address (variable, NULL));
  gcc_jit_rvalue *ptr_rvalue = gcc_jit_lvalue_as_rvalue (pointer);
  gcc_jit_rvalue *res =
    gcc_jit_context_new_call (ctxt, NULL, test_ptr, 1, &ptr_rvalue);
  gcc_jit_block_end_with_return (main_block, NULL, res);
}

void
verify_code (gcc_jit_context *ctxt, gcc_jit_result *result)
{
  CHECK_NON_NULL (result);
}
