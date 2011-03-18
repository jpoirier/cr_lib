cr_lib is a simple, portable, cooperative multitasking environment using coroutines. 

It features a mechanism for exiting and re-entering a function in a non-standard way using the 
standard C library's setjmp and longjmp functions. Coroutine threads run one at a time, are persistent 
for the lifetime of the application, and each thread must be explicitly scheduled. cr_lib includes a 
system coroutine, called cr_idle, that can be used in conjunction with the global variable cr_g_activate_id 
to activate a coroutine. cr_g_activate_id could be used, for example, in an interrupt service routine (ISR) 
of an event driven system. cr_lib is probably most useful for micro-controllers, DSPs, small GPPs, or 
educational purposes. An ANSI C99 compliant compiler is required. 

What is a coroutine: "A coroutine is represented by a closure (a code address and a referencing environment), 
into which we can jump by means of a nonlocal goto - in this case a special operation known as a transfer. 
In effect, coroutines are execution contexts that exist concurrently but execute one at a time, and transfer 
control to each other explicitly by name." 

From Programming Language Pragmatics by Morgan Kaufmann

See the documentation: /doc/index.html

hg
--
> hg commit -m '...put comments here�'
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
- setjmp invocation must be done in the context described in the standard 


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

� the entire controlling expression of a selection or iteration statement;
� one operand of a relational or equality operator with the other operand an integer
  constant expression, with the resulting expression being the entire controlling
  expression of a selection or iteration statement;
� the operand of a unary ! operator with the resulting expression being the entire
  controlling expression of a selection or iteration statement; or
� the entire expression of an expression statement (possibly cast to void).

If the invocation appears in any other context, the behavior is undefined.

The longjmp function
---------------------
All accessible objects have values, and all other components of the abstract machine
have state, as of the time the longjmp function was called, except that the values of
objects of automatic storage duration that are local to the function containing the
invocation of the corresponding setjmp macro that do not have volatile-qualified type
and have been changed between the setjmp invocation and longjmp call are indeterminate.



