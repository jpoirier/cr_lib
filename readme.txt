See the documentation: /doc/index.html

hg
--
> hg commit -m '...put comments hereÉ'
> hg push


Notes to me
-----------

- include gmake executable?
- TCC and DMC compiler links and information?
- pthread required to build demo_4.c on Windows

http://www.cs.utk.edu/~plank/plank/classes/cs360/360/notes/Setjmp/lecture.html

Caveats
-------
- currently, the library assumes a coroutine never returns
- setjmp doesn't save floating point environment
- setjmp invocation must be done in the contexts described in the standard 


ANSI C99 Standard text
----------------------
jmp_buf
-------
The environment of a call to the setjmp macro consists of information
sufficient for a call to the longjmp function to return execution to the correct
block and invocation of that block, were it called recursively. It does not include
the state of the floating-point status flags, of open files, or of any other component
of the abstract machine.

The setjmp macro
----------------
An invocation of the setjmp macro shall appear only in one of the following contexts:

Ñ the entire controlling expression of a selection or iteration statement;
Ñ one operand of a relational or equality operator with the other operand an integer
  constant expression, with the resulting expression being the entire controlling
  expression of a selection or iteration statement;
Ñ the operand of a unary ! operator with the resulting expression being the entire
  controlling expression of a selection or iteration statement; or
Ñ the entire expression of an expression statement (possibly cast to void).

If the invocation appears in any other context, the behavior is undefined.

The longjmp function
---------------------
All accessible objects have values, and all other components of the abstract machine
have state, as of the time the longjmp function was called, except that the values of
objects of automatic storage duration that are local to the function containing the
invocation of the corresponding setjmp macro that do not have volatile-qualified type
and have been changed between the setjmp invocation and longjmp call are indeterminate.



