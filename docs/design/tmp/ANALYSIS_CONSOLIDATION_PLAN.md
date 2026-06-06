# Analysis Consolidation Plan

## Target shape

### Application
- `AnalysisService` as the single entry point for running analyses
- `AnalysisRequest` as the use-case input DTO
- `AnalysisResult` as the use-case output DTO
- `AnalysisFilter` as the runtime filter helper over `WorkspaceState`
- `TableStrategy`, `PlotStrategy`, `CalcStrategy` as internal execution strategies

### Domain
- `Analysis`
- `AnalysisType`
- `AnalysisPolicy`
- `FilterSpecification` as the domain value object

## Keep in application
- Request/result DTOs
- execution orchestration
- strategy selection
- workspace filtering
- result assembly

## Keep in domain
- analysis state
- invariants
- reusable rules
- filter specification as a value object

## Rename candidates
- `RunAnalysis` -> `AnalysisService`
- `RunAnalysisRequest` -> `AnalysisRequest`
- `RunAnalysisResult` -> `AnalysisResult`
- `AnalysisEngine` -> merged into `AnalysisService`
- `Filter` -> `AnalysisFilter`
- `FilterSpec` -> `FilterSpecification`
- `TabAnalysisStrategy` -> `TableStrategy`
- `PlotAnalysisStrategy` -> `PlotStrategy`
- `CalcAnalysisStrategy` -> `CalcStrategy`

## Delete after consolidation
- legacy duplicate analysis entry points
- redundant engine wrapper
- old abbreviated strategy names
- obsolete request composer if it becomes unused
