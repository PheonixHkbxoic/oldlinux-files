/*  @(#) list.pl 1.4.0 (UvA SWI) Mon Jan 1 18:02:00 MET 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: basic list predicates
*/

:- module($list,
	[ length/2
	, select/3
	, delete/3
	, nth0/3
	, nth1/3
	, last/2
	, reverse/2
	, flatten/2
	, is_set/1
	, list_to_set/2
	, intersection/3
	, union/3
	, subset/2
	, subtract/3
	]).

%	length(?List, ?N)
%	Is true when N is the length of List.

length(List, Length) :-
	$length(List, Length), !.		% written in C
length(List, Length) :-
	var(Length),
	length2(List, Length).

length2([], 0).
length2([_|List], N) :-
	length2(List, M), 
	succ(M, N).

%	select(?List1, ?Elem, ?List2)
%	Is true when List1, with Elem removed results in List2.

select([X|Tail], X, Tail).
select([Head|Tail], Elem, [Head|Rest]) :-
	select(Tail, Elem, Rest).

%	delete(?List1, ?Elem, ?List2)
%	Is true when Lis1, with all occurences of Elem deleted results in
%	List2.

delete([], _, []) :- !.
delete([Elem|Tail], Elem, Result) :- !, 
	delete(Tail, Elem, Result).
delete([Head|Tail], Elem, [Head|Rest]) :-
	delete(Tail, Elem, Rest).

%	nth0(?Index, ?List, ?Elem)
%	Is true when Elem is the Index'th element of List. Counting starts
%	at 0.

nth0(Index, List, Elem) :-
	integer(Index), !,
	nth0_1(Index, List, Elem).
nth0(Index, List, Elem) :-
	var(Index), !,
	nth0_2(Index, List, Elem).

nth0_1(0, [Elem|_], Elem) :- !.		% take nth deterministically
nth0_1(N, [_|Tail], Elem) :-
	M is N - 1,
	nth0(M, Tail, Elem).

nth0_2(0, [Elem|_], Elem).		% match
nth0_2(N, [_|Tail], Elem) :-
	nth0_2(M, Tail, Elem), 
	succ(M, N).

%	nth1(?Index, ?List, ?Elem)
%	Is true when Elem is the Index'th element of List. Counting starts
%	at 1.

nth1(Index, List, Elem) :-
	integer(Index), !,
	N is Index - 1,
	nth0_1(N, List, Elem).
nth1(Index, List, Elem) :-
	var(Index),
	nth0_2(N, List, Elem),
	Index is N + 1.

%	last(?Elem, ?List)
%	Succeeds if `Last' unifies with the last element of `List'.

last(Elem, [Elem]).
last(Elem, [_|List]) :-
	last(Elem, List).

%	reverse(?List1, ?List2)
%	Is true when the elements of List2 are in reverse order compared to
%	List1.

reverse(L1, L2) :-
	$reverse(L1, [], L2).

$reverse([], List, List).
$reverse([Head|List1], List2, List3) :-
	$reverse(List1, [Head|List2], List3).

%	flatten(+List1, ?List2)
%	Is true when Lis2 is a non nested version of List1.

flatten(List, FlatList) :-
	$flatten(List, [], FlatList), !.

$flatten(Var, Tl, [Var|Tl]) :-
	var(Var), !.
$flatten([], Tl, Tl) :- !.
$flatten([Hd|Tl], Tail, List) :-
	$flatten(Hd, FlatHeadTail, List), 
	$flatten(Tl, Tail, FlatHeadTail).
$flatten(Atom, Tl, [Atom|Tl]).


		/********************************
		*       SET MANIPULATION        *
		*********************************/

%	is_set(+Set)
%	is True if Set is a proper list without duplicates.

is_set(0) :- !, fail.		% catch variables
is_set([]) :- !.
is_set([H|T]) :-
	memberchk(H, T), !, 
	fail.
is_set([_|T]) :-
	is_set(T).

%	list_to_set(+List, ?Set)
%	is true when Set has the same element as List in the same order

list_to_set([], []).
list_to_set([H|T], R) :-
	memberchk(H, T), !, 
	list_to_set(T, R).
list_to_set([H|T], [H|R]) :-
	list_to_set(T, R).

%	intersection(+Set1, +Set2, -Set3)
%	Succeeds if Set3 unifies with the intersection of Set1 and Set2

intersection([], _, []) :- !.
intersection([X|T], L, Intersect) :-
	memberchk(X, L), !, 
	Intersect = [X|R], 
	intersection(T, L, R).
intersection([_|T], L, R) :-
	intersection(T, L, R).

%	union(+Set1, +Set2, -Set3)
%	Succeeds if Set3 unifies with the union of Set1 and Set2

union([], L, L) :- !.
union([H|T], L, R) :-
	memberchk(H, L), !, 
	union(T, L, R).
union([H|T], L, [H|R]) :-
	union(T, L, R).

%	subset(+SubSet, +Set)
%	Succeeds if all elements of SubSet belong to Set as well.

subset([], _) :- !.
subset([E|R], Set) :-
	memberchk(E, Set), 
	subset(R, Set).

%	subtract(+Set, +Delete, -Result)
%	Delete all elements from `Set' that occur in `Delete' (a set) and
%	unify the result with `Result'.

subtract([], _, []) :- !.
subtract([E|T], D, R) :-
	memberchk(E, D), !,
	subtract(T, D, R).
subtract([H|T], D, [H|R]) :-
	subtract(T, D, R).
