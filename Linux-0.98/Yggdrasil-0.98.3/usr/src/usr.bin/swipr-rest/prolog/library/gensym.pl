/*  @(#) gensym.pl 1.4.0 (UvA SWI) Mon Jan 1 18:03:13 MET 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: play with gensym
*/

:- module(gensym,
	[ reset_gensym/0
    	, gensym/2
	]).

:- style_check(+dollar).

gensym(Base, Atom) :-
	concat($gs_, Base, Key), 
	flag(Key, Old, Old), 
	record_gensym(Key, Old),
	succ(Old, New), 
	flag(Key, _, New), 
	concat(Base, New, Atom).

record_gensym(Key, 0) :- !,
	recordz($gensym, Key).
record_gensym(_, _).

reset_gensym :-
	recorded($gensym, Key, Ref),
	    erase(Ref),
	    flag(Key, _, 0),
	fail.
reset_gensym.
