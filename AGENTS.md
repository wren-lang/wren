# Agent Guidance

## Model Selection

- Use the current model for fast iteration on planning, ticket writing, and
  small organizational updates.
- Recommend switching to a stronger model when the task becomes architecture-
  heavy, code-aware, or requires careful tradeoff analysis.
- For this bytecode work, suggest a stronger model before making final decisions
  on format, loader behavior, or compiler/runtime coupling.

## Workflow Notes

- Keep bytecode planning docs under `plan/`.
- Treat `plan/README.md` as the index.
- Treat `plan/bytecode-scope.md` as the v1 boundary.
- Use `plan/roadmap.md` for phases and `plan/tickets/` for detailed tasks.

## Scope Reminder

- v1 is single-file only.
- External module loading is out of scope.
- The artifact is version-locked.
- Obfuscation is out of scope.
