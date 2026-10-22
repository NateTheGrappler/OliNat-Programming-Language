# Comment tests

`BlockComments.oli` should print:

```text
4
/* This is still a string. */
```

Run it with the built interpreter. Block comments use `/* ... */` and do not nest.

The standalone scanner test also checks line numbers, adjacent and empty comments,
division operators, strings, line comments, and errors for unterminated comments.
From the repository root:

```sh
cc -std=c11 -Wall -Wextra -Iinclude -Isrc src/scanner.c \
  testCases/Comments/scanner_test.c -o /tmp/olinat-scanner-test
/tmp/olinat-scanner-test
```
