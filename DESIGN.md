# Vix Standard Design

This document defines why Vix Standard exists, what it is allowed to become, and the design rules that should keep it coherent as Vix evolves.

It is primarily written for maintainers and contributors.

This document does not define the final API.

Names such as `Result`, `Task`, `Duration`, `read`, or `run` are design candidates until their semantics, composition rules, costs, and boundaries have been validated.

The rules in this document are more important than the first names or implementations chosen.

## 1. Why Standard exists

C++ can be used to build almost anything, but its generality comes with a large surface area.

A developer performing ordinary work may need to understand several parts of the standard library, operating system APIs, third-party libraries, and unrelated conventions before completing a simple task.

Reading a file, launching a process, representing a timeout, parsing a value, performing asynchronous work, or reporting an error can each introduce a different model.

Vix has accumulated the same kind of fragmentation as it has grown.

Its capabilities are currently distributed across modules such as:

```text
fs
path
env
io
process
time
async
threadpool
requests
json
conversion
validation
db
kv
cache
...
```

The existence of these modules is not itself a problem.

Many of them represent real domains that should remain separate.

The problem begins when a developer must understand the module graph before expressing an ordinary intention.

Standard exists to provide a small common experience before domain specialization becomes necessary.

A developer should be able to learn a few ideas once and continue using them while building:

```text
a library
a command-line program
an application
a service
a system tool
an engine
a game
a test
```

Standard should not decide which one they are building.

## 2. Start from the developer experience

Standard is not initially being designed to repair the architecture of existing Vix modules.

Existing modules are useful evidence.

They show repeated needs, duplicated abstractions, successful APIs, historical constraints, and places where several incompatible models have emerged.

They should inform the design.

They should not dictate it.

The primary question is:

> What should a developer learn once and then reuse naturally while building almost anything with Vix?

The design process should begin there.

Only after Standard proves itself through real usage should existing Vix modules progressively adopt it where doing so improves their design.

The intended direction is:

```text
developer experience
        ↓
Standard
        ↓
real usage
        ↓
stabilization
        ↓
progressive adoption inside Vix
```

Standard must earn its place as shared infrastructure.

It must not become shared infrastructure merely because it was declared to be so.

## 3. Standard is not a new language

Standard remains C++.

It must not create a pseudo-language through macros, hidden code generation, artificial syntax, or template machinery that makes ordinary C++ behavior difficult to understand.

Important C++ concepts remain important:

```text
types
ownership
RAII
lifetimes
value semantics
move semantics
templates
compilation
linking
```

Standard should reduce unnecessary friction without teaching developers an incorrect mental model of C++.

Its purpose is not to hide the language.

Its purpose is to reduce the distance between a common intention and a correct C++ expression of that intention.

## 4. Standard does not replace the C++ standard library

The C++ standard library remains foundational.

Standard must not recreate standard types merely to place them inside `namespace vix`.

Do not create Vix replacements for concepts such as:

```text
std::vector
std::string
std::optional
std::variant
std::filesystem::path
standard algorithms
standard ranges
```

unless Vix has a concrete semantic requirement that the standard facility cannot satisfy.

A Vix abstraction is justified when it provides one or more of the following:

- a missing common semantic
- a coherent model shared across independent domains
- behavior unavailable in the required C++ version
- a better composition boundary
- an important portability boundary
- a substantially clearer error or lifetime model

Convenience alone is not enough.

When the standard library already provides the correct abstraction, Standard should normally compose with it rather than replace it.

## 5. Standard is not a smaller version of Vix

Standard must not become:

```text
Vix, but easier
```

There should not be one Vix for beginners and another Vix for advanced users.

Standard is the common vocabulary.

Specialized modules provide domain-specific vocabulary and deeper control.

A developer may begin with Standard and later use:

```text
db
game
async
realtime
process
requests
```

when the problem requires those domains.

Specialization should extend what the developer already understands rather than invalidate it.

## 6. Modules organize capabilities, not the learning model

The Vix repository may contain many modules.

The initial mental model required from a developer should remain much smaller.

A user should not need to begin with questions such as:

```text
Is this in fs?
Is this in io?
Is this in path?
Is this in env?
Is this in process?
```

The first question should normally be about intent.

The Vix CLI already demonstrates part of this principle.

For example:

```text
vix run
```

expresses the intention to run something even though different implementations may be selected internally depending on the target.

The implementation structure can remain rich.

The user-facing grammar should remain smaller.

## 7. Standard has two conceptual responsibilities

Standard can currently be understood as having two conceptual areas.

This does not yet prescribe directories, headers, or namespaces.

### 7.1 Common forms

These are small concepts that can retain the same meaning across several independent domains.

Current areas under investigation include:

```text
value or error
duration
instant
deferred or asynchronous work
input
output
```

These may eventually produce public types such as:

```cpp
Result<T, E>
Duration
Instant
Task<T>
```

The names are not yet final.

Their semantics matter first.

### 7.2 Everyday operations

Some intentions appear frequently enough that Vix should investigate whether they deserve a coherent everyday expression.

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

Their presence here does not mean they must all become root-level free functions.

Each must first prove that it represents a sufficiently general and stable operation.

## 8. Common vocabulary must not destroy domain vocabulary

Unification is not valuable when it removes useful meaning.

A SQL API naturally speaks about:

```text
query
transaction
row
```

A key-value store naturally speaks about:

```text
key
get
set
erase
```

HTTP naturally speaks about:

```text
request
response
```

A process API naturally speaks about:

```text
command
stdin
stdout
exit status
```

Standard must not replace these terms with weaker universal abstractions merely to make APIs look similar.

The intended relationship is:

```text
common vocabulary
        +
domain vocabulary
```

not:

```text
one vocabulary forced onto every domain
```

## 9. Similar structure does not imply shared semantics

Two concepts can look similar without representing the same thing.

A filesystem path and a hierarchical key may both consist of segments.

That does not make them the same abstraction.

A cache and a key-value store may both expose `get` and `put`.

That does not give them the same semantics.

A coroutine task, a process, and a build graph node may all represent work.

That does not mean they should share one public type.

Standard should generalize only when the semantic overlap is real.

Structural resemblance is not enough.

## 10. A Standard primitive should enable multiple solutions

A primitive should remain small while being useful in independent situations.

A duration is a good example.

The same concept can participate in:

```text
HTTP timeout
cache TTL
retry delay
sleep
timer
periodic interval
process timeout
expiration
```

The domains may define additional concepts such as `Timeout`, but they should not need incompatible representations of time without a good reason.

This is one of the strongest admission tests for Standard.

A primitive should be reusable because its semantics are genuinely shared.

## 11. Failure should have a coherent grammar

Vix currently contains several ways to represent failure or absence, including forms similar to:

```text
Result<T>
Result<T, E>
expected<T, E>
KvResult<T>
FormResult<T>
exception
bool
nullptr
optional<T>
```

These are not equivalent.

Normal absence is not an operational error.

For example, `std::optional<T>` is appropriate when a value may legitimately not exist.

An operation that failed should normally preserve information about why it failed.

Standard should investigate a common success-or-error form that allows domains to preserve their own error types.

A conversion error may remain a conversion-specific error.

Validation may need to return several validation errors.

A database failure may need database-specific information.

The shared mechanism must not erase domain semantics.

## 12. Standard must not hide errors to appear simple

An API is not simple merely because it returns `void`.

It is not simple merely because every failure becomes an exception.

It is not simple merely because it returns `false` without context.

Standard should seek semantic simplicity rather than superficial brevity.

Important failures must remain observable.

Normal absence should not become an error.

Real errors should not silently become absence.

## 13. Standard must not become another `utils`

This is a critical rule.

Standard is not a destination for code that has no obvious home.

A useful helper is not automatically a Standard primitive.

A function used from several files is not automatically a Standard primitive.

Even an abstraction used by several modules is not automatically a Standard primitive.

Before adding something to Standard, contributors must be able to explain why a general Vix developer should learn that concept once and continue finding the same semantics across independent kinds of software.

If that explanation is weak, the feature probably belongs elsewhere.

## 14. Standard must not become another `core`

Standard does not coordinate all of Vix.

It does not own every application.

It does not own the network, databases, games, runtime, build system, or distributed systems.

Domain modules must remain able to evolve without turning Standard into a central controller.

The growth of Vix should not force proportional growth in Standard.

Adding ten new Vix modules should not automatically add ten new Standard concepts.

## 15. Standard is not a framework

Standard must not impose a universal application architecture.

It should not require concepts such as:

```text
App
Application
Service
Controller
Model
Scene
Engine
Runtime
```

as the structure through which all software must be expressed.

A standalone C++ library must be able to use Standard.

A CLI must be able to use Standard without becoming a server.

A game must be able to use Standard without adopting an HTTP application model.

Standard provides building blocks.

The software retains ownership of its architecture.

## 16. Standard must work across several kinds of software

Every candidate concept should be tested against several independent uses.

At minimum, contributors should ask whether the concept remains natural when building:

```text
a library
a simple executable
a command-line tool
a service
a system program
an engine
a test
```

An abstraction that only becomes natural inside one family of software probably belongs to a specialized domain.

Standard must not privilege HTTP applications because of Vix history.

It must not privilege games, cloud systems, command-line programs, or any other single domain.

## 17. Performance is part of the design

Vix values runtime performance and fast development cycles.

Standard must preserve both.

A simple Standard feature must not accidentally introduce a large transitive dependency graph.

A small source file should not become expensive to compile merely because it uses one Standard type.

Compilation cost is a design constraint.

It should be measured alongside runtime cost.

## 18. One conceptual surface does not require one large header

Standard may be one conceptual product while remaining physically decomposed.

A developer using only one primitive should not need to parse unrelated networking, process, JSON, logging, or asynchronous headers.

Standard should support:

```text
one coherent vocabulary
+
small precise includes
```

These goals are compatible.

A large umbrella header must never be required to access Standard.

## 19. Transitive dependencies must remain small

Each Standard component should depend only on what it actually needs.

The most fundamental primitives should aim for extremely small dependency graphs.

A feature that requires a specialized Vix module internally must not force unrelated Standard users to pay for that dependency.

Ease of use must not be purchased through uncontrolled transitive dependencies.

## 20. Standard must not duplicate existing modules

If Vix already contains a specialized implementation, Standard should not copy it merely to expose a shorter API.

Avoid architectures such as:

```text
modules/fs
standard/filesystem
```

or:

```text
modules/process
standard/process2
```

when both sides implement the same capability independently.

Standard may compose an existing capability or provide a carefully designed everyday surface over it.

It should not solve fragmentation by introducing another implementation.

## 21. Existing modules may adopt Standard progressively

Standard is initially designed for developers.

Existing Vix modules may later adopt it.

That adoption should follow several rules.

First, a module should not be rewritten merely to use Standard.

Second, low-level modules may continue to use the C++ standard library, operating system APIs, or specialized dependencies directly when that is the correct implementation boundary.

Third, when a module is independently recreating a primitive that Standard has already stabilized, migration becomes worth evaluating.

Standard should spread because it makes code clearer and more coherent, not because every module has been ordered to depend on it.

## 22. Existing Vix code is evidence, not a contract

Current Vix modules reveal recurring needs.

They show several generations of APIs.

They show duplicated result types.

They show multiple representations of time.

They show useful façade patterns.

They also contain historical choices that may no longer represent the best direction.

Standard is not required to preserve every historical abstraction.

Compatibility and ideal Standard design are separate questions.

## 23. Standard should remain small for years

Every addition must be evaluated for its long-term effect.

The question is not:

> Is this useful?

The stronger question is:

> Is this fundamental enough that its absence causes developers to relearn the same semantic problem across independent domains?

A mature Standard may remain surprisingly small even while Vix becomes significantly larger.

Size is not a measure of success.

## 24. Adding something to Standard should be difficult

Before adding a new primitive or everyday operation, contributors should answer:

1. What real everyday problem does this solve?
2. Does the problem appear in several kinds of software?
3. Should a developer learn this concept once?
4. Does the C++ standard library already solve it adequately?
5. Does it actually belong to a specialized domain?
6. Can it compose with other Standard concepts without introducing a global architecture?
7. What dependencies does it introduce?
8. What is its compilation cost?
9. What is its runtime cost?
10. Can its semantics remain stable for several years?
11. Does its absence cause multiple independent parts of Vix to recreate the same model?
12. Is this truly a primitive or merely a convenient feature?

Weak answers to several of these questions are sufficient reason not to add the concept.

## 25. Removing an abstraction must remain possible

Standard should not accumulate abstractions simply because they were once implemented.

Before a concept becomes stable, it should be removable if real usage shows that it is unnecessary, too specialized, or difficult to compose.

A primitive that constantly requires exceptions, special cases, or domain-specific adaptation is probably not fundamental enough.

Four strong concepts are better than fifteen mediocre ones.

## 26. Standard abstractions should be understandable locally

A contributor opening a Standard header should be able to understand its responsibility without reading several unrelated modules.

A primitive should have:

- clear semantics
- explicit invariants
- understandable ownership
- understandable failure behavior
- predictable lifetime behavior
- limited hidden state

The implementation should reflect the small conceptual model.

## 27. Avoid hidden global machinery

Standard should avoid requiring:

```text
hidden global initialization
mandatory singletons
implicitly started threads
required global configuration
invisible shared mutable state
fragile initialization order
```

An ordinary operation should remain easy to reason about.

When a runtime is genuinely required, its existence should be visible in the model.

A locally simple-looking API must not secretly depend on a large global system without a strong reason.

## 28. Cost should be proportional to use

A program using a small part of Standard should not pay for the rest.

This applies to:

```text
compile time
binary size
allocations
threads
initialization
dependencies
runtime
```

Unused capabilities should ideally have no cost or the smallest practical cost.

API simplicity does not justify permanent hidden overhead.

## 29. Avoid unnecessary allocation

Standard should preserve Vix's performance discipline.

This does not mean that allocation is forbidden.

It means an abstraction should not allocate merely because doing so makes implementation easier.

Common paths should be examined carefully.

Unnecessary copies should be avoided.

Ownership should remain explicit.

Public abstractions should not prevent important optimizations when those optimizations become necessary.

## 30. Do not use runtime polymorphism by default

A general abstraction does not automatically need a virtual class hierarchy.

Runtime polymorphism should be used when the problem actually requires it.

Depending on the semantics, C++ may provide better tools through:

```text
values
templates
concepts
free functions
type erasure
variants
static polymorphism
```

Standard should use C++ naturally rather than reproduce the object model of another language.

## 31. Composition matters more than feature count

A good primitive should work well with other primitives.

For example, an eventual workflow may conceptually resemble:

```text
read
 ↓
parse
 ↓
validate
 ↓
transform
 ↓
serialize
 ↓
write
```

Likewise, concepts such as:

```text
Task<T>
Duration
Result<T, E>
```

should be able to participate in larger systems without requiring a new abstraction for every combination.

Standard should optimize for composition, not for the number of functions advertised.

## 32. Everyday APIs should be obvious without becoming artificial

Short names are useful only when their semantics remain clear.

Standard must not automatically produce APIs such as:

```cpp
vix::read(...)
vix::write(...)
vix::run(...)
```

only because they look elegant.

A free function, type, method, customization point, or domain object should be chosen according to semantics.

The preferred form is the one a developer can understand without knowing the implementation architecture while still preserving important domain distinctions.

## 33. Documentation should begin with use

A Standard user should not need to study its internal architecture before using it.

Documentation should begin with the developer's workflow and the public semantic model.

Implementation details such as:

```text
backend
scheduler
executor
OS adapter
internal storage
type erasure
```

should appear only when they affect public behavior or when a contributor intentionally studies the implementation.

The internal architecture is not the tutorial.

## 34. Standard expertise is not API memorization

Knowing Standard should not mean memorizing a long list of functions.

A developer who understands Standard deeply should understand:

```text
the small common forms
their semantics
their composition
their costs
their boundaries
when specialization becomes necessary
```

That knowledge should remain useful as Vix gains new domains.

## 35. Vix expertise does not require knowing every module

Vix may continue growing.

A Vix expert should not be defined as someone who has memorized every module.

They should understand the common vocabulary, its composition rules, and the boundaries between general and specialized concepts.

They can then become deeply familiar with Vix Game, Vix Database, Vix Realtime, or another domain without learning everything else.

## 36. Standard must not promise that C++ becomes easy

C++ remains a deep language.

Standard must not pretend that lifetime, ownership, concurrency, compilation, linking, or performance concerns have disappeared.

The goal is more precise:

> Avoid forcing developers to learn several unrelated models for the same recurring needs.

That goal is already ambitious enough.

## 37. Validate abstractions with real programs

A concept should not become fundamental because it looks elegant in isolation.

Standard abstractions should be exercised in real small programs before stabilization.

At minimum, test them in:

```text
a small library
a command-line program
a simple executable
a service
tests
```

When relevant, measure:

```text
clean compile time
incremental compile time
binary size
allocations
runtime cost
transitive dependencies
```

An abstraction that is elegant on paper but materially damages ordinary build performance should be reconsidered.

## 38. The first example should remain small

A useful test for a Standard abstraction is the size of its first meaningful example.

An everyday operation should not require several configuration objects before its basic behavior can be demonstrated.

If the first example requires explaining a backend, runtime, factory, context, and scheduler, the abstraction is probably not ready for Standard.

## 39. Complexity should appear progressively

Standard does not eliminate advanced requirements.

It should delay them until they are actually needed.

A developer should begin with the ordinary path.

More control may progressively expose concepts such as:

```text
configuration
policy
allocator
scheduler
backend
platform-specific behavior
```

Advanced control must not make the common path difficult.

## 40. Names are not sacred before semantics

Current names are hypotheses.

Names such as:

```text
Result
Task
Duration
Instant
Input
Output
read
write
parse
serialize
validate
run
print
log
```

may all change during design.

Before stabilizing a name, stabilize:

```text
what the concept means
what it guarantees
what it does not represent
how it composes
what it costs
where its boundary ends
```

An API should not survive merely because its name was chosen early.

## 41. Standard must have an identifiable boundary

At any point, contributors should be able to answer:

> Why does this belong in Standard instead of a specialized module?

A strong answer looks like:

> This concept appears in several independent domains, keeps the same semantics, and should be learned once by a general Vix developer.

A weak answer looks like:

> It is convenient and many programs might need it.

Many useful features are not fundamental.

## 42. Standard must resist its own success

If Standard becomes widely used, pressure to add more features will increase.

Every domain may eventually want direct exposure through Standard.

That growth must not be automatic.

A specialized module can be excellent without appearing in Standard.

Standard succeeds when it remains useful while Vix grows around it.

## 43. Current areas under investigation

The study of existing Vix modules currently suggests several areas worth deeper design work:

```text
success and failure
time
deferred and asynchronous work
input and output
conversion between representations
small everyday operations
```

These are investigation areas, not a frozen feature list.

No header should be considered justified merely because its current filename already exists in the repository.

Each concept must still pass the design process described in this document.

## 44. Reference question

When a design decision becomes unclear, return to this question:

> What should a developer learn once and then reuse naturally while building almost anything with Vix?

If an abstraction genuinely answers that question, it may belong in Standard.

If it does not, it probably belongs somewhere else.

## 45. Final principle

Standard must not contain every common solution.

It should provide the small set of concepts from which many solutions can be built without forcing developers to constantly change mental models.

Vix may become much larger.

Standard should be able to remain small.
