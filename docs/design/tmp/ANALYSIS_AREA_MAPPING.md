# Analysis Area Mapping

## Short answer
Yes: `RunAnalysisRequest` and `RunAnalysisResult` are DTOs. They should still live in the application layer because they define the use-case boundary, not the domain model.

## Goal
Make the analysis area smaller, clearer, and more consistent:
- domain holds the analysis model and rules
- application holds execution, orchestration, and result assembly
- names are shorter, explicit, and less abbreviated

## Recommended target structure

### Domain
```text
core/include/core/domain/analysis/
  Analysis.h
  AnalysisType.h
  AnalysisPolicy.h
  FilterSpecification.h
  AnalysisOutputType.h   (optional, only if it becomes a stable domain concept)
```

### Application
```text
core/include/core/application/analysis/
  AnalysisService.h
  AnalysisRequest.h
  AnalysisResult.h
  AnalysisFilter.h
  strategies/
    TableStrategy.h
    PlotStrategy.h
    CalcStrategy.h
```

### Source layout
```text
core/src/application/analysis/
  AnalysisService.cpp
  AnalysisFilter.cpp
  AnalysisOutputPolicy.cpp   (optional, if output-type selection stays separate)
  strategies/
    TableStrategy.cpp
    PlotStrategy.cpp
    CalcStrategy.cpp
```

## File-by-file mapping

### Keep in application
- `core/include/core/application/analysis/RunAnalysisRequest.h`
  - rename to `AnalysisRequest.h`
  - stays application DTO
- `core/include/core/application/analysis/RunAnalysisResult.h`
  - rename to `AnalysisResult.h`
  - stays application DTO / presenter-facing contract
- `core/include/core/application/analysis/RunAnalysis.h`
  - rename to `AnalysisService.h` or `AnalysisRunner.h`
  - stays application orchestration
- `core/include/core/application/analysis/AnalysisEngine.h`
  - either merge into `AnalysisService` or replace with it
  - should not be a parallel concept to `RunAnalysis`
- `core/src/application/analysis/RunAnalysis.cpp`
  - rename to `AnalysisService.cpp` if the service becomes the entry point
- `core/src/application/analysis/AnalysisEngine.cpp`
  - merge or delete after consolidation
- `core/src/application/analysis/ComposeAnalysisRequest.cpp`
  - keep only if it remains a real mapper/builder
  - otherwise fold into `AnalysisService`
- `core/include/core/application/analysis/ComposeAnalysisRequest.h`
  - same decision as source file
- `core/src/application/analysis/Filter.cpp`
  - rename to `AnalysisFilter.cpp`
  - keep in application if it applies selection over workspace data
- `core/include/core/application/analysis/Filter.h`
  - rename to `AnalysisFilter.h`
- `core/src/application/analysis/OutputTypeResolver.cpp`
  - either fold into a policy or keep as internal application helper
- `core/src/application/analysis/OutputTypeResolver.h`
  - same decision as source file
- `core/src/application/analysis/FilteredTransactions.h`
  - rename to something clearer like `AnalysisSelection.h` or `AnalysisTransactionSet.h`
  - keep only if it represents an application-level result shape
- `core/src/application/analysis/strategies/CalcAnalysisStrategy.cpp`
  - rename to `CalcStrategy.cpp` or keep as `CalcStrategy.cpp`
- `core/src/application/analysis/strategies/PlotAnalysisStrategy.cpp`
  - rename to `PlotStrategy.cpp`
- `core/src/application/analysis/strategies/TabAnalysisStrategy.cpp`
  - rename to `TableStrategy.cpp`

### Move to domain
- `core/include/core/domain/values/FilterSpec.h`
  - consider renaming to `FilterSpecification.h`
  - remains a domain value object
- `core/include/core/domain/entities/Analysis.h`
  - stays domain entity
- `core/include/core/domain/values/AnalysisType.h`
  - stays domain value object
- `core/include/core/domain/policies/AnalysisPolicy.h`
  - stays domain policy

### Possibly move from domain to application
If any future analysis code in domain starts doing execution-like work, move it back out:
- filter application over `WorkspaceState`
- analysis result materialization
- output-type resolution if it depends on runtime context rather than pure analysis state

### Candidate deletions after consolidation
These should go away if the target structure is adopted fully:
- `core/src/application/analysis/AnalysisEngine.cpp`
- `core/include/core/application/analysis/AnalysisEngine.h`
- `core/include/core/application/analysis/RunAnalysis.h`
- `core/include/core/application/analysis/RunAnalysisRequest.h`
- `core/include/core/application/analysis/RunAnalysisResult.h`
- `core/src/application/analysis/ComposeAnalysisRequest.cpp`
- `core/include/core/application/analysis/ComposeAnalysisRequest.h`
- `core/src/application/analysis/OutputTypeResolver.cpp` if merged into `AnalysisService`
- `core/src/application/analysis/OutputTypeResolver.h` if merged into `AnalysisService`

## Naming decisions I would recommend
- `TabStrategy` -> `TableStrategy` is better
- `PlotStrategy` stays as-is
- `CalcStrategy` should probably stay separate, but not as the main UI selection path
- `FilterSpec` -> `FilterSpecification` is a good cleanup candidate
- `RunAnalysisRequest` -> `AnalysisRequest`
- `RunAnalysisResult` -> `AnalysisResult`
- `AnalysisEngine` -> `AnalysisService`

## My recommendation on the DTO question
Yes, they are DTOs.
But DTOs do not have to live in a separate `dto/` folder if the application layer is the use-case boundary.

So the cleanest split is:
- domain: model and rules
- application: DTOs, orchestration, selection, result building

## Suggested next step
If you want, I can turn this mapping into an actual refactor plan and start with the non-breaking rename/consolidation sequence.
