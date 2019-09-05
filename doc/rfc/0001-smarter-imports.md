# Smarter Imports

**Note: This is now mostly implemented, though the implementation differs
somewhat from this original proposal.**

Here's a proposal for improving how imported modules are identified and found
to hopefully help us start growing an ecosystem of reusable Wren code. Please
do [let me know][list] what you think!

[list]: https://groups.google.com/forum/#!forum/wren-lang

## Motivation

As [others][210] [have][325] [noted][346], the way imports work in Wren,
particularly how the CLI resolves them, makes it much too hard to reuse code.
This proposal aims to improve that. It doesn't intend to fix *everything* about
imports and the module system, but should leave the door open for later
improvements.

[210]: https://github.com/wren-lang/wren/issues/210
[325]: https://github.com/wren-lang/wren/issues/325
[346]: https://github.com/wren-lang/wren/issues/346

### Relative imports

Today, it's hard to reuse your own code unless you literally dump everything in
a single directory. Say you have:

```text
script_a.wren
useful_stuff/
  script_b.wren
  thing_1.wren
  thing_2.wren
```

`script_a.wren` and `script_b.wren` are both scripts you can run directly from
the CLI. They would both like to use `thing_1.wren`, which in turn imports
`thing_2.wren`. What does `thing_1.wren` look like? If you do:

```scala
// thing_1.wren
import "thing_2"
```

Then it works fine if you run `script_b.wren` from the `useful_stuff/`
directory. But if you try to run `script_a.wren` from the top level directory,
then it looks for `thing_2.wren` *there* and fails to find it. If you change the
import to:

```scala
// thing_1.wren
import "useful_stuff/thing_2"
```

Then `script_a.wren` works, but now `script_b.wren` is broken. The problem is
that all imports are treated as relative to the directory containing the
*initial script* you run. That means you can't reuse modules from scripts that
live in different directories.

In this example, if feels like imports should be treated as relative to the
file that contains the import statement. Often you want to specify, "Here is
*where* this other module is, relative to where *I* am."

### Logical imports

If we make imports relative, is that enough? Should *all* imports be relative? I
don't think so. First of all, some modules are not even on the file system.
There is no relative path that will take you to "random" — it's built into the
VM itself. Likewise, "io" is baked into the CLI.

Today, when you write:

```scala
import "io"
```

You aren't saying *where* that module should be found, you're saying *what*
module you want. Assuming we get a package manager at some point, these kinds of
"logical" imports will be common. So I want these too.

If you look at other langauges' package managers, you'll find many times a
single package offers a number of separate libraries you can use. So I also
want to support logical imports that contain a path too — the import would say
both *what* package to look in and *where* in that package to look.

### Only logical imports?

Given some kind of package-y import syntax, could we get rid of relative imports
and use those for everything? You'd treat your own program like it was itself
some kind of package and anything you wanted to import in it you'd import
relative to your app's root directory.

The problem is that the "root directory" for your program's "package" isn't
well-defined. We could say it's always the same directory as the script you're
running, but that's probably too limiting. You may want to run scripts that live
in subdirectories.

We could walk up the parent directories looking for some kind of "manifest" file
that declares "the root of the package is here", but that seems like a lot of
hassle if you just want to create a couple of text files and start getting some
code running. So, for your own programs, I think it's nice to still support
"pure" relative imports.

### Ambiguity?

OK, so we want both relative imports and logical imports. Can we use the same
syntax for both? We could allow, say:

```scala
import "a/b"
```

And the semantics would be:

1.  Look for a module "a/b.wren" relative to the file containing the import. If
    found, use it.

2.  Otherwise, look inside some "package" directory for a package named "a" and
    a module named "b.wren" inside it. If found use that.

3.  Otherwise, look for a built in module named "a".

This is pretty much how things work now, but I don't think it's a good idea.
Relative imports will tend to be short — often single words like "utils".
Assuming we get a healthy package ecosystem at some point, the chances of one of
those colliding with a logical import name are high.

Also, when reading code, I think it's important to be able to easily tell "this
import is from my own program" without having to know the names of all of the
files and directories in the program.

## Proposal

OK, so here's my goals:

1.  A way to import a module relative to the one containing the import.
2.  A way to import a module from some named logical package, possibly at a
    specific path within that package.
3.  Distinct syntaxes for each of these.

I tried a few different ideas, and my favorite is:

### Relative imports

Relative imports use the existing syntax:

```scala
// Relative path.
import "ast/expr"
```

This looks for the file `ast/expr.wren` relative to the directory containing the
module that has this import statement in it.

You can also walk out of directories if you need to import a module in a parent
folder:

```scala
import "../../other/stuff"
```

### Logical imports

If you want to import a module from some named logical entity, you use an
*unquoted* identifier:

```scala
import random
```

Being unquoted means the names must be valid Wren identifiers and can't be
reserved words. I think that's OK. It would confuse the hell out of people if
you had a library named "if". I think the above *looks* nice, and the fact that
it's not quoted sends a signal (to me at least) that the name is a "what" more
than a "where".

If you want to import a specific module within a logical entity, you can have a
series of slash-separate identifiers after the name:

```scala
import wrenalyzer/ast/expr
```

This imports module "ast/expr" from "wrenalyzer".

## Implementation

That's the proposed syntax and basic semantics. The way we actually implement it
is tricky because Wren is both a standalone interpreter you can run on the
command line and an embedded scripting language. We have to figure out what goes
into the VM and what lives in the CLI, and the interface between the two.

### VM

As usual, I want to keep the VM minimal and free of policy. We do need to add
support for the new unquoted syntax. The more significant change is to the API
the VM uses to talk to the host app when a module is imported. The VM doesn't
know how to actually load modules. When it executes an import statement, it
calls:

```c
char* loadModuleFn(WrenVM* vm, const char* name);
```

The VM tells the host app the import string and the host app returns the code.
In order to distinguish relative imports (quoted) from an identical unquoted
name and path, we need to pass in an extra to bit to tell the host whether there
were quotes or not.

The more challenging change (and the reason I didn't support them when I first
added imports to Wren) is relative imports. There are two tricky parts:

First, the host app doesn't have enough context to resolve a relative import.
Right now, the VM only passes in the import string. It doesn't tell which module
*contains* that import string, so the host has no way of knowing what that
import should be relative *to*.

That's easy to fix. We have the VM pass in the name of the module that contains
the import.

The harder problem is **canonicalization**. When you import the same module
twice, the VM ensures it is only executed once and both places use the same
module data. This is important to ensure you don't get confusing things like
duplicate static state or other weird side effects.

To do that, the VM needs to be able to tell when two imports refer to the "same"
module. Right now, it uses the import string itself. If two imports use the same
string, they are the same module.

With relative imports, that is no longer valid. Consider:

```text
script_a.wren
useful_stuff/
  thing_1.wren
  thing_2.wren
```

Now imagine those files contain:

```scala
// script_a.wren
import "useful_stuff/thing_1"
import "useful_stuff/thing_2"

// useful_stuff/thing_1.wren
import "thing_2"

// useful_stuff/thing_2.wren
// Stuff...
```

Both `script_a.wren` and `thing_1` import `thing_2`, but the import *strings*
are different. The VM needs to be able to figure out that those two imports
refer to the same module. I don't want path manipulation logic in the VM, so it
will delegate to the host app for that as well.

Given the import string and the name of the module containing it, the host app
produces a "fully-qualified" or "canonical" name for the imported module. It is
*that* resulting string that the VM uses to tell if two imports resolve to the
same module. (It's also the string it uses in things like stack traces.)

This means importing becomes a three stage process:

1.  First the VM asks the host to resolve an import. It gives it the (previously
    resolved) name of the module containing the import, the imports string, and
    whether or not it was quoted. The host app returns a canonical string for
    that import.

2.  The VM checks to see if a module with that canonical name has already been
    imported. If so, it reuses that and its done.

3.  Otherwise, it circles back and asks the host for the source of the module
    with that given canonical name. It compiles and executes that and goes from
    there.

So we add a new callback to the embedding API. Something like:

```c
char* resolveModuleFn(WrenVM* vm,
    // Canonical name of the module containing the import.
    const char* importer,

    // The import string.
    const char* path,

    // Whether the path name was quoted.
    bool isQuoted);
```

The VM invokes this for step one above. The other two steps are the existing
loading logic but now using the canonicalized string.

### CLI

All of the policy lives over in the CLI (or in your app if you are embedding the
VM). You are free to use whatever canonicalization policy makes sense for you.
For the CLI, and for the policy described up in motivation, it's something like
this:

*   Imports are slash-separated paths. Resolving a relative path is normal path
    joining relative to the directory containing the import. So if you're
    importing "a/b" from "c/d" (which is a file named "d.wren" in a directory
    "c"), then the canonical name is "c/a/b" and the file is "c/a/b.wren".

    ".." and "." are allowed and are normalized. So these imports all resolve
    to the same module:

    ```scala
    import "a/b/c"
    import "a/./b/./c"
    import "a/d/../b/c"
    ```

*   If an import is quoted, the path is considered relative to the importing
    module's path, and is in the same package as the importing module.

    So, if the current file is "a/b/c.wren" in package "foo" then these are
    equivalent:

    ```scala
    import "d/e"
    import foo/a/b/d/e
    ```

*   If an import is unquoted, the first identifier is the logical "package"
    containing the module, and the remaining components are the path within that
    package. The canonicalized string is the logical name, a colon, then the
    resolved full path to the import (without the ".wren" file extension).
    So if you import:

    ```scala
    import wrenalyzer/ast/expr
    ```

    The canonical name is "wrenalyzer:ast/expr".

*   If an import is a single unquoted name, the CLI implicitly uses the name as
    the module to look for within that package. These are equivalent:

    ```scala
    import foo
    import foo/foo
    ```

    We could use some default name like "module" instead of the package name,
    similar to Python, but I think this is actually a little more usable in
    practice. If you're hacking on a bunch of packages at the same time, it's
    annoying if every tab in your text editor just says "module.wren".

*   The canonicalized string for the main script or a module imported using a
    relative path from the main script is just the normalized file path,
    probably relative to the working directory.

*   Since colon is used to separate the name from path, path components with
    colons are not allowed.

### Finding logical imports

The last remaining piece is how the CLI physically locates logical imports. If
you write:

```scala
import foo
```

Where does it look for "foo"? Of course, if "foo" is built into the VM like
"random", then that's easy. Likewise, if it's built into the CLI like "io",
that's easy too.

Otherwise, it will try to find it on the file system. We don't have a package
manager yet, so we need some kind of simple policy so you can "hand-author" the
layout a package manager would produce. Borrowing from Node, the basic idea is
pretty simple.

To find a logical import, the CLI starts in the directory that contains the main
script (not the directory containing the module doing the import), and looks for
a directory named "wren_modules". If not found there, it starts walking up
parent directories until it finds one. If it does, it looks for the logical
import inside there. So, if you import "foo", it will try to find
"wren_modules/foo/foo.wren".

Once it finds a "wren_modules" directory, it uses that one directory for all
logical imports. You can't scatter stuff across multiple "wren_modules" folders
at different levels of the hierarchy. If it can't find a "wren_modules"
directory, or it can't find the requested module inside the directory, the
import fails.

This means that to reuse someone else's Wren "package" (or your own for that
matter), you can just stick a "wren_modules" directory next to the main script
for your app or in some parent directory. Inside that "wren_modules" directory,
copy in the package you want to reuse. If that package in turn uses other
packages, copy those into the *same* "wren_modules" directory. In other words,
the transitive dependencies get flattened. This is important to handle shared
dependencies between packages without duplication.

You only need to worry about all of this if you actually have logical imports.
If you just have a couple of files that import each other, you can use straight
relative imports and everything just works.

## Migration

OK, that's the plan. How do we get there? I've start hacking on the
implementation a little and, so far, it seems straightforward. Honestly, it will
probably take less time than I spent writing this up.

The tricky part is that this is a breaking change. All of your existing quoted
import strings will mean something different. We definitely *can* and will make
breaking changes in Wren, so that's OK, but I'd like to minimize the pain. Right
now, Wren is currently at version 0.1.0. I'll probably consider the commit right
before I start landing this to be the "official" 0.1.0 release and then the
import changes will land in "0.2.0". I'll work in a branch off master until
everything looks solid and then merge it in.

If you have existing Wren code that you run on the CLI and that contains
imports, you'll probably need to tweak them.

If you are hosting Wren in your own app, the imports are fine since your app
has control over how they resolve. But you will have to fix your app a little
since the import embedding API is going to change to deal with canonicalization.
I think I can make it so that if you don't provide a canonicalization callback,
then the original import string is treated as the canonical string and you
fall back to the current behavior.

## Alternatives

Having both quoted and unquoted import strings is a little funny, but it's the
best I could come up with. For what it's worth, I [borrowed it from
Racket][racket].

[racket]: https://docs.racket-lang.org/guide/module-basics.html

I considered a couple of other ideas which are potentially on the table if
most of you don't dig the main proposal:

### Node-style

In Node, [all imports are quoted][node]. To distinguish between relative and
logical imports, relative imports always start with "./". In Wren, it would be:

[node]: https://nodejs.org/api/modules.html

```scala
import "./something/relative"
import "logical/thing"
```

This is simpler than the main proposal since there are no syntax changes and we
don't need to push the "was quoted?" bit through the embedding API. But I find
the "./" pretty unintuitive especially if you're not steeped in the UNIX
tradition. Even if you are, it's weird that you *need* to use "./" when it means
nothing to the filesystem.

### Unquoted identifiers

The other idea I had was to allow both an unquoted identifier and a quoted
path, like:

```scala
import wrenalyzer "ast/expr"
```

The unquoted name is the logical part — the package name. The quoted part is
the path within that logical package. If you omit the unquoted name, it's a
straight relative import. If you have a name but no path, it's desugars to use
the name as the path.

This is a little more complex because we have to pass around the name and path
separately between the VM and the host app during canonicalization. If we want
the canonicalized form to keep those separate as well, then the way we keep
track of previously-loaded modules needs to get more complex too. Likewise the
way we show stack traces, etc.

The main proposal gloms everything into a single string using ":" to separate
the logical name part from the path. That's a little arbitrary, but it keeps
the VM a good bit simpler and means the idea of there being a "package name" is
pure host app policy.
