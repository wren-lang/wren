^title Maps

A map is an *associative* collection. It holds a set of entries, each of which
maps a *key* to a *value*. The same data structure has a variety of names in
other languages: hash table, dictionary, association, table, etc.

You can create a map by placing a series of comma-separated entries inside
curly braces. Each entry is a key and a value separated by a colon:

<pre class="snippet">
{
  "maple":  "Sugar Maple (Acer Saccharum)",
  "larch":  "Alpine Larch (Larix Lyallii)",
  "oak":    "Red Oak (Quercus Rubra)",
  "fir":    "Fraser Fir (Abies Fraseri)"
}
</pre>

This creates a map that associates a type of tree (key) to a specific 
tree within that family (value). Syntactically, in a map literal, keys 
can be any literal, a variable name, or a parenthesized expression. 
Values can be any expression. Here, we're using string literals for both keys 
and values.

*Semantically*, values can be any object, and multiple keys may map to the same
value. 

Keys have a few limitations. They must be one of the immutable built-in
[value types][] in Wren. That means a number, string, range, bool, or `null`.
You can also use a [class object][] as a key (not an instance of that class, 
the actual class itself).

[value types]: values.html
[class object]: classes.html

The reason for this limitation&mdash;and the reason maps are called "*hash*
tables" in other languages&mdash;is that each key is used to generate a numeric
*hash code*. This lets a map locate the value associated with a key in constant
time, even in very large maps. Since Wren only knows how to hash certain
built-in types, only those can be used as keys.

## Adding entries

You add new key-value pairs to the map using the [subscript operator][]:

[subscript operator]: method-calls.html#subscripts

<pre class="snippet">
var capitals = {}
    capitals["Georgia"] = "Atlanta"
    capitals["Idaho"] = "Boise"
    capitals["Maine"] = "Augusta"
</pre>

If the key isn't already present, this adds it and associates it with the given
value. If the key is already there, this just replaces its value.

## Looking up values

To find the value associated with some key, again you use your friend the
subscript operator:

<pre class="snippet">
System.print(capitals["Idaho"]) //> Boise
</pre>

If the key is present, this returns its value. Otherwise, it returns `null`. Of
course, `null` itself can also be used as a value, so seeing `null` here
doesn't necessarily mean the key wasn't found.

To tell definitively if a key exists, you can call `containsKey()`:

<pre class="snippet">
var capitals = {"Georgia": null}

System.print(capitals["Georgia"]) //> null (though key exists)
System.print(capitals["Idaho"])   //> null 
System.print(capitals.containsKey("Georgia")) //> true
System.print(capitals.containsKey("Idaho"))   //> false
</pre>

You can see how many entries a map contains using `count`:

<pre class="snippet">
System.print(capitals.count) //> 3
</pre>

## Removing entries

To remove an entry from a map, call `remove()` and pass in the key for the
entry you want to delete:

<pre class="snippet">
capitals.remove("Maine")
System.print(capitals.containsKey("Maine")) //> false
</pre>

If the key was found, this returns the value that was associated with it:

<pre class="snippet">
System.print(capitals.remove("Georgia")) //> Atlanta
</pre>

If the key wasn't in the map to begin with, `remove()` just returns `null`.

If you want to remove *everything* from the map, like with [lists][], you call
`clear()`:

[lists]: lists.html

<pre class="snippet">
capitals.clear()
System.print(capitals.count) //> 0
</pre>

## Iterating over the contents

The subscript operator works well for finding values when you know the key
you're looking for, but sometimes you want to see everything that's in the map.
You can use a regular for loop to iterate the contents, and map exposes two 
additional methods to access the contents: `keys` and `values`. 

The `keys` method on a map returns a [Sequence][] that [iterates][] over all of
the keys in the map, and the `values` method returns one that iterates over the values.

[sequence]: modules/core/sequence.html
[iterates]: control-flow.html#the-iterator-protocol

Regardless of how you iterate, the *order* that things are iterated in 
isn't defined. Wren makes no promises about what order keys and values are 
iterated. All it promises is that every entry will appear exactly once.

**Iterating with for(entry in map)**   
When you iterate a map with `for`, you'll be handed an _entry_, which contains
a `key` and a `value` field. That gives you the info for each element in the map.

<pre class="snippet">
var birds = {
  "Arizona": "Cactus wren",
  "Hawaii": "Nēnē",
  "Ohio": "Northern Cardinal"
}

for (bird in birds) {
  System.print("The state bird of %(bird.key) is %(bird.value)")
}
</pre>

**Iterating using the keys**   

You can also iterate over the keys and use each to look up its value:

<pre class="snippet">
var birds = {
  "Arizona": "Cactus wren",
  "Hawaii": "Nēnē",
  "Ohio": "Northern Cardinal"
}

for (state in birds.keys) {
  System.print("The state bird of %(state) is " + birds[state])
}
</pre>

<br><hr>
<a class="right" href="method-calls.html">Method Calls &rarr;</a>
<a href="lists.html">&larr; Lists</a>
