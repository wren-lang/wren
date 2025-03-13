## Save and restore a snapshot of a Wren VM

NOTE: still in flux!

### Identify an Obj independently...

That's *pointer unswizzling*.

#### ... from a bare C pointer

References as pointers are everywhere, but we need to identify an Obj.

#### ... from the order of the linked list of all Obj

Iterating on the singly-linked list of all Obj from `vm->first`.
When saving an existing VM, we iterate them from the most recently created to the oldest created (e.g. the Object class).
When restoring a snapshot, the first item read, which was the most recently created, is now created first, and will become the oldest created.



### Census

The VM must be paused.
It's related to a VM, yet it has no pointer to it.

GC is forbidden meanwhile saving/restoring
nothing is hold!



Can now fork a VM :-)
Conceptually extensible to a full hibernation/thaw



The serialization file format is underspecified.
Upon de-serialization, not enough tests are done.
[KINDS-OF-DOC]: https://github.com/munificent/craftinginterpreters/blob/master/note/design%20breaks.md
Values of enum ObjType are reused as is, thus no longer changeable.




# Safety

ALLOCATE is expected to succeed. (As elsewhere in the WrenVM.)



# TODO Security

A fuzzy snapshot is an art better suited to museums than computer science. :-)

what if restoring 0 ObjFOO? Does ALLOCATE work?

what if ObjFn#9 references:
- ObjFn#1000?
- itself?

vm->modules must be a Map from ObjString to ObjModule

    mkdir test/snapshot/restore/bad...


When restoring, an absent `fn->debug->name` could be synthesized as `fn0`

Validate bytecode
- opcodes are known
- opcode arg is valid WRT constants count
- all jumps land on an instruction start
- ...




## Unit tests

hijacked^Wenhanced test/test.c

all but 5 tests pass
- you must preserve line numbers (with the '1' char), otherwise many sadness, such as:
> FAIL: test/limit/too_many_inherited_fields.wren
>       Expected runtime error on line 139 but was on line 0.
- The 5 failing tests are the ones of APITest_Run()
  - they use the VM after its initialization, but the transient VM is not a replacement of the tested one.



# TODO

# should re-shuffle code

src/snapshot/count.c
src/snapshot/types.h
src/snapshot/save.h
src/snapshot/restore.h

## easy, but require to unmark static or re-shuffle code

String restore
Map resize


TODO:
- TAG_NAN saved when WREN_NAN_TAGGING can't be restored in a VM without WREN_NAN_TAGGING.

dedup strings more

dedup other?
  CONST_STRING should memoize
sizeof(sObj) is 32; CAN shrink
NOT fool-proof
  src/vm/wren_vm.c:533
    bindMethod() should IS_CLOSURE()
RLE for:
- empty methods
- debugLines

what if many local vars on top-level?

CHAR(oneCharStr) should be CHAR(c)

in src/vm/wren_vm.c, should wrenCollectGarbage() gray the 11 vm->FOOclass?
  no



# FlatBuffers in C, flatcc

need a lib?
it adds padding.
must hook its allocation to ALLOCATE()

[]: https://flatbuffers.dev/md__internals.html
> scalars of various sizes, all aligned to their own size

> Strings are simply a vector of bytes, and are always null-terminated.
