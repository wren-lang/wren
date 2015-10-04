^title Contributing
^category reference

It should be obvious by now that Wren is under active development and there's
lots left to do. We'd be delighted to have you participate.

## Getting acquainted

[Pull requests][pull request] and [bug reports][issue] are always welcome. But,
if you'd like to dip your toes in the water before diving in, please join
 [the mailing list][list] and say, "Hi". There's no strangers to Wren, just
friends we haven't met yet.

## The source

Wren uses the OSI-approved [MIT license][mit]. I'm not sure exactly what that
means, but I went with the most permissive license I could find.

The source is developed [on GitHub][github]. My hope is that the codebase,
tests, and [documentation][docs] are easy to understand and contribute to. If
they aren't, that's a bug.

## Finding something to hack on

Eventually, the [issue tracker][issue] will be populated with a more complete
set of changes and features I have in mind. Until then, one easy way to find
things that need doing is to look for `TODO` comments in the code.

Also, writing code in Wren and seeing what problems you run into is incredibly
helpful. Embedding Wren in an application will also exercise lots of corners of
the system and highlight problems and missing features.

Of course, new ideas are also welcome as well! If you have an idea for a
significant change or addition, please file a [proposal][] to discuss it
before writing lots of code. Wren tries very *very* hard to be minimal which
means often having to say "no" to language additions, even really cool ones.

## Hacking on docs

The [documentation][] is one of the easiest&mdash;and most
important!&mdash;parts of Wren to contribute to. The source for the site is
written in [Markdown][] (and a little [SASS][]) and lives under `doc/site`. A
simple Python script, `util/generate_docs.py`, converts that to HTML and CSS.

[documentation]: /
[markdown]: http://daringfireball.net/projects/markdown/
[sass]: http://sass-lang.com/

The site uses [Pygments][] for syntax highlighting, with a custom lexer plug-in
for Wren. To install that, run:

[pygments]: http://pygments.org

    :::sh
    $ cd util/pygments-lexer
    $ sudo python setup.py develop
    $ cd ../.. # Back to the root Wren directory.

Now you can build the docs:

    :::sh
    $ make docs

This generates the site in `build/docs/`. You can run any simple static web
server from there. Python includes one:

    :::sh
    $ cd build/docs
    $ python -m SimpleHTTPServer

Running `make docs` is a drag every time you change a line of Markdown or SASS,
so there is also a file watching version that will automatically regenerate the
docs when you edit a file:

    :::sh
    $ make watchdocs

## Hacking on the VM

The basic process is simple:

1. **Make sure you can build and run the tests locally.** It's good to ensure
   you're starting from a happy place before you poke at the code. Running the
   tests is as simple as:

        :::sh
        $ make test

    If there are no failures, you're good to go.

2. **[Fork the repo][fork] so you can change it locally.** Please make your
   changes in separate [feature branches][] to make things a little easier on
   me.

3. **Change the code.** Please follow the style of the surrounding code. That
   basically means `camelCase` names, `{` on the next line, keep within 80
   columns, and two spaces of indentation. If you see places where the existing
   code is inconsistent, let me know.

4. **Write some tests for your new functionality.** They live under `test/`.
   Take a look at some existing tests to get an idea of how to define
   expectations.

5. **Make sure the tests all pass, both the old ones and your new ones.**

6. **Add your name and email to the [AUTHORS][] file if you haven't already.**

7. **Send a [pull request][].** Pat yourself on the back for contributing to a
   fun open source project! I'll take it from here and hopefully we'll get it
   landed smoothly.

If at any point you have questions, feel free to [file an issue][issue] or ask
on the [mailing list][list]. You can also email me directly (`robert` at
`stuffwithstuff.com`) if you want something less public. *Thank you!*

[mit]: http://opensource.org/licenses/MIT
[github]: https://github.com/munificent/wren
[fork]: https://help.github.com/articles/fork-a-repo/
[docs]: https://github.com/munificent/wren/tree/master/doc/site
[issue]: https://github.com/munificent/wren/issues
[proposal]: https://github.com/munificent/wren/labels/proposal
[feature branches]: https://www.atlassian.com/git/tutorials/comparing-workflows/centralized-workflow
[authors]: https://github.com/munificent/wren/tree/master/AUTHORS
[pull request]: https://github.com/munificent/wren/pulls
[list]: https://groups.google.com/forum/#!forum/wren-lang
