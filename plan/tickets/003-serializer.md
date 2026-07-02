# Ticket 003 - Serializer

## Goal

Add the compiler-side export path that writes compiled module state to a
serialized artifact.

## Notes

The serializer should be a thin export layer over the existing compile path.
Spin up a normal `WrenVM`, compile the source the usual way, then write out the
compiled module state. Avoid creating a separate compiler pipeline for v1.

## Acceptance Criteria

- A serializer entry point exists.
- It can write a compiled single-file module to disk or memory.
- It reuses the existing compiler/VM path rather than inventing a new IR.
- It fails cleanly if the compiled output contains something v1 does not support.
- It writes a recognizable artifact header.
