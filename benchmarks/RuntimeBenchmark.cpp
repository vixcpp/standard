/**
 *
 *  @file RuntimeBenchmark.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/vixcpp/standard
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Vix.cpp Standard
 */

#include <vix/Duration.hpp>
#include <vix/Log.hpp>
#include <vix/Parse.hpp>
#include <vix/Read.hpp>
#include <vix/Result.hpp>
#include <vix/Run.hpp>
#include <vix/Serialize.hpp>
#include <vix/Task.hpp>
#include <vix/Validate.hpp>
#include <vix/Write.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string_view>

namespace
{
  enum class Error
  {
    failed,
    invalid
  };

  class MemoryInput
  {
  public:
    using error_type = Error;

    explicit MemoryInput(
        std::span<const std::byte> data) noexcept
        : data_(data)
    {
    }

    [[nodiscard]] vix::Result<std::size_t, error_type> read(
        std::span<std::byte> buffer) noexcept
    {
      if (position_ >= data_.size())
      {
        return vix::Result<
            std::size_t,
            error_type>::success(0);
      }

      const auto count =
          std::min(
              buffer.size(),
              data_.size() - position_);

      std::copy_n(
          data_.begin() +
              static_cast<std::ptrdiff_t>(position_),
          count,
          buffer.begin());

      position_ += count;

      return vix::Result<
          std::size_t,
          error_type>::success(count);
    }

    void reset() noexcept
    {
      position_ = 0;
    }

  private:
    std::span<const std::byte> data_;
    std::size_t position_{0};
  };

  class MemoryOutput
  {
  public:
    using error_type = Error;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data) noexcept
    {
      const auto remaining =
          bytes_.size() - position_;

      const auto count =
          std::min(
              data.size(),
              remaining);

      std::copy_n(
          data.begin(),
          count,
          bytes_.begin() +
              static_cast<std::ptrdiff_t>(position_));

      position_ += count;

      return vix::Result<
          std::size_t,
          error_type>::success(count);
    }

    void reset() noexcept
    {
      position_ = 0;
    }

  private:
    std::array<std::byte, 256> bytes_{};
    std::size_t position_{0};
  };

  struct PositiveValidator
  {
    [[nodiscard]] vix::Result<void, Error> operator()(
        int value) const noexcept
    {
      if (value <= 0)
      {
        return vix::Result<
            void,
            Error>::failure(Error::invalid);
      }

      return vix::Result<
          void,
          Error>::success();
    }
  };

  class Runnable
  {
  public:
    [[nodiscard]] int run(
        int value) const noexcept
    {
      return value * 2;
    }
  };

  class LogTarget
  {
  public:
    void log(
        vix::LogLevel level,
        std::string_view message) noexcept
    {
      level_ = level;
      size_ = message.size();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
      return size_;
    }

  private:
    vix::LogLevel level_{vix::LogLevel::info};
    std::size_t size_{0};
  };

  vix::Task<int> make_task(
      int value)
  {
    co_return value;
  }

  void ResultSuccess(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto result =
          vix::Result<int, Error>::success(42);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(result.value());
    }
  }

  void ResultFailure(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto result =
          vix::Result<int, Error>::failure(
              Error::failed);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(result.error());
    }
  }

  void ResultMap(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto result =
          vix::Result<int, Error>::success(21);

      auto mapped =
          result.map(
              [](int value) noexcept
              {
                return value * 2;
              });

      benchmark::DoNotOptimize(mapped);
      benchmark::DoNotOptimize(mapped.value());
    }
  }

  void ResultAndThen(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto result =
          vix::Result<int, Error>::success(21);

      auto chained =
          result.and_then(
              [](int value)
              {
                return vix::Result<
                    int,
                    Error>::success(value * 2);
              });

      benchmark::DoNotOptimize(chained);
      benchmark::DoNotOptimize(chained.value());
    }
  }

  void ParseInteger(
      benchmark::State &state)
  {
    constexpr std::string_view input{
        "123456"};

    for (auto _ : state)
    {
      auto result =
          vix::parse<int>(input);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(result.value());
    }
  }

  void ParseDouble(
      benchmark::State &state)
  {
    constexpr std::string_view input{
        "12345.6789"};

    for (auto _ : state)
    {
      auto result =
          vix::parse<double>(input);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(result.value());
    }
  }

  void SerializeInteger(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto result =
          vix::serialize(123456);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(result.value());
    }
  }

  void SerializeDouble(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto result =
          vix::serialize(12345.6789);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(result.value());
    }
  }

  void ValidateSuccess(
      benchmark::State &state)
  {
    PositiveValidator validator;

    for (auto _ : state)
    {
      auto result =
          vix::validate(
              42,
              validator);

      benchmark::DoNotOptimize(result);
    }
  }

  void ValidateFailure(
      benchmark::State &state)
  {
    PositiveValidator validator;

    for (auto _ : state)
    {
      auto result =
          vix::validate(
              -1,
              validator);

      benchmark::DoNotOptimize(result);
    }
  }

  void DurationArithmetic(
      benchmark::State &state)
  {
    const auto first =
        vix::Duration::milliseconds(500);

    const auto second =
        vix::Duration::microseconds(250);

    for (auto _ : state)
    {
      auto result =
          (first + second) * 4;

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(
          result.count_nanoseconds());
    }
  }

  void ReadMemory(
      benchmark::State &state)
  {
    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
        std::byte{0x06},
        std::byte{0x07},
        std::byte{0x08}};

    MemoryInput input(source);
    std::array<std::byte, 8> buffer{};

    for (auto _ : state)
    {
      input.reset();

      auto result =
          vix::read(
              input,
              buffer);

      benchmark::DoNotOptimize(result);
      benchmark::DoNotOptimize(buffer);
    }

    state.SetBytesProcessed(
        state.iterations() *
        static_cast<std::int64_t>(
            source.size()));
  }

  void WriteMemory(
      benchmark::State &state)
  {
    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
        std::byte{0x06},
        std::byte{0x07},
        std::byte{0x08}};

    MemoryOutput output;

    for (auto _ : state)
    {
      output.reset();

      auto result =
          vix::write(
              output,
              data);

      benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(
        state.iterations() *
        static_cast<std::int64_t>(
            data.size()));
  }

  void RunOperation(
      benchmark::State &state)
  {
    Runnable runnable;

    for (auto _ : state)
    {
      auto result =
          vix::run(
              runnable,
              21);

      benchmark::DoNotOptimize(result);
    }
  }

  void LogOperation(
      benchmark::State &state)
  {
    LogTarget target;

    for (auto _ : state)
    {
      vix::log(
          target,
          vix::LogLevel::info,
          "runtime benchmark");

      benchmark::DoNotOptimize(
          target.size());
    }
  }

  void TaskCreateAndComplete(
      benchmark::State &state)
  {
    for (auto _ : state)
    {
      auto task =
          make_task(42);

      benchmark::DoNotOptimize(task);

      task.start();

      benchmark::DoNotOptimize(
          task.result());
    }
  }

  BENCHMARK(ResultSuccess);
  BENCHMARK(ResultFailure);
  BENCHMARK(ResultMap);
  BENCHMARK(ResultAndThen);

  BENCHMARK(ParseInteger);
  BENCHMARK(ParseDouble);

  BENCHMARK(SerializeInteger);
  BENCHMARK(SerializeDouble);

  BENCHMARK(ValidateSuccess);
  BENCHMARK(ValidateFailure);

  BENCHMARK(DurationArithmetic);

  BENCHMARK(ReadMemory);
  BENCHMARK(WriteMemory);

  BENCHMARK(RunOperation);
  BENCHMARK(LogOperation);

  BENCHMARK(TaskCreateAndComplete);

} // namespace

BENCHMARK_MAIN();
