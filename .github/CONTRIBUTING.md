Code style
----------

1. Use `c++11` features whenever possible.

2. Try to write self-documenting code, if you can't or you need to write difficult to read code for performance reasons at least try to write good comments.

3. Write comments for public API or class methods using Doxygen syntax.

4. DW1000Ng API should be written in a different file to let the user decide when to import them (Arduino is memory-constrained).