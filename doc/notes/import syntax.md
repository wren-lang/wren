So we need some syntax to distinguish between a relative import and a logical
import. I'm not sure which way to go, and I'd like some feedback (or possibly
other alternate ideas I haven't considered).

My two favorites are:

```
// Use
use "relative/path"
import "logical/path"

// Node-style
import "./relative/path"
import "logical/path"
```

If you folks are OK with "use", that's my preference. But otherwise, the Node
style will definitely work too. I'm open to other ideas as well, including a few
below, but I'd like to not bikeshed this forever.

## Background

There are four general approaches we can take:

### Use a modifier ("modifier")

Both kinds of imports start with `import`, but then we use a second keyword
afterwards to identify either a relative or logical import. We could use *two*
keywords -- one for each kind -- but that's unnecessarily verbose. Instead, we
use the presence or absence of the keyword to distinguish. In other words:

```
import foo "string"
import "string"
```

The specific questions we have to answer are:

1. Which kind of import gets the keyword? Ideally, the most common kind of
   import would be the one that doesn't need an extra keyword.

2. What keyword? This is surprisingly hard. Probably some kind of preposition.

### Use different keywords ("keyword")

Instead of using `import` for both logical and relative imports, we could have
two keywords, one for each kind. The specific questions to answer then are:

1. Which kind of import gets `import`?
2. What's the other keyword?

### Use different syntax for the path ("syntax")

Instead of always using a string literal to identify what's being imported, we
could use a different kind of token or tokens for the different kinds of import.
For example, a string literal for one kind, and an identifier token for the
other:

import identifier
import "string literal"

The specific questions are:

1. Which kind of import uses a string literal?
2. What's the syntax for the other kind?

### Use a signifier in the import string itself to distinguish ("string")

An import is always `import` followed by a string literal. Then we use some
specific markers inside the string literal itself to distinguish the two kinds.
For example, Node says that an import string starting with "./" or "../" is
relative and other import strings are logical.

The specific question to answer is what kind of signifier we'd use. I think
Node's convention is the only real contender here, though.

One feature this style has that none of the others do is that it means the
language syntax itself has no notion of logical and relative imports. This
means there is no overhead or complexity for host applications where that
distinction isn't meaningful.

## Contenders

These are options I'm open to, in roughly descending order of preference:

### Node-style (string)

If the string starts with "./" or "../", it's relative.

```
import "./relative/path"
import "logical/path"
```

This is how Node works, so there's prior art. It keeps the language completely
simple. It does feel sort of arbitrary and magical to me, but it's the simplest,
most expedient solution.

### Use (keyword)

The `use` keyword is for relative imports, `import` is for logical.

```
use "relative/path"
import "logical/path"
```

The `use` keyword comes from Pascal, but that's not very widely known. I kind
of like this. It's short, and `use` feels "nearer" to me than "import" so it
has the right connotation. (You can't "use" something unless you have it near
to hand.)

It adds a little complexity to the language and VM. We have to support both
keywords and pass that "use versus import" bit through the name resolution
process. But that's pretty minor.

### Slashes (syntax)

If the path is a string literal, it's relative. Otherwise, it is a
slash-separated series of unquoted identifiers.

```
import "relative/path"
import logical/path
```

This means you can't (easily) use reserved words as names of logical imports.
This was my initial pitch. I still like how it looks, but I seem to be in the
minority.

### Relative (modifier)

The `relative` modifier is for relative imports.

```
import relative "relative/path"
import "logical/path"
```

It's explicit, which is good. It is unfortunately verbose. I think `relative`
is too useful of a word to make into a reserved word, which means it would have
to be a contextual keyword (i.e. treated like a reserved word after `import`
but behaving like a regular identifier elsewhere). I'm not generally a fan of
contextual keywords—they tend to make things like syntax highlighters more
difficult to create—so I try to avoid them.

## Rejected

I considered these ideas, but don't think they are good enough approaches for
various reasons:

### Package identifier (syntax)

If an unquoted identifier appears before the import string, then it's a logical
import within that package. Otherwise, it's relative.

```
import "relative/path"
import logical "path"
```

This was one of my initial ideas. It has the same problem as other unquoted
imports in that it makes it harder to have odd package names. It means the VM
has to understand this syntax and figure out how to display package names in
stack traces and stuff, so there is some extra complexity involved.

The form where you have both a package name and a relative path within that
package is pretty unusual and likely unintuitive to users.

### Dotted (syntax)

If the path is a string literal, it's relative. Otherwise, it is a
dot-separated series of unquoted identifiers.

```
import "relative/path"
import logical.path
```

Similar to slashes, but using dots. This helps make logical imports look more
visually distinct from relative ones. But it also makes them look more similar
to getter calls, which they aren't related to at all.

### Include (keyword)

The `include` keyword is for relative imports, `import` is for logical.

```
include "relative/path"
import "logical/path"
```

Ruby uses `include` for applying mixins. "Include" reads to me more like some
kind of transclusion thing, so it feels a little weird.

### Require (keyword)

The `require` keyword is for relative imports, `import` is for logical.

```
require "relative/path"
import "logical/path"
```

Node uses "require" and ES6 uses "import" so this is kind of confusing. Ruby
uses `require` and `require_relative`, so using `require` for a relative import
is kind of confusing. Lua also uses `require`, but for both relative and
logical. Overall, this feels murky and unhelpful to me.

### Angle-brackets (syntax)

As in C/C++, an import string can be in angle brackets or quotes. Angle brackets
are for logical imports, quotes for relative.

```
import "relative/path"
import <logical/path>
```

Hard pass. It requires context-sensitive tokenization (!) in C and we definitely
don't want to go there.

### URI scheme (string)

An import string starting with "package:" and maybe "wren:" is treated as
logical, like they are URIs with an explicit scheme. Others are relative.

```
import "relative/path"
import "package:logical/path"
import "wren:random"
```

This is (roughly) how Dart works. I'm not a fan. I think it's too verbose for
logical imports.

### Package (modifier)

A `package` modifier indicates a logical import. Others are relative.

```
import "relative/path"
import package "logical/path"
```

Pretty long, and I'm not too crazy about baking "package" into the language and
VM.

### From (modifier)

A `from` modifier indicates, uh, one kind of import.

```
import "some/path"
import from "other/path"
```

It looks nice, but it's totally unclear to me whether logical imports should
get `from` or relative ones. Also kind of confusing in that Python and ES6 use
`from` in their notation for importing explicit variables from a module (where
Wren uses `for`).
