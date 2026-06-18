# Architecture Rules

## Dependency Flow

To maintain a clean and maintainable architecture, the following dependency flow rules **MUST** be strictly enforced:

1. **UI Layer** depends on **Managers Layer**.
2. **Managers Layer** depends on **Domain Layer** and **Adapters Layer**.
3. **Adapters Layer** depends on **Domain Layer**.
4. **Domain Layer** must remain **pure**. It cannot depend on any higher-level modules, UI components, Adapters, Qt frameworks, or platform-specific APIs (e.g., Windows APIs).

**NO REVERSE DEPENDENCIES ARE ALLOWED.**

## Layer Responsibilities

* **Domain (`src/core/domain`)**: Pure business logic, core entities, and data structures. Uses only standard C++.
* **Adapters (`src/core/adapters`)**: Platform-specific logic, API integrations, and Qt wrappers.
* **Managers (`src/managers`)**: Orchestration of domain objects and adapters.
* **UI (`src/ui`)**: Presentation and user interaction.
