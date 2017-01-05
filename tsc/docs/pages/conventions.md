Coding conventions
==================

This file documents the coding and other procedural conventions used
in TSC. If you plan on contributing to the game, please read this.

The general style used for TSC’s codebase is the [Stroustrup
style][1]. For any formatting questions, you should therefore read
that document prior to consulting this one, which outlines the
additions and differences to the standard Stroustrup style.

Note that for Emacs an appropriate style file is already provided in
the source tree, `tsc/.dir_locals.el`. That file will automatically be
picked up by Emacs when you edit the sourcecode, so you don’t have to
worry about changing the indentation and other style settings
manually. Stil, a style file doesn’t lift the burden of properly
formatting the code, it is just a help.

\tableofcontents

File names
----------

Sourcecode files end in `.cpp`, header files end in `.hpp` in case of
C++ code (which should be everything in TSC). Use `.c` and `.h` if for
some unknown reason pure C code is required.

Indentation
-----------

Code in TSC is indented with 4 spaces, no tabs. Most code editors can
be configured to automatically insert the required number of spaces
when you hit the TAB key.

~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
void foo()
{
    cool_code();
}
~~~~~~~~~~~~~~~~~~~~~~~~~~

License header
--------------

Each file in TSC is required to start with the following license
header:

~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
/***************************************************************************
* <FILENAME>.<cpp/hpp> - <SHORT DESCRIPTION OF THE FILE>
*
* Copyright © <YEAR> The TSC Contributors
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Never change existing copyright notices**, except it is assigned to
“The TSC Contributors” and you only want to update the date to the
current one. For example if you have a line

~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright © 2014 The TSC Contributors
~~~~~~~~~~~~~~~~~~~~~~~~~

and you have changed the file in 2015, you can change it like this:

~~~~~~~~~~~~~~~~~~~~~~~
Copyright © 2014-2015 The TSC Contributors
~~~~~~~~~~~~~~~~~~~~~~~

If later in 2016 you work again on the file, you can change it like
this:

~~~~~~~~~~~~~~~~~~~~~~~
Copyright © 2014-2016 The TSC Contributors
~~~~~~~~~~~~~~~~~~~~~~~

If nobody worked on the file in 2015, but only in 2014 and 2016, the
notice has to look like this (note the comma!):

~~~~~~~~~~~~~~~~~~~~~~~
Copyright © 2014,2016 The TSC Contributors
~~~~~~~~~~~~~~~~~~~~~~~

In any case, do never-ever change notices that are not assigned to
“The TSC Contributors”.

If you contribute to TSC the first time, don’t forget to include
yourself into `docs/authors.txt` so you show up in the credits!

Class names
-----------

As a difference to Stroustrup style, due to historical reasons class
names start with a lowercase c, following by the real name in
Camel_Case.

~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class cFoo_Bar {
    // ...
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Nesting classes is OK for indicating a very strong relationship
between two classes.

~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class cPath {

    class cSegment {
    };

};
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Do not repeat the outer class name in the inner class’ name, it’s
obvious from the namespacing (`cPath::cPathSegment` is **wrong**,
`cPath::cSegment` is correct). Do not nest further than one level
unless absolutely required.

Methods
-------

Another difference to Stroustrup style is method naming, again for
historical reasons. Methods in classes are uppercase, both for member
and static methods, and long names use Camel_Case. Class-independent
methods (i.e. utility methods) are lowercase.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class cFoo {
public:
    void Cool_Method();
};

void utility_method();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Variables and parameters
------------------------

Variable and parameter names use an abbreviated hungarian
notation.

* For local throw-away variables just use a descriptive name,
  don’t apply any prefixes.
* Member variables are prefixed with "m_"
* Static member variables are prefixed with "s_"
* Global variables are prefixed with "g_"

In any case, a variable for a pointer should always contain
"p". Prefixes are combinable. Examples:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
int count; // local variable
int m_count; // member variable
static int s_count; // static member variable
extern int g_count; // global variable
int* p_count; // local-var pointer
int* mp_count; // member-var pointer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Variable access
---------------

Usually member variables should be either `private` or `protected`,
and getter and setter methods should be defined to access these
variables. This way, you can be sure in the implementation of your
class that nobody messes with the variables from the outside in an
unexpected way.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class cFoo {
public:
    int Get_Bar();
    void Set_Bar(int x);

private:
    int m_bar;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Additional method calls have performance overhead. Most getters and
setters are trivial, so you want to squash the performance overhead by
taking advantage of C++’s `inline` keyword that retains the secrecy
principle and still allows the compiler to optimize the method call
away:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class cFoo {
public:
    inline int Get_Bar(){ return m_bar; }
    inline void Set_Bar(int x){ m_bar = x; }

private:
    int m_bar;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Only do this for small one-line accessor methods, otherwise
you abuse the `inline` keyword.

Macros
------

Try to prevent macro use as much as possible. C++ has templates that
will cover a good number of macro usecases, but sometimes you just
can’t avoid a macro (such as the one defining the event handler
callback for mruby (a C library!) functions; doing that without the
`MRUBY_IMPLEMENT_EVENT()` macro would be much more work and much more
unclear than this way). If you come to the conclusion you absolutely
must use a macro, make it use a name IN_ALL_CAPS so it stands out of
the code and warns everyone it is a macro.

The header guards that prevent `#include` from including a header file
multiple times are of course an exception. Header guard macros in TSC
should always look like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#ifndef TSC_NAMESPACE_CLASS_HPP
#define TSC_NAMESPACE_CLASS_HPP
// Code...
#endif
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Do not indent `#ifdef` and `#if defined` conditions. They are not part
of the code and must quickly be found from just glancing over the file
when you want to know whether you are in a Windows- or Linux-specific
part of the code.

Implementing code that needs to differenciate between multiple
different operating systems (or other environment-related macros)
should look like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
void foo()
{
#if defined(_WIN32)
    windows_specific_stuff();
    with_another_line();
#elsif defined(__linux)
    linux_specific_stuff();
#else
#error unsupported platform
#endif
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Be sure to always add an `#else` part containing an `#error` line so
that users that want to compile on an unhandled platform receive a
compilation error rather than a dysfunctional program.

Comments
--------

Comments should be added as suggested in the Stroustrup style
document, i.e. where you have to say things that you can’t express in
code. Comments that repeat what the code is saying already are bad,
especially if they get out of sync with the code itself.

Single and two-line comments use `//`. For comments with three and
more lines, use `/* */`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// Short comment

/* Long comment that explains a more complicated factum that
 * definitely needs more than three lines, because the explanation
 * is so insanely complicated that each less line would prevent
 * the understanding.
 */
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Comments should not exceed a line-length of 80 characters, try to
break at column 60. Use proper English grammar and punctuation. Short
oneline comments may ommit the final period.

Namespace resolution
--------------------

Don’t leave spaces around the `::` operator, neither in a definition
nor in the implementation.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
cFoo :: Bar() // WRONG
{
}

cFoo::Bar() // correct
{
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

External includes
-----------------

All external `#include` statements should be in
`core/global_basic.hpp`. Do not include external headers in other
files. Background for this is that in some cases the order of includes
is important, which cannot easily be guaranteed when headers include
other headers of TSC, which include yet another TSC header, which then
finally includes some library header, etc. `#include` statements for
TSC-own headers can be done anywhere you need them.

TSC namespace
-------------

All TSC code is required to be defined under the `TSC` namespace. In
the header files, always write out `namespace TSC {` in full and
indent accordingly. In your sourcecode files, add a line

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
using namespace TSC;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

near the top so you don’t have to indent all your implementation
code. Please do **not** follow the old TSC style of using `namespace
TSC {` and not indenting the rest of the file, this is highly
discouraged.

Version policy
--------------

TSC uses [semantic versioning](http://semver.org/). In short this
means:

* Version numbers are triplets of form `MAJOR.MINOR.TINY`.
* `TINY` is increased for bugfixes, internal code restructuring, etc.
* `MINOR` is increased when new features are added that maintain
  backward compatibility (especially with regard to level and save
  file formats).
* `MAJOR` is increased when backward compatibility is broken.

Additionally, each compiled executable of TSC knows about the exact
commit’s hash it was compiled from. Execute TSC like this to receive
the full version information:

~~~~~~~~~~~~~~~~~~~~~~~~~~
$ tsc --version
~~~~~~~~~~~~~~~~~~~~~~~~~~

Branching
---------

The version control system (VCS) in use for TSC is
[Git](http://www.git-scm.com/), and the repository currently resides
at [GitHub](https://github.com/Secretchronicles/TSC). For managing
these sources, we generally follow the so-called [“Git
Flow”](http://nvie.com/posts/a-successful-git-branching-model/) model,
which heavily relies on Git’s lightweight branches and in a nutshell
means the following for TSC:

* Each commit on `master` is a final release and is tagged
  accordingly.
* The development of the next feature release happens in `devel`.
* If a regular release is about to be shipped, a branch
  `release-X.Y.Z` is created, where `X.Y.Z` is the version
  number. This branch only receives bugfixes, and it is merged every
  now and then into `devel` to make it also receive the bugfixes.
* Larger new features in development are branched off `devel` into a
  branch named `feature-something`, where `something` is a short
  description of the feature. `devel` should be merged every now
  and then into the feature branch. When the feature is completed,
  it is merged back into `devel` (unless it breaks compatibility, see
  below).

Beware the following exception, which is a **difference to official
GitFlow**: As we use semantic versioning (see the preceeding section),
we must be careful as to what to merge into `devel` as `devel` will
automatically become the codebase for the next release. Semantic
versioning however requires us to handle features or other changes
that break backward compatibility specifically, they are not allowed
to just go into the next release, i.e. into `devel`, so that the user
is able to derive compatibility information from TSC’s version
number. To prevent feature branches from getting silently out-of-date
when they are completed and not merged into devel, we maintain
specific `devel-X.0.0` branches that serve the purpose of the `devel`
branch for the next **breaking** release. The lifecycle of a feature
branch whose feature breaks backward compatibility hence looks like
this:

1. If you know from the beginning your feature will break backward
  compatibility, branch off `devel-X.0.0` a branch named
  `bfeature-something` (note the leading `b`). If you don’t know that,
  or if there is no `devel-X.0.0` branch yet, branch off `devel`. Once
  you know your feature will break, correct your branch name (by
  creating a new `bfeature` branch and merging your `feature` branch
  into it, then deleting the `feature` branch).
2. Make your development in the `bfeature` branch, occasionally
  merging `devel-3.0.0`, or if that doesn’t exist, `devel` into your
  branch.
3. Once the breaking feature is completed, merge it into
  `devel-X.0.0`. If there is no `devel-X.0.0` branch at that time,
  punch the project lead until he creates one.

Occasionally, `devel` will be merged into `devel-X.0.0`, but this
doesn’t have to be done by you. Also, when the breaking major release
is finally due, `devel-X.0.0` will be merged into `devel`, as it then
constitutes the next release anyway.

If you feel the need to discuss the code you wrote, be it a feature or
a bugfix, open a pull request on GitHub even if you have write access
to the repository; GitHub allows pull requests in the same
repository. If your code is not ready yet, prepend a prominent “[WIP]” (=
Work In Progress) mark to the title.

Once your code has been merged into one of the main development or
release branches, please delete the old branch on the remote so that
it doesn’t clutter the branch list anymore:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git checkout devel
$ git branch -d yourbranchname
$ git push origin :yourbranchname
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first command switches your working area away from the branch that
is going to be deleted, the second command deletes the branch locally,
and the third command pushes the deletion onto the remote (note the
colon in the command).

Merging
-------

From time to time, the project receives _pull requests_ (PRs) or other
forms of contributions. This sections outlines how to proceed with
them.

In general, the commit log should contain valid metadata, i.e. each
commit should be attributed to the person who really authored the
content. While this seems obvious, it isn’t that easy when you cope
with someone who doesn’t know how to use git, e.g. artists. In this
case, you have to ensure the commit is really attributed correctly to
the artist, and not to you; see the below section on non-git
contributions for further information on this.

Additionally, hackers are used to not use their real name in the
author information of commits. This is culture, so don’t try to do
something against this. On the other hand, even most hackers are eager
to see their real name in the credit information of a program, mostly
as larger programs serve as a great reference for hiring. In TSC, we
resolve this little conflict by the special file `docs/authors.txt`,
which contains a list of all people who have any kind of contribution
currently in the game. That file maps a real name to a nickname, and
this file is parsed and compiled into a C++ object file during the
build process, serving the crediting information for the credits menu
in the compiled game later on. Additionally, this file allows the team
to track who holds legal rights on the game in some way. In case of a
lawsuit (hope we won’t get there), the email addresses in that file
(hopefully) allow us to contact all the persons who hold rights on the
game and ask them for whatever actions may be needed in the course of
the proceedings. The format of the file should be easy to follow; just
ensure you keep the alphabetical order so nobody complains about being
listed first or last.

For people, who have done much for the game, but have no direct
contributions to the code or the assets (e.g. packaging for Linux
distributions), and hence do not hold copyright on the game, there’s a
separate file, `docs/specialthanks.txt`, which is also read in and
shown on the credits screen.

When you merge anything other than code into the game, please try to
get hold of some kind of proof of the license. For example, if someone
sends you some great peace of music by email and states it is
"CC-BY-SA 3.0" or so, save the email as a separate file, and commit
that email file to the private licensing repository on the Alexandria
server (`/var/local/repos/TSC-Licensing.git`). For any kind of
contribution via public channels it is sufficient if the author of the
contribution mentions the license publically. Specifically for
contributions via the forum, [the forum
rules](https://forum.secretchronicles.org/forum_rules) already guarantee a
proper license so that further proof is not required.

This is some precaution against the possibility some malicious man
tries to sue us for using his music or other asset, although he
allowed us to use it.

The license for any code contributed is the GPLv3 or later. This is
obvious from our code license, the GPLv3: When the contributor takes
the sourcecode and changes it, his changes are automatically licensed
under GPLv3 due to its viral effect. This is how the GPLv3 works.

### Pull Requests ###

PRs can either be filed on GitHub, where they appear at [a special
repository page](https://github.com/Secretchronicles/TSC/pulls), or
are emailed to you. As a team member with write access, you should
review the PRs on that GitHub page every now and then to ensure that
the people who have filed a PR receive a timely response, even if it
is just a “thank you, but I currently can’t review for time
constraints” or something like that. For email PRs, this goes as well.

When you find the time, review the PR. That is, check it against the
coding guidelines in this document and your common sense; point out
obvious mistakes and ask people to fix problems with the coding
style. If the PR is made against the wrong branch (see above), guide
them carefully to re-file the PR so it applies to the correct
branch; this is necessary, because different branches may contain
entirely different code. It is the requestor’s responsibility to fix
conflicts that appear when his request needs to be merged into another
branch.

Don’t reject a PR for only one or two simple errors. Instead, ask the
requestor to fix them, and if he doesn’t, just merge the request
anyway and fix the errors yourself. As an exception from this rule,
**never** merge a PR that uses tabs for indentation. This destroys the
entire overview of the commit diff(s).

There’s no hard requirement how many people from the core team need to
agree with a PR. Just merge it if you find it to be good, but please
make sure you understand the implications. If for example there’s a
pull request that changes the entire powerup handling to something
totally different, it is obvious that such a grave change needs some
discussion among all team members.

Ask the requestor to describe his changes; it is hard to just judge
from the code what really happens. Pull requests without descriptions
that are more than some trivial addition/fix should not be merged
unless explained.

### Git patches ###

Sometimes people don’t want to go through the hazzle of creating a
publically available Git repository, uploading their changes there,
and filing a PR. Git has a mechanism that allows to create a special
form of patch that can be attached to an email or other means of
communication. These patch files, which are created by the `$ git
format-patch` command, contain both a regular `diff` and the metadata
needed for a commit. You can use `$ git am` to apply these patches if
you receive them.

### Other contributions ###

It happens that people would like to have content included into the
game, but they don’t know how to use Git. This shouldn’t stop you from
including valuable content, instead, use the `--author` option of the
`$ git commit` command. This results in the commit’s metadata being
adjusted so that the contribution is properly assigned to the real
contributor rather than you. Especially for legal reasons, correct
metadata in the commit log is invaluable information. As an example,
if you receive a graphic contribution by "John Doe", whose nickname is
"jonny" and whose email is "john.doe@example.com", you’d do the
following:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git add file1 file2
$ git commit --author="jonny <john.doe@example.com>"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Ensure that you add the full real name with nickname mapping to
`docs/authors.txt` as outlined already.

Signoff
-------

There are times when you want to commit something into the main
development branch from which you don’t really know whether it’s
legally acceptable. Take for example a graphic that has
been sent to the mailinglist with the goal to get it included into the
game. Due to timing constraints, nobody answers the contribution, and
when three days later someone comes around and reviews it and finds it
good, the author has disappeared and is not catchable anymore. The
graphic is awesome, but the contribution email did not include any
license information. Now, if you decide to add it into one of
the main branches, it will fall back on you in case of possible legal
problems following. You probably don’t want that. So, what do do?

The hierarchy in open-source projects is usually very flat. Some
structures, though, are inevitable. One of them is the project
leadership, which includes the burden to finally decide on all
problems that cannot be answered by the team in a collective mannor
for whatever reason. Unless a lawyer joins the team, this also
includes the final decision on legal questions regarding
contributions. Thus, the answer to the above question is to have the
project lead or the project assistant lead “signoff” your commit. With
the signoff, **the person who does the signoff certifies that to the
best of his knowledge the contribution is legally OK**.

The submission procedure for a signoff request is as follows. Extract
the legally fragile content in such a way you can add it in as a
single commit. If it’s a code patch in Git format, that has already
been done for you. If it isn’t, you have to construct a patch in Git
format by first including and commiting the contribution as usual (if
required, under use of the `--author` option as described above). Then
create the Git patch:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git format-patch HEAD~1
$ git reset --hard HEAD~1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This creates a `.patch` file in the current working directory and then
immediately removes the commit from the Git history so you don’t
accidentally push it onto the remote. Send the patch file to one of
the persons mentioned above by email or post it on the tracker.

This is all you have to do; the project lead/assistant lead will keep
track of the rest. For reference, what he’ll be doing is the
following: First, he will carefully check if the contribution is
legally safe. If he comes to the result it isn’t, it will not get
in. Period. If he is sure all is well, then he will add the patch back
in using the `git am` command. Then he will do the classic signoff
command:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git commit --amend --gpg-sign=THEGPGKEYID --signoff
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This keeps the `author` metadata information and additionally adds
`signoff` metadata information. The `signoff` information will then be
set to the person who did the signoff (i.e. the project lead/assistant
lead). Afterwards, an ordinary `git push` will follow to add the
signed-off commit to the remote repository.

The signoff procedure has the purpose of handing the bloat load of the
legal question off to someone different so you don’t have to deal with
details you don’t know how to deal with. It’s *not* a way to get any
weirdly licensed code into the project codebase; the maintainer doing
the signoff still has to check the legal circumstances and may well
reject the contribution if he isn’t certain about permission. The
maintainer doing the signoff also will not necessarily check the
contribution semantically; whether it solves an important problem or
has a good code quality is not checked. The signoff procedure solely
focuses on the legal aspect; it is on you as a developer to check the
contribution matches the conventions layed down in this document. If
you want to initiate a technical peer-review of the contribution, file
a usual pull request again the target branch.

### Third party’s pull request ###

If you have to deal with an entire pull request full of questionable
stuff that’s a problem. Signing off all the commits in the PR is
not practicable; thus the pull request has to be squashed into one
single commit the reviewer can sign off. You might be able to do it
like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git pull git://git.example.com thebranch
$ git reset <initial commit>
$ git add .
$ git commit --amend -m "Squashed version of git://git.example.com thebranch"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Replace `<initial commit>` with the initial commit of the target
branch, which you can find by using a tool that displays Git
history graphically such as `gitk`. The `reset` command will
discard all the commits apart from the branch’s initial commit, but
leave the changes in the working directory. Using `git commit
--amend`, you overwrite the initial commit of the branch with the
contents of the working directory. The commit message you use should
contain the URL where you pulled from originally. Also ensure the
author information of that commit is correct.

Now you have a single commit you can apply the above procedure to
(i.e. `format-patch`, etc.).

Documentation
-------------

TSC uses [Doxygen][2] for documentation. Inside the header files,
please only provide Doxygen "brief" descriptions of the elements (if
any) in order to not clutter the header files, which this way can be
used as a short quick-reference (Doxygen uses the "brief" comment for
display in the function summary). For the more complete descriptions,
use doxygen "full" documentation comments around the actual
definitions in the `.cpp` files. Doxygen comment style is `/**` for
full comment blocks, and `///` for brief comments.

Example `foo.hpp`:

\verbatim
class cFoo
{
   /// Does important things.
   void Bar();
};
\endverbatim

Example `foo.cpp`:

\verbatim
/**
 * This method does important things by
 * implementing the XYZ pattern. It
 * can be used...
 */
void cFoo::Bar()
{
    // Code...
}
\endverbatim

Example of a fully documented function:

\verbatim
// .hpp file
class cFoo {
public:
    /// Do important things with the position.
    Bar(int x, int y, int count);
};
\endverbatim

\verbatim
// .cpp file

/**
 * This method does important things with
 * the given position.
 *
 * \param x     The X coordinate.
 * \param y     The Y coordinate.
 * \param count The number of times to do it.
 *
 * \returns whether the action was successful.
bool cFoo::Bar(int x, int y, int count)
{
    // Code...
}
\endverbatim

Non-code contributions
----------------------

If you are contributing assets other than source code, you have to
explicitely state the license for your asset at a place that is not
the project’s Git repository itself. If you are not a team member,
this is usually easy because you have to file a ticket on the tracker
or email or otherwise contact the team, and in this contact message
you can simply state your licensing terms. It is recommended to
license non-code contributions as CC-BY 4.0 or later to ensure we can
use and modify the asset.

If you are a team member, you have Git access. Just committing the
assets to the Git repository may be a bit of a temptation, but you
should not do that, at least not only do that, because there needs to
be some kind of licensing confirmation outside of the Git
repository. It is recommended to file a new Pull Request for the
graphics you want to include and state your licensing terms
there. After you did that, you can merge the Pull Request yourself or
wait for someone else from the team to review and merge it. Might be
good to have discussion regarding new assets anyway. The recommended
workflow that eases this process looks like this:

1. Create a new branch for your assets that branches off the current
   release branch, if any, or the `devel` branch.
2. Create the assets in one or more commits, as you see fit.
3. File a public Pull Request on GitHub against the branch you
   branched off the assets branch.
4. In the Pull Request, state the licensing.
5. Optionally, have some discussions with the other team members over
   the new assets.
6. Merge the Pull Request.
7. Delete the asset branch.

For the curious: The explicit licensing statement is not needed for
source code, because TSC’s code is only licensed as GPLv3+. This means
that if you add to it, you are forced to license your code as GPLv3+
as well, because any different licensing would be a copyright
infringement. Graphics, music, and other assets are not part of the
source code, but new works on their own, without the GPL expanding to
them. That is why the team needs a licensing confirmation on non-code
assets.

Miscellanea
-----------

* Use `true` and `false` instead of `1` and `0`.
* A function should ideally fit onto a single screen page. While this
  is not always possible (both due to logical code requirements and
  different screen sizes), you should ask yourself if refactoring is
  needed when your function gets a thousand lines long.

State of transition
-------------------

TSC’s codebase has seen different coding styles in the past. You will
find that most of TSC does not correspond to this styleguide, but we
are working on it. Some day we will have it done...

[1]: http://www.stroustrup.com/Programming/PPP-style-rev3.pdf
[2]: http://www.stack.nl/~dimitri/doxygen
