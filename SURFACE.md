# Vix Standard Surface

This document defines how Vix Standard is represented across the repository, build system, public headers, namespaces, and user-facing API.

`DESIGN.md` defines what Standard is allowed to become.

This document defines how Standard should appear to developers.

The main rule is simple:

> Standard may be implemented as a Vix module without becoming a specialized module in the developer's mental model.

The repository structure, CMake target structure, header layout, and C++ namespace structure serve different purposes and do not need to mirror each other exactly.

## 1. Physical structure and public structure are different

Vix Standard is maintained as its own repository:

```text
vixcpp/standard
```

Inside the main Vix repository, it may live at:

```text
modules/standard
```

This is an implementation and repository organization decision.

It does not mean the public API should be represented as:

```cpp
vix::standard::Result
vix::standard::Duration
vix::standard::Task
```

Standard represents the general vocabulary of Vix.

A better conceptual model is:

```text
Vix
├── Result
├── Duration
├── Instant
├── Task
├── ...
│
├── process
├── db
├── requests
├── game
├── realtime
└── ...
```

The root contains the small general vocabulary.

Specialized domains remain explicitly namespaced.

## 2. Repository identity

The repository is:

```text
https://github.com/vixcpp/standard
```

The repository name describes the component being maintained.

It does not dictate the final public namespace.

The repository may contain:

```text
include/
src/
tests/
examples/
benchmarks/
cmake/
```

like any other independently maintained Vix component.

## 3. Main Vix repository location

Inside the Vix umbrella repository, Standard is represented as:

```text
modules/standard
```

This keeps the existing repository organization coherent.

The `modules` directory describes how Vix source components are assembled.

It does not define which concepts are equally important to users.

For example:

```text
modules/standard
modules/process
modules/db
modules/game
```

may exist beside each other physically while having different conceptual roles.

Standard is the general surface.

The others represent specialized capabilities.

## 4. CMake target

The standalone CMake target for Standard should be:

```cmake
vix::standard
```

A project that only needs Standard should eventually be able to express that dependency directly:

```cmake
target_link_libraries(my_library
    PRIVATE
        vix::standard
)
```

This must not implicitly require the complete Vix SDK.

The target should remain usable by:

```text
libraries
executables
CLI tools
tests
services
other Vix modules
```

without assuming a particular application architecture.

## 5. `vix::standard` and `vix::vix` have different roles

The Standard target must not replace the Vix umbrella target.

Conceptually:

```text
vix::standard
    general Standard surface

vix::<domain>
    specialized capability

vix::vix
    umbrella Vix SDK target
```

For example:

```cmake
target_link_libraries(tool PRIVATE vix::standard)
```

means the program needs the general Standard surface.

```cmake
target_link_libraries(tool
    PRIVATE
        vix::standard
        vix::process
)
```

means the program also requires process-specific functionality.

```cmake
target_link_libraries(application PRIVATE vix::vix)
```

may represent a broader Vix SDK configuration.

These dependency choices should remain distinguishable.

## 6. Standard is not normally exposed as `vix::standard::*`

The public Standard vocabulary should normally live directly inside:

```cpp
namespace vix
```

rather than:

```cpp
namespace vix::standard
```

For example, if the concepts are eventually accepted:

```cpp
vix::Result<T, E>
vix::Duration
vix::Instant
vix::Task<T>
```

are preferred over:

```cpp
vix::standard::Result<T, E>
vix::standard::Duration
vix::standard::Instant
vix::standard::Task<T>
```

The second form treats Standard as one specialized domain among many.

That is not its intended role.

## 7. The root `vix` namespace represents general vocabulary

A symbol directly inside:

```cpp
namespace vix
```

should communicate:

> This concept belongs to the general Vix vocabulary.

A symbol inside:

```cpp
namespace vix::process
```

should communicate:

> This concept belongs to the process domain.

A symbol inside:

```cpp
namespace vix::db
```

should communicate:

> This concept belongs to the database domain.

This distinction should remain visible throughout the public API.

## 8. Specialized domains keep specialized namespaces

Standard must not flatten the rest of Vix.

Domain-specific APIs should continue to use natural namespaces.

Examples:

```cpp
vix::process::Command
vix::process::Process

vix::db::Database
vix::db::Row

vix::requests::Request
vix::requests::Response

vix::game::Scene
vix::game::GameRuntime
```

Standard should not introduce root aliases such as:

```cpp
vix::Command
vix::Database
vix::Request
vix::Scene
```

unless those names later acquire genuinely domain-independent semantics.

Shorter syntax alone is not sufficient justification.

## 9. The root namespace is a limited resource

Adding a public symbol directly to `namespace vix` should be difficult.

The root namespace must not grow in proportion to the number of Vix modules.

If Vix grows from:

```text
20 modules
```

to:

```text
100 modules
```

the Standard vocabulary should still be able to remain small.

The root namespace must not become a catalog of every useful Vix type.

## 10. Root aliases do not create general concepts

An alias such as:

```cpp
namespace vix
{
    using Database = db::Database;
}
```

does not make `Database` a general Vix primitive.

Likewise:

```cpp
namespace vix
{
    using requests::get;
}
```

does not automatically make HTTP `get` part of Standard.

Standard should not use aliases as a shortcut for flattening domain APIs.

The namespace boundary must remain semantic.

## 11. Public headers should represent concepts, not repository layout

The physical location:

```text
modules/standard
```

does not require public headers to use:

```cpp
#include <vix/standard/Result.hpp>
```

For a general Vix concept, the preferred direction is:

```cpp
#include <vix/Result.hpp>
#include <vix/Duration.hpp>
#include <vix/Instant.hpp>
#include <vix/Task.hpp>
```

This allows the public header structure to match the developer's conceptual model.

Internally, those headers may still be provided by the `standard` repository.

## 12. Specialized headers remain explicit

Domain headers should continue to expose their specialization.

For example:

```cpp
#include <vix/process/Command.hpp>
#include <vix/db/Database.hpp>
#include <vix/game/Scene.hpp>
```

This creates a useful visual distinction.

```cpp
#include <vix/Duration.hpp>
```

means the program is using a general Vix concept.

```cpp
#include <vix/process/Command.hpp>
```

means the program is entering the process domain.

The include structure should help developers understand the architecture rather than merely reflect source repository boundaries.

## 13. Public header ownership

A possible Standard repository structure is:

```text
standard/
├── include/
│   └── vix/
│       ├── Result.hpp
│       ├── Duration.hpp
│       ├── Instant.hpp
│       └── Task.hpp
└── ...
```

The installed result may therefore expose:

```text
<vix/Result.hpp>
<vix/Duration.hpp>
<vix/Instant.hpp>
<vix/Task.hpp>
```

without exposing `standard` in the include path.

This is intentional.

## 14. No mandatory umbrella header

Standard must not require developers to include a large umbrella header.

This must remain valid as a design goal:

```cpp
#include <vix/Duration.hpp>
```

without requiring:

```cpp
#include <vix.hpp>
```

or:

```cpp
#include <vix/standard.hpp>
```

A developer using one small concept should only pay for the dependencies needed by that concept.

## 15. `vix.hpp` is not the definition of Standard

The existing:

```cpp
#include <vix.hpp>
```

must not automatically become the public representation of Standard.

Its historical role, transitive dependencies, and compile-time cost are separate concerns.

Standard must be independently usable without relying on the umbrella header.

A future umbrella strategy may be reconsidered only after its compile-time cost is measured and its role is clearly defined.

## 16. `vix/standard.hpp` is not required

A header such as:

```cpp
#include <vix/standard.hpp>
```

may be considered later, but it is not fundamental to Standard.

If such a header ever exists, it must not become the only supported entry point.

Precise includes remain first-class.

Any umbrella header must have a clearly measured dependency and compilation cost.

## 17. One conceptual product can have many small headers

Standard being one product does not imply one translation unit or one include file.

The intended relationship is:

```text
one mental model
+
small independent components
```

not:

```text
one mental model
=
one huge header
```

The public organization should preserve compile-time isolation.

## 18. Header independence is a design requirement

A public Standard header should include only what it needs.

Where practical, this should compile:

```cpp
#include <vix/Duration.hpp>

int main()
{
}
```

without depending on unrelated Standard concepts.

Likewise:

```cpp
#include <vix/Result.hpp>
```

should not require process, networking, JSON, logging, databases, or asynchronous runtimes unless the semantics genuinely require them.

## 19. Standard types must work outside Vix applications

General Standard types must not require the presence of:

```text
vix::App
HTTP server
GameRuntime
Engine
global runtime
Vix CLI
```

A normal C++ library should be able to use Standard directly.

For example:

```cpp
#include <vix/Result.hpp>

namespace image
{
    vix::Result<Image, LoadError> load(...);
}
```

This code should not need to become a Vix application.

## 20. Standard operations may compose specialized implementations

An everyday Standard operation may eventually use a specialized Vix capability internally.

For example, a future:

```cpp
vix::run(...)
```

could potentially compose process functionality.

That does not mean all process concepts become Standard concepts.

The relationship can be:

```text
simple intention
      ↓
Standard surface
      ↓
specialized implementation
```

while advanced users continue to access:

```cpp
vix::process::Command
```

directly.

## 21. Standard and domain APIs may coexist

A simple surface should not replace advanced control.

Conceptually:

```cpp
auto result = vix::run(...);
```

could represent an ordinary workflow.

A developer requiring process-specific control may instead use:

```cpp
vix::process::Command command;
```

The second API does not make the first unnecessary.

The first does not make the second obsolete.

They operate at different levels of specialization.

## 22. Moving from Standard to a domain should add knowledge

When a developer begins with Standard and later needs more control, specialization should extend the existing model.

The intended progression is:

```text
common operation
      ↓
need more control
      ↓
specialized domain API
```

not:

```text
common operation
      ↓
discard everything learned
      ↓
learn an unrelated model
```

Standard should make specialization easier to enter.

## 23. General forms and domain errors can coexist

Suppose Standard eventually provides:

```cpp
vix::Result<T, E>
```

A domain may still define:

```cpp
vix::process::Error
vix::conversion::Error
vix::db::Error
```

and use them as:

```cpp
vix::Result<Value, vix::conversion::Error>
```

or:

```cpp
vix::Result<RowSet, vix::db::Error>
```

The general form belongs to Standard.

The error semantics belong to the domain.

Standard should provide structure without erasing meaning.

## 24. General time representation and domain concepts can coexist

If Standard eventually defines:

```cpp
vix::Duration
```

a specialized API may still expose:

```cpp
vix::requests::Timeout
```

when `Timeout` represents additional HTTP-specific semantics.

The specialized concept may internally contain or compose a `vix::Duration`.

Standard should eliminate unnecessary representation fragmentation without eliminating useful domain concepts.

## 25. Internal namespaces are not public surface

Implementation details should not be exposed merely because Standard itself has a general role.

Internal implementation may use an explicit internal namespace such as:

```cpp
namespace vix::detail
```

when necessary.

Such namespaces are not part of the supported public surface.

They must not become a convenient dumping ground.

If a concept is public, it must have a deliberate public location.

If it is internal, that boundary should remain obvious.

## 26. Avoid `vix::standard::detail`

The hierarchy:

```cpp
vix::standard::detail
```

should not be introduced automatically.

The public vocabulary does not need a `standard` namespace merely to provide internal organization.

Internal names should follow the smallest clear implementation boundary required by the code.

## 27. Include paths must not reveal internal dependencies unnecessarily

A public Standard header may internally compose another component.

That does not mean users should need to include internal dependency headers themselves.

For example, a public API should not require a user to know:

```text
which backend implements it
which scheduler is used
which operating system adapter is selected
which private helper stores the state
```

The public include should correspond to the concept being used.

## 28. CMake dependencies must match actual needs

The `vix::standard` target should not link every potential backend or module preemptively.

Dependencies should be added only when required by the implemented Standard surface.

The dependency graph should remain inspectable.

A contributor adding a new dependency to Standard should be able to explain:

```text
why it is required
which Standard capability uses it
whether it is public or private
whether it affects consumers
whether it affects compile time
whether it affects binary size
```

## 29. Prefer private implementation dependencies

Where implementation permits it, specialized dependencies should remain private to Standard.

A dependency should become part of the public interface only when Standard's public API actually requires its types, definitions, or semantics.

This helps preserve a small consumer surface.

## 30. Header-only is not automatically preferable

A Standard component should not become header-only merely because header-only code is easy to distribute.

Header-only designs can increase:

```text
compile time
template instantiation
binary duplication
implementation exposure
dependency propagation
```

Small value types and templates may naturally live in headers.

Non-template implementation should be allowed to live in `.cpp` files when that produces a cleaner and cheaper public surface.

## 31. Compiled code is not automatically preferable either

Moving everything into `.cpp` files is also not a goal.

The correct representation depends on:

```text
semantics
ABI requirements
optimization
template requirements
compile-time cost
binary cost
distribution needs
```

Standard should measure rather than follow a blanket header-only or compiled-library rule.

## 32. Public names should be difficult to add

Before adding:

```cpp
vix::Something
```

contributors should answer:

1. Is `Something` independent of a specialized domain?
2. Does it retain the same semantics across several kinds of software?
3. Should a developer learn it once?
4. Does the C++ standard library already provide the right concept?
5. Will this name remain meaningful if Vix gains many more modules?
6. Can it be used outside a Vix application architecture?
7. Is it a real concept rather than an alias for a specialized API?
8. Does it justify occupying the root namespace?

If the answer is uncertain, the symbol should not enter the root namespace yet.

## 33. Public header names should be difficult to add

Adding:

```text
include/vix/Something.hpp
```

has the same conceptual weight as adding:

```cpp
vix::Something
```

The root include directory is also a limited public surface.

It must not become a flat mirror of every feature implemented across Vix.

Specialized features belong under specialized include paths.

## 34. Root free functions require the same discipline

Functions such as:

```cpp
vix::read(...)
vix::write(...)
vix::parse(...)
vix::run(...)
vix::print(...)
```

may be useful, but short names do not automatically make them Standard operations.

Each one must have semantics that remain understandable at root scope.

For every candidate root function, contributors should ask:

```text
What does it operate on?
What does it return?
How does failure work?
Can overloads remain understandable?
Does another domain already own this verb?
Can ADL or overload resolution become ambiguous?
Does the name remain meaningful without knowing the implementation?
```

A root function should be rejected if its generality creates ambiguity.

## 35. Standard should avoid a giant root overload set

A design where every module contributes another overload of:

```cpp
vix::read(...)
```

or:

```cpp
vix::write(...)
```

could eventually become difficult to reason about.

Standard must not turn the root namespace into an uncontrolled customization system.

If a verb requires extensibility, the extension mechanism must be intentionally designed and constrained.

The existence of a common English verb is not enough to justify global overloads.

## 36. Extension mechanisms must be deliberate

If Standard eventually needs extensible operations, contributors must decide explicitly whether they use:

```text
member functions
free functions
templates
C++ concepts
customization point objects
tag_invoke-like mechanisms
ADL
traits
runtime interfaces
```

No extension mechanism should emerge accidentally from overload accumulation.

The mechanism must preserve compile-time performance, diagnostics, and understandable semantics.

## 37. C++ namespace and CMake namespace mean different things

The syntax:

```text
vix::standard
```

in CMake and:

```cpp
vix::Result
```

in C++ are not inconsistent.

They exist in different systems.

CMake uses `vix::standard` to identify a build dependency.

C++ uses `vix::Result` to express that `Result` belongs to the general Vix vocabulary.

Trying to force these representations into the same hierarchy would optimize for visual symmetry rather than developer understanding.

## 38. Repository path and include path mean different things

Likewise:

```text
modules/standard
```

and:

```cpp
#include <vix/Result.hpp>
```

serve different purposes.

The first tells contributors where the implementation repository is mounted.

The second tells developers which public concept they are using.

The implementation structure should not leak into the public model without a reason.

## 39. Surface stability matters more than internal stability

Standard internals may evolve significantly while the public semantic surface remains stable.

Implementation details such as:

```text
storage layout
backend selection
small-object optimization
scheduler implementation
OS abstraction
private helper types
```

should remain replaceable where possible.

Public names, semantics, ownership rules, and failure behavior require much stronger stability.

## 40. A public concept should have one canonical representation

Standard should avoid exposing multiple equivalent public paths such as:

```cpp
vix::Duration
vix::time::Duration
vix::standard::Duration
vix::utils::Duration
```

as equally recommended APIs.

Compatibility aliases may temporarily exist during migration, but documentation must identify one canonical representation.

Long-term duplication at the public surface recreates fragmentation.

## 41. Compatibility must not define the new surface

Existing Vix modules may already expose types with names Standard later adopts.

This should be handled deliberately.

A historical type should not automatically become the canonical Standard type merely because it already exists.

Likewise, Standard should not create permanent duplicate types merely to avoid migration work.

Compatibility strategy and ideal surface design must be treated as separate decisions.

## 42. Standard versioning belongs to its repository

The standalone Standard repository should remain independently testable and versionable as a Vix component.

Its integration into Vix releases may follow the umbrella release process, but its implementation should not require modifications to unrelated modules for every change.

This supports isolated development and testing.

## 43. Standard must be usable standalone

The Standard repository should eventually support a standalone workflow such as:

```text
clone
configure
build
test
install
find_package
```

without requiring the complete Vix source tree unless a specific capability genuinely depends on it.

The exact packaging implementation may evolve.

Standalone usability is part of keeping Standard a real general component rather than an umbrella-only convenience layer.

## 44. Standalone installation must preserve public names

Whether Standard is consumed from:

```text
the Vix umbrella repository
a standalone installation
a package manager
a source dependency
```

the public C++ names should remain the same.

A developer should not write different C++ code depending on how Standard was obtained.

## 45. Standard should not expose Vix repository mechanics

Public code should not depend on knowledge of:

```text
git submodules
umbrella build directories
module checkout layout
Vix repository internals
```

Those are development and distribution concerns.

The installed API should look like a normal C++ library.

## 46. Tests should verify the surface

Standard tests should not only verify runtime behavior.

They should also verify public-surface properties where practical.

Examples include:

```text
individual headers compile independently
public headers do not require unrelated modules
standalone consumers can link vix::standard
public types are available in intended namespaces
internal headers are not required by examples
```

Compile tests are part of API testing.

## 47. Benchmarks should include compile-time surface cost

Standard benchmarks should eventually measure not only runtime behavior but also the cost of including and using its public components.

Representative measurements may include:

```text
empty translation unit
include one Standard header
instantiate one common type
use one common operation
incremental rebuild after implementation change
incremental rebuild after consumer change
```

The goal is not to chase meaningless microseconds.

The goal is to detect architectural regressions before they become permanent.

## 48. Examples must use the intended public surface

Examples inside the Standard repository should not use private shortcuts unavailable to external users.

They should include public headers and link the public target.

If an example needs internal headers to remain simple, the public surface is probably incomplete or the example is testing the wrong level.

## 49. A library is the first important surface test

Standard must work naturally inside a normal C++ library.

For example:

```cpp
#include <vix/Result.hpp>

namespace parser
{
    vix::Result<Document, ParseError> parse_document(...);
}
```

This use case is important because it prevents Standard from drifting toward an application framework.

A library has no reason to adopt an application runtime merely to use a general primitive.

## 50. CLI programs are another important surface test

A command-line program should be able to combine Standard concepts without adopting unrelated application architecture.

Conceptually:

```cpp
#include <vix/Result.hpp>
#include <vix/Duration.hpp>
```

should remain sufficient when those are the only required concepts.

Adding CLI behavior later should introduce only the APIs actually needed for that behavior.

## 51. Services must not define the general surface

Standard should also work well for long-running services, but service-specific needs must not become universal merely because they are common in backend software.

Concepts such as:

```text
server
request context
service container
dependency injection
application lifecycle
```

must remain outside Standard unless they independently satisfy the general admission rules.

## 52. Surface decisions must survive domain changes

A useful question for every public Standard symbol is:

> Would this still make sense if Vix stopped being used for HTTP software tomorrow?

Likewise:

> Would it still make sense if Vix became heavily used for games, compilers, embedded tools, desktop software, or libraries?

If the answer depends strongly on one domain, the symbol probably does not belong at the general surface.

## 53. Current representation

The current intended representation is:

```text
Git repository
    vixcpp/standard

Vix source tree
    modules/standard

CMake target
    vix::standard

Public headers
    <vix/...>

Public C++ namespace
    vix::...

Specialized public namespaces
    vix::<domain>::...
```

For example, if the current candidates survive design:

```cpp
#include <vix/Result.hpp>
#include <vix/Duration.hpp>
#include <vix/process/Command.hpp>

vix::Result<int, Error> result;
vix::Duration timeout;

vix::process::Command command;
```

This representation is intentional.

The physical repository structure and the public C++ structure do not need to be identical.

## 54. Current candidate headers are not yet API commitments

The repository currently contains candidate files such as:

```text
Result.hpp
Duration.hpp
Instant.hpp
Task.hpp
Input.hpp
Output.hpp
Read.hpp
Write.hpp
Parse.hpp
Serialize.hpp
Validate.hpp
Run.hpp
Print.hpp
Log.hpp
```

Their existence is not proof that all of them belong in Standard.

Each file is a design hypothesis.

A candidate may be:

```text
renamed
merged
split
moved to another module
replaced with a standard C++ facility
or removed entirely
```

before the Standard API is stabilized.

The architecture must follow semantics, not placeholder filenames.

## 55. Canonical surface rule

For every accepted Standard concept, there should eventually be one canonical answer to each of these questions:

```text
Which repository owns it?
Which CMake target provides it?
Which header exposes it?
Which namespace contains it?
What are its semantics?
What does it depend on?
What is the advanced path when more control is needed?
```

If several competing answers remain indefinitely, Standard has recreated the fragmentation it was designed to reduce.

## 56. Review questions for public surface changes

Before accepting a public-surface change, reviewers should ask:

1. Does this belong to the general Vix vocabulary?
2. Is this a domain concept disguised as a convenience API?
3. Is the namespace correct?
4. Is the header path correct?
5. Does the include pull unnecessary dependencies?
6. Does the CMake target expose unnecessary dependencies?
7. Does this work in a standalone library?
8. Does this require a global runtime unexpectedly?
9. Does the C++ standard library already provide the right surface?
10. Is there now more than one canonical Vix representation for the same concept?
11. Does the advanced domain API remain accessible?
12. Does the new surface remain understandable without knowing repository internals?

If these questions reveal ambiguity, the surface is not ready.

## 57. Final rule

Standard may be a module in the architecture of Vix.

It should not feel like one more module that developers must discover before they can use Vix.

Its build identity is:

```text
vix::standard
```

Its general public vocabulary normally belongs directly to:

```cpp
vix::
```

Specialized domains remain under:

```cpp
vix::<domain>::
```

The repository structure organizes Vix for maintainers.

The public surface organizes Vix for developers.
