# AGENTS.md

## Project Goal

CapnZero is an IDL-driven RPC/signaling code generator.

The current implementation generates C++ client/server code and has accumulated
technical debt in its Python generator implementation.

The immediate goal is to refactor the generator into a clean architecture that
can support multiple target languages without changing existing C++ behavior.

The next major feature is Python code generation for both clients and servers.

The intended architecture is approximately:

    CapnZero IDL
         |
         v
    parse / validate
         |
         v
    language-neutral model / IR
         |
         +--------------------+
         |                    |
         v                    v
    C++ generation       Python generation
      |       |            |        |
    client   server       client    server

The parser and semantic model must not encode C++-specific concepts unless those
concepts are genuinely part of the CapnZero protocol.

C++ and Python generation should consume the same semantic representation.


## Primary Refactoring Rule

Preserve existing behavior before improving it.

The current C++ generator is the compatibility baseline.

During structural refactoring:

- Do not intentionally change generated C++ output.
- Do not intentionally change protocol semantics.
- Do not combine large behavioral changes with structural refactoring.
- Existing tests must continue to pass.
- Prefer small, independently verifiable changes.
- Run tests frequently.

Where practical, generated C++ output should remain byte-for-byte identical
during the refactoring phase.

If preserving exact output is impractical for a particular change, explain why
before making the change and verify that the generated code is semantically and
wire-compatible.


## Before Refactoring

Before making substantial changes:

1. Study the repository.
2. Understand the existing generator pipeline.
3. Run the complete existing test suite.
4. Establish whether the repository currently has a clean test baseline.
5. Identify what behavior the tests protect.
6. Identify important generator behavior that is currently unprotected.
7. Identify C++ assumptions embedded in parsing or intermediate structures.
8. Propose an incremental refactoring plan.

Do not begin with a large rewrite.

If important existing behavior is insufficiently tested, add characterization
tests before restructuring the corresponding code.


## Tests Are the Safety Net

Treat the existing test suite as authoritative evidence of current intended
behavior unless repository documentation clearly says otherwise.

For generator behavior, prefer tests that exercise complete IDL examples and
inspect generated output over tests that are tightly coupled to generator
implementation details.

Where appropriate, add golden/characterization tests:

    input IDL
        |
        v
    generator
        |
        v
    generated files
        |
        v
    compare with expected output

Representative test IDLs should eventually cover at least the protocol features
supported by CapnZero, such as:

- interfaces/services
- RPC methods
- parameters
- return values
- signals
- properties, if supported
- structs
- enums
- optional values, if supported
- collections, if supported
- namespaces/modules
- edge cases already supported by the existing generator

Do not invent new IDL semantics merely to make the architecture prettier.


## Desired Internal Architecture

Aim for three conceptually separate stages:

### 1. Parsing

Responsible for turning source IDL into syntactic structures.

Parsing should answer:

    "What did the source file say?"

It should not contain target-language rendering logic.


### 2. Semantic Validation and IR

Responsible for turning parsed input into a validated, language-neutral
representation of the CapnZero API.

This layer should answer:

    "What API/protocol does this IDL describe?"

Examples of concepts that belong here include:

- interface
- method/RPC
- signal
- property
- parameter
- return value
- struct
- enum
- protocol type
- optionality
- collection/container shape
- names and documentation

The exact model should emerge from the existing IDL rather than being invented
up front.

Do not put target-language spellings into the IR.

For example, avoid IR fields such as:

    cpp_type = "std::optional<std::string>"

Prefer semantic information such as:

    Optional(String)

and let each language backend decide how that is represented.


### 3. Language Generation

Each target language maps the common IR to its own source representation.

For example:

    Optional(String)

might become:

    C++:
        std::optional<std::string>

    Python:
        str | None

Language-specific concerns belong here:

- imports/includes
- namespaces/packages/modules
- language type names
- identifier escaping
- formatting
- source filenames
- client implementation details
- server implementation details
- runtime-library integration

Do not contaminate the common semantic model with these concerns.


## Do Not Over-Abstract

Do not introduce an abstraction merely because another implementation might
exist someday.

The Python backend is the concrete second implementation that justifies
separating language-neutral generator concepts from C++ generation.

Prefer ordinary data structures and functions over elaborate class hierarchies.

Avoid speculative abstractions such as:

- generic plugin architectures
- visitor frameworks without a concrete need
- abstract generator factories
- deep inheritance trees
- configurable rendering pipelines for hypothetical languages

A useful rule is:

> Abstract when a second implementation or concrete boundary exists, not when
> something could theoretically vary.


## C++ Generator

The existing C++ generator defines current compatibility behavior.

During the initial refactor, move C++-specific decisions behind the C++ backend
without redesigning those decisions at the same time.

A successful intermediate milestone is:

    old IDL
       |
       v
    new parser / IR architecture
       |
       v
    C++ backend
       |
       v
    same generated C++ as before

Only after that milestone should intentional C++ output improvements be
considered.


## Python Generator

Python generation is a first-class target, not a thin afterthought around the
C++ implementation.

The eventual goal is to generate both:

- Python clients
- Python servers

Python code should feel natural to Python users rather than mechanically
transliterating generated C++.

However, protocol semantics must remain compatible across languages.

Do not design the common IR around Python conveniences either.


## Cross-Language Compatibility

Eventually the following combinations should interoperate:

    C++ server    <-> C++ client
    C++ server    <-> Python client
    Python server <-> C++ client
    Python server <-> Python client

Use shared representative IDLs for these tests.

Cross-language integration tests are especially valuable because they verify
actual protocol compatibility rather than merely verifying generated text.

When Python support exists, prefer at least one small end-to-end integration
fixture that exercises both an RPC and a signal if the runtime makes this
practical.


## Client and Server Symmetry

Do not assume C++ is always the server or Python is always the client.

The semantic model should describe the protocol independently of which language
implements either side.

Avoid structures equivalent to:

    CppServerModel
        -> somehow generate Python client

Prefer:

    ProtocolModel
        -> CppServerGenerator
        -> CppClientGenerator
        -> PythonServerGenerator
        -> PythonClientGenerator


## Generated Code vs Runtime Code

Keep generator concerns and runtime-library concerns distinct.

Generated code may depend on small language-specific CapnZero runtime libraries
where that substantially simplifies generated output.

Do not generate enormous amounts of repetitive infrastructure merely because
the generator can.

Conversely, do not hide protocol-specific generated information inside runtime
reflection or dynamic machinery without a concrete reason.

Prefer generated code that is:

- understandable
- debuggable
- deterministic
- reasonably small
- statically checkable where the language permits it


## Deterministic Generation

Generation should be deterministic.

The same IDL and generator version should produce the same output regardless of:

- execution order
- hash-map iteration order
- temporary filesystem state
- unrelated files
- machine-specific paths

Avoid embedding timestamps or environment-specific information in generated
files unless explicitly required.


## Error Handling

Invalid IDL should fail clearly and as early as practical.

Prefer errors that identify:

- the invalid construct
- its source location when available
- why it is invalid

Do not silently reinterpret malformed input merely to continue generation.

Do not catch broad exceptions only to replace useful diagnostics with generic
errors.


## Python Implementation Style

The generator itself is Python.

Prefer straightforward modern Python.

Use:

- type annotations where they improve understanding
- dataclasses or similarly simple value types for structural data when useful
- explicit names
- small cohesive modules
- deterministic transformations
- pure functions where they naturally fit

Avoid:

- clever metaprogramming
- excessive inheritance
- global mutable state
- hidden mutation across generation phases
- giant functions mixing parsing, validation, and rendering
- stringly typed semantic models when proper structured values are practical

Do not perform a broad style rewrite merely for stylistic consistency.


## Rendering

Keep semantic decisions separate from textual formatting.

Avoid spreading large source-code string concatenations throughout semantic
processing.

It should be possible to understand:

    protocol semantics

without simultaneously understanding:

    indentation and source formatting

The exact rendering mechanism is not prescribed. Improve it only as far as
needed to make generation maintainable.

Do not introduce a heavy templating framework unless the existing complexity
demonstrates a concrete need.


## Dependencies

Do not add dependencies casually.

Before adding a dependency, determine whether the repository already contains
an appropriate solution or whether the Python standard library is sufficient.

A new dependency should solve a concrete problem and provide enough value to
justify installation and maintenance cost.


## Refactoring Workflow

Prefer this loop:

    inspect
      |
      v
    establish/test current behavior
      |
      v
    make one coherent structural change
      |
      v
    run tests
      |
      v
    inspect generated diff
      |
      v
    continue

Do not accumulate many unrelated refactors before running tests.

When generated output changes unexpectedly, investigate the change rather than
blindly updating golden files.


## Golden Files

Golden files are compatibility evidence, not obstacles to refactoring.

Never update expected generated output simply because a refactor changed it.

When a golden diff occurs, determine whether it is:

1. accidental behavior change,
2. harmless formatting change,
3. intentional generator improvement,
4. bug fix.

During compatibility-preserving refactoring, category 1 must be fixed and
categories 2-4 should normally be deferred.

Intentional output changes should happen in dedicated changes with tests and a
clear explanation.


## Commit / Change Scope

Keep changes reviewable.

Prefer changes such as:

    Extract semantic Method model from C++ generator

over:

    Rewrite generator architecture and add Python support

A useful progression may be:

1. establish baseline and characterization tests
2. isolate parsing
3. introduce/extract language-neutral semantic model
4. move C++ type mapping into C++ backend
5. move C++ rendering into C++ backend
6. verify existing C++ generation remains compatible
7. introduce Python type mapping
8. generate Python client
9. generate Python server
10. add cross-language integration tests

This ordering is guidance, not a mandate. Adapt it to what the repository
actually contains.


## When Unsure

Do not guess about existing CapnZero semantics.

Inspect:

- existing tests
- existing generated examples
- generator implementation
- documentation
- runtime code
- actual users of generated code

If these disagree, report the discrepancy before choosing one interpretation.


## Working With Existing Ugly Code

Ugly code that currently works is evidence.

Understand why it exists before replacing it.

Some awkward-looking behavior may encode:

- wire compatibility
- historical API compatibility
- code-generation ordering requirements
- language edge cases
- runtime assumptions

Delete complexity when it is demonstrated to be unnecessary, not merely because
the replacement looks cleaner.


## Definition of Success

The refactor is successful when:

- the existing C++ generator behavior remains protected by tests
- parsing is separated from target-language rendering
- CapnZero protocol semantics have a clear language-neutral representation
- C++ generation consumes that representation
- Python generation can consume the same representation
- Python clients and servers can be generated
- C++ and Python implementations interoperate
- generator code is easier to understand and modify
- adding Python did not require duplicating the parser or protocol model

The ultimate architecture should make this statement true:

> CapnZero defines a protocol. C++ and Python are implementations of that
> protocol, not part of its definition.
