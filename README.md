# Compiler Workbench

## Overview

Compiler Workbench is a desktop Qt application for exploring core compiler construction topics in one integrated interface. It combines finite automata design, lexical analysis, parsing, semantic analysis, and basic target-code generation in a single workbench-style application.

The project is aimed at education and experimentation. It lets a user move through a simplified compiler pipeline visually and interactively instead of treating each stage as an isolated console tool.

## What the Project Does

The application provides four main work areas:

1. `Automaton Designer`
2. `Lexical Analyzer`
3. `Parser / Parse Tree`
4. `Semantic Analysis`

Together, these sections cover the flow from automata construction through tokenization, parsing, semantic checking, and source-to-source code generation.

## Main Features

### 1. Automaton Designer

- Create and manage multiple automata
- Build both DFAs and NFAs
- Add states visually on a canvas
- Add transitions between states
- Mark states as initial or final
- Rename automata and states
- Delete states, transitions, and entire automata
- Validate DFA constraints while adding transitions
- Test whether an input string is accepted
- Trace execution step by step
- Convert NFA to DFA
- Minimize DFA
- Build an automaton from a regular expression

### 2. Lexical Analyzer

- Enter source code in a dark editor
- Tokenize the source using the project lexer
- Toggle `Skip Whitespace`
- Toggle `Skip Comments`
- Inspect generated tokens in a table
- View lexical diagnostics in a separate diagnostics panel
- Clear the analyzer state quickly

### 3. Parser / Parse Tree

- Load predefined grammars
- Add grammar productions manually
- Delete selected productions
- Clear all grammar productions
- Inspect grammar metadata
- Parse input strings using the current grammar
- View parse output and parse errors
- Open the generated parse tree in a dedicated popup viewer

### 4. Semantic Analysis and Code Generation

- Analyze tokenized source code semantically
- Populate a symbol table
- Inspect the generated AST
- View semantic diagnostics
- Generate target code
- Switch between supported target languages:
  - Python
  - Java
  - JavaScript
  - Assembly
- Copy generated code to the clipboard

## Technology Stack

- `C++17`
- `Qt 6`
- `qmake`
- `MinGW` toolchain on Windows

## Project Structure

```text
src/
  main.cpp
  models/
    Automaton/
    Grammar/
    LexicalAnalysis/
    Semantic/
  ui/
    Automaton/
    Grammar/
    LexicalAnalysis/
    Semantic/
    AppTheme.*
    MainWindow.*
    SyntaxHighlighter.*
  utils/
    Automaton/
    Grammar/
    LexicalAnalysis/
    Semantic/
```

### High-Level Breakdown

- `models/`
  Core domain objects such as automata, tokens, grammar structures, parse trees, AST nodes, and symbol-table entities.

- `utils/`
  Processing logic including NFA-to-DFA conversion, DFA minimization, regex conversion, lexing, parsing, semantic analysis, and code generation.

- `ui/`
  The Qt widgets and presentation layer for each workbench tab.

## Build Instructions

### Prerequisites

- Qt 6 installed
- MinGW toolchain compatible with the Qt installation
- `qmake`
- `mingw32-make`

### Build

From the project root:

```powershell
cd build
qmake ..\tf-v3.pro
mingw32-make -f Makefile.Release
```

### Run

```powershell
.\release\CompilerProject.exe
```

## Current Capabilities

The project already supports:

- visual automaton authoring
- DFA and NFA operations
- regex-to-automaton workflow
- source-code tokenization
- grammar-driven parsing
- parse tree inspection
- semantic checking
- AST visualization
- basic multi-target code generation

This makes it useful as both a teaching tool and a compact demo of a staged compiler pipeline.

## What Can Be Improved

### Engineering Improvements

- Add automated tests for:
  - automaton operations
  - lexer behavior
  - parser behavior
  - semantic checks
  - code generation output
- Add regression tests for representative source programs

### Product Improvements

- Add save/load support for automata and project sessions
- Add import/export for grammars
- Add syntax-aware code editor features such as line numbers and better inline diagnostics
- Improve code generation quality and target-language coverage
- Add support for more language constructs in semantic analysis
- Add a ML for code translate
