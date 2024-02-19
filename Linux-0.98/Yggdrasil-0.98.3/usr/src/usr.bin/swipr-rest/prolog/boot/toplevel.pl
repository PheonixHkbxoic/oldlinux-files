/*  @(#) toplevel.pl 1.4.0 (UvA SWI) Mon Jan 1 18:02:00 MET 1990

    Copyright (c) 1990 Jan Wielemaker. All rights reserved.
    jan@swi.psy.uva.nl

    Purpose: top level user interaction
*/

:- module($toplevel,
	[ $init/0 			% start Prolog (does not return)
	, $init_return/0		% initialise Prolog and return
	, $toplevel/0			% Prolog top-level (re-entrant)
	, $abort/0 			% restart after an abort
	, $break/0 			% live in a break
	, $compile/0 			% `-c' toplevel
	, $welcome/0			% banner
	, prolog/0 			% user toplevel predicate
	, time/1			% time query
	]).


		/********************************
		*         INITIALISATION        *
		*********************************/

$welcome :-
	$version(Version),
	$ttyformat('Welcome to SWI-Prolog (Version ~w)~n', [Version]),
	$ttyformat('Copyright (c) 1990, University of Amsterdam.  '),
	$ttyformat('All rights reserved.~n~n').

$load_init_file(none) :- !.
$load_init_file(Base) :-
	member(Prefix, ['', '~/']),
	concat(Prefix, Base, InitFile), 
	exists_file(InitFile), !, 
	user:ensure_loaded(InitFile).
$load_init_file(_).

$check_novice :-
	$novice(on, on), 
	getenv('PROLOGCHILD', _), !, 
	format('Cannot start Prolog from a child process running under Prolog~n'), 
	format('Please type Control-D or `exit'' to return to Prolog~n'), 
	halt.
$check_novice.


		/********************************
		*        TOPLEVEL GOALS         *
		*********************************/

$init :-
	$init_return,
	$toplevel.

$init_return :-
	$check_novice, 
	$clean_history,
	$option(init_file, File), 
	$load_init_file(File), 
	$option(goal, GoalAtom), 
	term_to_atom(Goal, GoalAtom), 
	ignore(user:Goal).

$abort :-
	see(user), 
	tell(user), 
	flag($break_level, _, 0), 
	$ttyformat('~nExecution Aborted~n~n'),
	$toplevel.

$break :-
	flag($break_level, Old, Old), 
	succ(Old, New), 
	flag($break_level, _, New), 
	$ttyformat('Break Level [~w]~n', [New]),
	$toplevel,
	$ttyformat('Exit Break Level [~w]~n', [New]),
	flag($break_level, _, Old), !.

$toplevel :-
	$option(top_level, TopLevelAtom), 
	term_to_atom(TopLevel, TopLevelAtom), 
	user:TopLevel.

%	$compile
%	Tolpevel called when invoked with -c option.

$compile :-
	$compile_wic.


		/********************************
		*    USER INTERACTIVE LOOP      *
		*********************************/

prolog :-
	flag($tracing, _, off), 
	flag($break_level, BreakLev, BreakLev), 
	repeat, 
	    (   $module(TypeIn, TypeIn), 
		$system_prompt(TypeIn, BreakLev, Prompt),
		prompt(Old, '|    '), 
		trim_stacks,
		read_history(h, '!h', 
			      [trace, end_of_file], 
			      Prompt, Goal, Bindings), 
		prompt(_, Old)
	    ->  $execute(Goal, Bindings)
	    ), !.

$system_prompt(user, 0, '%w ?- ') :- !.
$system_prompt(user, Lev, Prompt) :- !,
	concat_atom(['[', Lev, '] %w ?- '], Prompt).
$system_prompt(Mod, 0, Prompt) :- !,
	concat_atom([Mod, ': %w ?- '], Prompt).
$system_prompt(Mod, Lev, Prompt) :- !,
	concat_atom([Mod, ': [', Lev, '] %w ?- '], Prompt).

$execute(Var, _) :-
	var(Var), !,
	$ttyformat('... 1,000,000 ... 5,000,000 ... 10,000,000 years later~n~n'),
	$ttyformat('~t~8|>> 42 << (last release gives the question)~n'),
	fail.
$execute(end_of_file, _) :- !.
$execute(Goal, Bindings) :-
	$module(TypeIn, TypeIn), 
	TypeIn:$dwim_correct_goal(Goal, Bindings, Corrected), !, 
	$execute_goal(Corrected, Bindings).
$execute(_, _) :-
	notrace, 
	$ttyformat('~nNo~n'),
	fail.

$execute_goal(trace, []) :-
	trace, 
	$ttyformat('~n'),
	$write_bindings([]), !, 
	fail.
$execute_goal(Goal, Bindings) :-
	$module(TypeIn, TypeIn), 
	$user_call(TypeIn:Goal),
	$ttyformat('~n'),
	$write_bindings(Bindings), !, 
	notrace, 
	fail.
$execute_goal(_, _) :-
	notrace, 
	$ttyformat('~nNo~n'),
	fail.

$user_call(Goal) :-
	Goal.

:- $hide($user_call, 1),
   $show_childs($user_call, 1),
   $predicate_attribute($user_call(_), system, 0).

$write_bindings([]) :- !, 
	$ttyformat('Yes~n').
$write_bindings(Bindings) :-
	repeat,
	    $output_bindings(Bindings),
	    get_respons(Action),
	(   Action == redo
	->  !, fail
	;   Action == show_again
	->  fail
	;   !, format(user_output, '~n~nYes~n', [])
	).

:- flag($toplevel_print_predicate, _, print).

$output_bindings([]) :- !,
	$ttyformat('Yes~n').
$output_bindings([Name = Var]) :- !,
	$output_binding(Name, Var),
	write(user_output, ' '),
	ttyflush.
$output_bindings([Name = Var|Rest]) :-
	$output_binding(Name, Var),
	nl(user_output),
	$output_bindings(Rest).

$output_binding(Name, Var) :-
	write(user_output, Name),
	write(user_output, ' = '),
	flag($toplevel_print_predicate, Pred, Pred),
	Goal =.. [Pred, user_output, Var],
	Goal.

get_respons(Action) :-
	repeat,
	    ttyflush,
	    get_single_char(Char),
	    answer_respons(Char, Action),
	    (   Action == again
	    ->  $ttyformat('Action? '),
		fail
	    ;   !
	    ).

answer_respons(Char, again) :-
	memberchk(Char, "?h"), !,
	show_toplevel_usage.
answer_respons(Char, redo) :-
	memberchk(Char, ";nrNR"), !,
	$format_if_tty(';~n').
answer_respons(Char, redo) :-
	memberchk(Char, "tT"), !,
	trace,
	$format_if_tty('; [trace]~n').
answer_respons(Char, continue) :-
	memberchk(Char, [0'c, 0' , 10, 0'y, 0'Y]), !.
answer_respons(0'b, show_again) :- !,
	break.
answer_respons(Char, show_again) :-
	print_predicate(Char, Pred), !,
	$format_if_tty('~w~n', [Pred]),
	flag($toplevel_print_predicate, _, Pred).
answer_respons(_, again) :-
	$ttyformat('~nUnknown action (h for help)~nAction? '),
	ttyflush.

print_predicate(0'd, display).
print_predicate(0'w, write).
print_predicate(0'p, print).

show_toplevel_usage :-
	$ttyformat('~nActions:~n'),
	$ttyformat('; (n, r):     redo    t:               trace & redo~n'),
	$ttyformat('b:            break   c (ret, space):  continue~n'),
	$ttyformat('d:            display p                print~n'),
	$ttyformat('w:            write   h (?):           help~n').

$format_if_tty(Fmt) :-
	$format_if_tty(Fmt, []).
$format_if_tty(Fmt, Args) :-
	$tty, !,
	$ttyformat(Fmt, Args).
$format_if_tty(_, _).

:- module_transparent
	time/1, 
	$time_call/2.

time(Goal) :-
	statistics(cputime, OldTime), 
	statistics(inferences, OldInferences), 
	$time_call(Goal, Result), 
	statistics(inferences, NewInferences), 
	statistics(cputime, NewTime), 
	UsedTime is NewTime - OldTime, 
	UsedInf  is NewInferences - OldInferences, 
	(   UsedTime =:= 0
	->  Lips = 'Infinite'
	;   Lips is integer(UsedInf / UsedTime)
	), 
	$ttyformat('~D inferences in ~2f seconds (~w Lips)~n',
			[UsedInf, UsedTime, Lips]),
	Result == yes.

$time_call(Goal, yes) :-
	Goal, !.
$time_call(_Goal, no).
