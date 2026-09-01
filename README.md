# Vix Standard

The standard building blocks of Vix.cpp.

Vix Standard is the general-purpose surface shared across different kinds of C++ software built with Vix.

It is designed around a small set of concepts that a developer can learn once and reuse while building libraries, command-line tools, applications, services, engines, tests, and other C++ software.

Standard does not replace the C++ standard library and does not define an application architecture.

Its purpose is to reduce unnecessary fragmentation between recurring operations and concepts while keeping C++ semantics, performance, and compile-time cost visible.

## Direction

Standard is being designed around two areas.

### Common forms

Small concepts whose semantics can remain useful across independent domains.

Current areas under investigation include:

```text
value or error
duration
instant
deferred or asynchronous work
input
output
```

Possible APIs such as `Result`, `Duration`, `Instant`, and `Task` are still design candidates.

### Everyday operations

Common intentions that may deserve a consistent Vix expression.

Current candidates include:

```text
read
write
parse
serialize
validate
run
print
log
```

These names are not yet commitments.

Each concept must first prove that it is general enough to belong in Standard.

## Public model

Standard is maintained as its own Vix module:

```text
vixcpp/standard
```

and is integrated into the main Vix repository under:

```text
modules/standard
```

Its CMake target is intended to be:

```cmake
vix::standard
```

General Standard concepts normally belong directly to the public Vix namespace:

```cpp
vix::Result<T, E>
vix::Duration
vix::Instant
```

Specialized domains remain explicit:

```cpp
vix::process::Command
vix::db::Database
vix::game::Scene
```

The physical module layout and the public C++ namespace intentionally serve different purposes.

## Design constraints

Standard should remain small even as Vix grows.

A feature does not belong in Standard merely because it is useful.

A candidate should represent a recurring semantic that developers would otherwise need to relearn across independent domains.

Standard should also preserve:

- precise and lightweight includes
- small transitive dependency graphs
- fast compilation
- predictable runtime cost
- clear ownership and lifetime semantics
- observable failure behavior
- natural C++ composition
- access to specialized APIs when more control is required

Standard must not become another `utils`, another `core`, a framework, or a flattened collection of aliases to every Vix module.

## Status

Vix Standard is currently under design.

The files and API names present in the repository are not yet stable API commitments.

Candidate concepts may be renamed, merged, moved, replaced, or removed as the design is tested against real C++ programs.

## Design documents

Contributor decisions should follow:

- [`DESIGN.md`](DESIGN.md), design principles and admission rules
- [`SURFACE.md`](SURFACE.md), repository, CMake, header, namespace, and public-surface rules

When a proposed feature does not clearly fit those rules, it should not be added to Standard yet.

## License

Vix Standard is licensed under the Apache License 2.0. See [`LICENSE`](LICENSE).
