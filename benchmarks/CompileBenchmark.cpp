/**
 *
 *  @file CompileBenchmark.cpp
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
#include <vix/Input.hpp>
#include <vix/Instant.hpp>
#include <vix/Log.hpp>
#include <vix/Output.hpp>
#include <vix/Parse.hpp>
#include <vix/Print.hpp>
#include <vix/Read.hpp>
#include <vix/Result.hpp>
#include <vix/Run.hpp>
#include <vix/Serialize.hpp>
#include <vix/Task.hpp>
#include <vix/Validate.hpp>
#include <vix/Write.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

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
        std::span<std::byte> buffer)
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

  private:
    std::span<const std::byte> data_;
    std::size_t position_{0};
  };

  class MemoryOutput
  {
  public:
    using error_type = Error;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data)
    {
      bytes_.insert(
          bytes_.end(),
          data.begin(),
          data.end());

      return vix::Result<
          std::size_t,
          error_type>::success(data.size());
    }

  private:
    std::vector<std::byte> bytes_;
  };

  struct PositiveValidator
  {
    [[nodiscard]] vix::Result<void, Error> operator()(
        int value) const
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
    [[nodiscard]] vix::Result<int, Error> run(
        int value)
    {
      return vix::Result<
          int,
          Error>::success(value * 2);
    }
  };

  class LogSink
  {
  public:
    void log(
        vix::LogLevel level,
        std::string_view message)
    {
      level_ = level;
      message_.assign(message);
    }

  private:
    vix::LogLevel level_{vix::LogLevel::info};
    std::string message_;
  };

  vix::Task<int> asynchronous_value(
      int value)
  {
    co_return value;
  }

  [[nodiscard]] int exercise_standard_surface()
  {
    auto parsed =
        vix::parse<int>("21");

    if (!parsed)
    {
      return 1;
    }

    auto valid =
        vix::validate(
            parsed.value(),
            PositiveValidator{});

    if (!valid)
    {
      return 1;
    }

    auto mapped =
        parsed.map(
            [](int value)
            {
              return value * 2;
            });

    if (!mapped)
    {
      return 1;
    }

    auto serialized =
        vix::serialize(
            mapped.value());

    if (!serialized)
    {
      return 1;
    }

    const auto duration =
        vix::Duration::milliseconds(250);

    const auto instant =
        vix::Instant::now();

    const auto deadline =
        instant + duration;

    if (deadline < instant)
    {
      return 1;
    }

    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}};

    MemoryInput input(source);
    MemoryOutput output;

    std::array<std::byte, 4> buffer{};

    auto read_result =
        vix::read(
            input,
            buffer);

    if (!read_result)
    {
      return 1;
    }

    auto write_result =
        vix::write(
            output,
            std::span<const std::byte>{
                buffer.data(),
                read_result.value()});

    if (!write_result)
    {
      return 1;
    }

    auto print_result =
        vix::print(
            output,
            serialized.value());

    if (!print_result)
    {
      return 1;
    }

    Runnable runnable;

    auto run_result =
        vix::run(
            runnable,
            parsed.value());

    if (!run_result)
    {
      return 1;
    }

    LogSink log_sink;

    vix::log(
        log_sink,
        vix::LogLevel::info,
        "compile benchmark");

    auto task =
        asynchronous_value(
            run_result.value());

    task.start();

    if (!task.done())
    {
      return 1;
    }

    return task.result() == 42
               ? 0
               : 1;
  }

} // namespace

/*
 * This executable is a compilation workload, not a runtime benchmark.
 *
 * Its build duration should be measured externally by the compiler, build
 * system, or CI harness. The translation unit intentionally exercises the
 * public Standard surface so changes to header weight and template complexity
 * can be compared over time.
 */
int main()
{
  return exercise_standard_surface();
}
