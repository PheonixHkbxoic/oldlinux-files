/*  @(#) quintus.pl 1.4.0 (UvA SWI) Mon Jan 1 18:03:13 MET 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: Quintus compatibility predicates
*/

:- module(quintus,
	[ unix/1
%	, file_exists/1
	, call/2
	, call/3

	, abs/2
	, sin/2
	, cos/2
	, tan/2

	, (mode)/1
	, (public)/1
	, (meta_predicate)/1
	, no_style_check/1
	]).

		/********************************
		*      SYSTEM INTERACTION       *
		*********************************/

%	unix( +Action )
%	interface to  Unix.   Currently  only  `system(  Command  )'  is
%	available as `Action'.

unix( system(Command) ) :-
	shell( Command ).

%	file_exists( +File )
%	Succeeds if `File' exists as a file or directory in the Unix file
%	system.

file_exists( File ) :-
	exists_file( File ).

		/********************************
		*          META CALLS           *
		*********************************/

%	call( +Pred,+Argument,... )
%	call `Pred', appending the additional arguments to the goal

call( Pred,A0 ) :-
	apply( Pred,[A0] ).
call( Pred,A0,A1 ) :-
	apply( Pred,[A0,A1] ).


		/********************************
		*          ARITHMETIC           *
		*********************************/

%	abs( +Number,-Absolute )
%	Unify `Absolute' with the absolute value of `Number'.

abs( Number,Absolute ) :-
	Absolute is abs(Number).

%	Math library predicates

sin(A, V) :- V is sin(A).
cos(A, V) :- V is cos(A).
tan(A, V) :- V is tan(A).


		/********************************
		*          STYLE CHECK          *
		*********************************/

q_style_option(single_var, singleton) :- !.
q_style_option(Option, Option).

no_style_check(QOption) :-
	q_style_option(QOption, SWIOption),
	style_check(-SWIOption).

		/********************************
		*            OPERATORS          *
		*********************************/

:- op(0, fy, not).

		/********************************
		*         DIRECTIVES            *
		*********************************/

:- op(1150, fx, [(mode), (public)]).

mode(_).
public(_).

		/********************************
		*            MODULES            *
		*********************************/

:- op(1150, fx, (meta_predicate)).

:- module_transparent
	(meta_predicate)/1,
	(meta_predicate1)/1.

meta_predicate((Head, More)) :- !, 
	meta_predicate1(Head), 
	meta_predicate(More).
meta_predicate(Head) :-
	meta_predicate1(Head).

meta_predicate1(Head) :-
	Head =.. [Name|Arguments], 
	member(Arg, Arguments), 
	module_expansion_argument(Arg), !, 
	functor(Head, Name, Arity), 
	module_transparent(Name/Arity).
meta_predicate1(_).		% just a mode declaration

module_expansion_argument(:).
module_expansion_argument(N) :- integer(N).
