using BenchmarkDotNet.Configs;
using BenchmarkDotNet.Jobs;
using BenchmarkDotNet.Running;
using Benchmarks;

// Debug config unused but left for convenience to allow benchmark debugging
var debugConfig = ManualConfig.Create(DefaultConfig.Instance)
    .AddJob(Job.InProcess)
    .WithOptions(ConfigOptions.DisableOptimizationsValidator);

BenchmarkRunner.Run<BondTradeLoaderBenchmarks>();