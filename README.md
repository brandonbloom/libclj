# Overview

This is an implementation of Clojure's reader in C.

It's got a relatively complete tokenizer, but is far from complete as a useful parser.

I'm publishing this source code in its incomplete form for two reasons:

1. I don't have more time to work on it, right now.
2. Rich Hickey published a spec for ["EDN"](https://github.com/richhickey/edn)

There is also the beginnings of a [Ruby extension](https://github.com/brandonbloom/libclj-ruby).


### Design

The reader machinery uses a callback, rather than return values. This simplified the memory management strategy, allows for streaming readers to be implemented, and creates flexibility for language extension implementers.
