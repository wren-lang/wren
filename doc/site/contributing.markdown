^title Contributing
^category reference

It should be obvious by now that Wren is under active development and there's
lots left to do. I am delighted to have you participate.

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
significant change or addition, please do get in touch first before writing
lots of code to make sure your idea will be a good fit.

## Making a change

The basic process is simple:

1. **Make sure you can build and run the tests locally.** It's good to ensure
   you're starting from a happy place before you poke at the code. Running the
   tests is as simple as:

        :::sh
        $ make test

    If there are no failures, you're good to go.

2. **[Fork the repo][fork] so you can change it locally.** Please make your
   changes in a separate [feature branches][] to make things a little easier on
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

If at any point you have questions, feel free to [file an issue][issue] or
email me (`robert` at `stuffwithstuff.com`). *Thank you!*

[mit]: http://opensource.org/licenses/MIT
[github]: https://github.com/munificent/wren
[fork]: https://help.github.com/articles/fork-a-repo/
[docs]: https://github.com/munificent/wren/tree/master/doc/site
[issue]: https://github.com/munificent/wren/issues
[feature branches]: https://www.atlassian.com/git/tutorials/comparing-workflows/centralized-workflow
[authors]: https://github.com/munificent/wren/tree/master/AUTHORS
[pull request]: https://github.com/munificent/wren/pulls
