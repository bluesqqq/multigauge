# Doxygen Style

This document defines the Doxygen style for `multigauge-core`.

The goal is to keep public header docs clear, consistent, and focused on the API contract.

## Scope

These rules primarily apply to public declarations in headers.

This includes:

- public functions and methods
- constructors
- destructors
- getters
- setters

Not every public member needs a comment. Document public members only when they are non-trivial.

## Required Function Documentation

Every public function or method in a header should include:

- a brief
- `@param` for every parameter
- `@return` for every non-`void` function

## Brief Rules

The first sentence is the brief. `@brief` is not required.

The brief should:

- begin with a present-tense verb for functions and methods
- state what the function does
- be short and direct
- focus only on function purpose

The brief must not include:

- examples
- return semantics
- rationale
- TODOs
- historical context
- casual commentary
- UX commentary

Good:

```cpp
/// Removes an element from the gauge face.
```

Bad:

```cpp
/// Removes an element from the gauge face and returns the former parent ID.
```

```cpp
/// Removes an element from the gauge face.
///
/// For example, this is commonly used after a drag-delete interaction.
```

This rule is for functions and methods.

Type and member comments do not need to follow it if a noun phrase reads better.

## Parameter Rules

Every parameter must have an `@param`.

- Short descriptions are fine.
- Do not write filler text.
- Document caller-facing details when they matter.

This includes things like:

- units
- coordinate spaces
- indexing rules
- nullability
- ownership
- path syntax
- valid ranges

Good:

```cpp
/// @param index Desired zero-based sibling index, or a negative value to append.
```

## Return Rules

Every non-`void` function must have an `@return`.

`@return` should explain how to read the return value.

- If the meaning is simple, keep it short.
- If the return value is structured, document the payload shape.
- If machine-readable error codes matter to callers, document them.

If the function returns structured data, always document its shape.

Good:

```cpp
/// @return `{ "id": uint, "parentId": uint }` for the inserted element,
/// or one of `NoFace`, `ParentNotFound`, `BadJson`, or `InsertFailed`.
```

Bad:

```cpp
/// Adds an element from JSON and returns `{ "id": uint, "parentId": uint }`.
```

## Nullability And Ownership

Document nullability and ownership explicitly when they matter to the public contract.

This applies especially to:

- raw pointers
- smart pointers
- pointer-like handles
- references whose lifetime is not obvious
- returned objects whose ownership is transferred, borrowed, shared, or retained

Document nullability in `@param` or `@return` when needed.

State things like:

- whether a pointer argument may be `nullptr`
- whether a returned pointer may be `nullptr`
- whether a reference is expected to outlive the object using it
- whether ownership is transferred, retained, borrowed, or shared

Good:

```cpp
/// @param parent Parent element. Must not be `nullptr`.
/// @return Attached face pointer, or `nullptr` if no face is set.
```

For ownership, prefer direct words such as:

- owns
- does not own
- transfers ownership
- retains ownership
- borrows
- shares ownership

Do not leave ownership unclear when it affects correct API use.

## Constraints And Notes

Caller-facing constraints should go in `@note`, not in the brief.

Use `@note` only for API contract constraints.

Use `@note` for things like:

- root element restrictions
- indexing behavior
- coordinate space requirements
- path format requirements
- special input interpretation rules

`@note` must not be used for:

- examples
- rationale
- implementation details
- historical context
- TODOs
- casual commentary
- UX commentary
- return semantics that belong in `@return`
- repeating what already belongs in the brief

Do not use `@note` as a catch-all for extra prose.

Good:

```cpp
/// Reorders an element within its current parent.
///
/// @note Negative indices move the element to the end.
```

## What To Leave Out

Do not include the following in public header Doxygen unless they are directly part of the public contract:

- examples
- rationale
- TODOs
- historical context
- casual commentary
- UX commentary
- implementation details

Implementation details should usually be omitted.

Include them only when the implementation difference is itself part of the public contract.

## Implementation Details

Public header Doxygen should describe the contract, not the implementation.

Most of the time, implementation details do not belong in Doxygen.

A rare exception is when different implementations are meaningful to users of the API.

Example:

- two functions with intentionally different algorithmic behavior that the caller must choose between

## Public Members

Document public members only when they are non-trivial.

Do not add comments to public members purely for completeness if their meaning is already obvious.

## Consistency

Similar functions should use similar wording.

Consistency matters.

This applies especially to:

- overloads
- getters and setters
- related mutation functions
- families of query functions

## Preferred Order

Use this order for function documentation:

1. brief
2. `@note` if needed
3. `@param`
4. `@return`
5. anything else

## Header Vs Source Comments

These rules are mainly for public header documentation.

Comments in source files can be more flexible and may describe:

- implementation details
- tricky logic
- internal invariants
- algorithm-specific reasoning

Do not force source-file comments to follow this exact public Doxygen structure.
