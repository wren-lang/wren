^title Maps
^category types

A map is an *associative* collection. It holds a set of entries, each of which
maps a *key* to a *value*. The same data structure has a variety of names in
other languages: hash table, dictionary, association, table, etc.

You can create a map by placing a series of comma-separated entries inside
curly braces. Each entry is a key and a value separated by a colon:

    :::dart
    {
      "george": "harrison",
      "john": "lennon",
      "paul": mccartney",
      "ringo": "starr"
    }

This creates a map that maps the first names of the Beatles to their last
names. Here, we're using strings for both keys and values.

Values can be any Wren object, and multiple keys may map to the same value.

Keys have a few limitations. They must be one of the immutable built-in [value
types](values.html) in Wren. In other words, a number, string, range, bool, or
`null`. You can also use a [class object](classes.html) as a key. Any given key
can only be present in the map once. If you use it twice, the latter replaces
the former's value.

## Adding entries

**TODO**

## Looking up values

**TODO**

## Removing entries

**TODO**

## Iterating over the contents

**TODO**
