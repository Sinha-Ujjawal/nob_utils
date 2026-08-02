```
Test Case: test_nob_bisect
Target: x86_64-linux-gnu
Errors:
zig: warning: future releases of the clang compiler will prefer GCC installations containing libstdc++ include directories; '/usr/lib/gcc/x86_64-linux-gnu/13' would be chosen over '/usr/lib/gcc/x86_64-linux-gnu/14' [-Wgcc-install-dir-libstdcxx]

Status: ❌ Failure
```
```
Test Case: test_nob_bisect
Target: x86_64-macos-none
Errors:

Status: ✅ Success
```
```
Test Case: test_nob_bisect
Target: x86_64-windows-gnu
Errors:
tests/test_nob_bisect.c:18:5: error: call to undeclared function 'qsort_r'; ISO C99 and later do not support implicit function declarations [-Wimplicit-function-declaration]
   18 |     qsort_r(&arr, ARRAY_LEN(arr), sizeof(*arr), is_lte_for_int, NULL);
      |     ^
1 error generated.

Status: ❌ Failure
```
```
Test Case: test_nob_bisect
Target: aarch64-linux-gnu
Errors:

Status: ✅ Success
```
```
Test Case: test_nob_bisect
Target: aarch64-macos-none
Errors:

Status: ✅ Success
```
```
Test Case: test_nob_bisect
Target: aarch64-windows-gnu
Errors:
tests/test_nob_bisect.c:18:5: error: call to undeclared function 'qsort_r'; ISO C99 and later do not support implicit function declarations [-Wimplicit-function-declaration]
   18 |     qsort_r(&arr, ARRAY_LEN(arr), sizeof(*arr), is_lte_for_int, NULL);
      |     ^
1 error generated.

Status: ❌ Failure
```
