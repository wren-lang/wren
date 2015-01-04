^title Embedding API
^category reference

As an embedded scripting language, the C API your host app uses to interact
with Wren is one of the key facets of the system. It's so important that... I
haven't fleshed it out much yet.

I believe good API design can't be done in a vacuum and I haven't built many
applications that embed Wren yet, so I don't have a good testbed for the
embedding API. Now that the language itself is further along, I'm starting to
work on this, but it isn't quite there yet. Feedback and contributions are
definitely welcome!

In the meantime, you can see the current API in
[`wren.h`](https://github.com/munificent/wren/blob/master/include/wren.h).