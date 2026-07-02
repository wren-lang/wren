# Ticket 005 - Tests

## Goal

Add tests for the minimal bytecode flow.

## Notes

The most important tests are the ones that prove the artifact can round-trip
and behave like the source version for a small set of representative programs.
Keep the first pass focused on the narrow v1 scope.

## Acceptance Criteria

- Header/version rejection tests exist.
- Truncation/corruption tests exist.
- Source-vs-bytecode execution equivalence is covered for a few representative cases.
- Basic class/function/closure cases are covered.
- Tests do not depend on external module loading.
