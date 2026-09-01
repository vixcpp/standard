/**
 *
 *  @file ReadWriteTest.cpp
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

#include <vix/Read.hpp>
#include <vix/Write.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <type_traits>
#include <vector>

namespace
{
  enum class ReadError
  {
    failed
  };

  enum class WriteError
  {
    failed
  };

  class TestInput
  {
  public:
    using error_type = ReadError;

    explicit TestInput(
        std::span<const std::byte> data) noexcept
        : data_(data)
    {
    }

    [[nodiscard]] vix::Result<std::size_t, error_type> read(
        std::span<std::byte> buffer)
    {
      if (failed_)
      {
        return vix::Result<
            std::size_t,
            error_type>::failure(ReadError::failed);
      }

      if (position_ >= data_.size())
      {
        return vix::Result<
            std::size_t,
            error_type>::success(0);
      }

      const auto remaining =
          data_.size() - position_;

      const auto count =
          std::min(
              buffer.size(),
              remaining);

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

    void fail() noexcept
    {
      failed_ = true;
    }

  private:
    std::span<const std::byte> data_;
    std::size_t position_{0};
    bool failed_{false};
  };

  class ConstTestInput
  {
  public:
    using error_type = ReadError;

    [[nodiscard]] vix::Result<std::size_t, error_type> read(
        std::span<std::byte> buffer) const
    {
      if (buffer.empty())
      {
        return vix::Result<
            std::size_t,
            error_type>::success(0);
      }

      buffer.front() = std::byte{0x2A};

      return vix::Result<
          std::size_t,
          error_type>::success(1);
    }
  };

  class TestOutput
  {
  public:
    using error_type = WriteError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data)
    {
      if (failed_)
      {
        return vix::Result<
            std::size_t,
            error_type>::failure(WriteError::failed);
      }

      bytes_.insert(
          bytes_.end(),
          data.begin(),
          data.end());

      return vix::Result<
          std::size_t,
          error_type>::success(data.size());
    }

    [[nodiscard]] const std::vector<std::byte> &bytes() const noexcept
    {
      return bytes_;
    }

    void fail() noexcept
    {
      failed_ = true;
    }

  private:
    std::vector<std::byte> bytes_;
    bool failed_{false};
  };

  class PartialOutput
  {
  public:
    using error_type = WriteError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data)
    {
      if (data.empty())
      {
        return vix::Result<
            std::size_t,
            error_type>::success(0);
      }

      return vix::Result<
          std::size_t,
          error_type>::success(1);
    }
  };

  class ConstTestOutput
  {
  public:
    using error_type = WriteError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data) const
    {
      return vix::Result<
          std::size_t,
          error_type>::success(data.size());
    }
  };

  static_assert(vix::Input<TestInput>);
  static_assert(vix::ConstInput<ConstTestInput>);

  static_assert(vix::Output<TestOutput>);
  static_assert(vix::Output<PartialOutput>);
  static_assert(vix::ConstOutput<ConstTestOutput>);

  static_assert(std::same_as<
                decltype(vix::read(
                    std::declval<TestInput &>(),
                    std::declval<std::span<std::byte>>())),
                vix::Result<std::size_t, ReadError>>);

  static_assert(std::same_as<
                decltype(vix::write(
                    std::declval<TestOutput &>(),
                    std::declval<std::span<const std::byte>>())),
                vix::Result<std::size_t, WriteError>>);

  TEST(ReadWriteTest, ReadDelegatesToInput)
  {
    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}};

    TestInput input(source);

    std::array<std::byte, 3> buffer{};

    auto result =
        vix::read(input, buffer);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 3);
    EXPECT_EQ(buffer, source);
  }

  TEST(ReadWriteTest, ReadPreservesPartialProgress)
  {
    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}};

    TestInput input(source);

    std::array<std::byte, 2> buffer{};

    auto result =
        vix::read(input, buffer);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 2);

    EXPECT_EQ(buffer[0], std::byte{0x01});
    EXPECT_EQ(buffer[1], std::byte{0x02});
  }

  TEST(ReadWriteTest, ReadPreservesInputPositionAcrossCalls)
  {
    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}};

    TestInput input(source);

    std::array<std::byte, 2> first{};
    std::array<std::byte, 2> second{};

    auto first_result =
        vix::read(input, first);

    auto second_result =
        vix::read(input, second);

    ASSERT_TRUE(first_result);
    ASSERT_TRUE(second_result);

    EXPECT_EQ(first_result.value(), 2);
    EXPECT_EQ(second_result.value(), 2);

    EXPECT_EQ(first[0], std::byte{0x01});
    EXPECT_EQ(first[1], std::byte{0x02});

    EXPECT_EQ(second[0], std::byte{0x03});
    EXPECT_EQ(second[1], std::byte{0x04});
  }

  TEST(ReadWriteTest, ReadReturnsZeroAtEndOfInput)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    TestInput input(source);

    std::array<std::byte, 1> buffer{};

    auto first =
        vix::read(input, buffer);

    auto second =
        vix::read(input, buffer);

    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    EXPECT_EQ(first.value(), 1);
    EXPECT_EQ(second.value(), 0);
  }

  TEST(ReadWriteTest, ReadPreservesInputErrorType)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    TestInput input(source);
    input.fail();

    std::array<std::byte, 1> buffer{};

    auto result =
        vix::read(input, buffer);

    static_assert(std::same_as<
                  decltype(result),
                  vix::Result<std::size_t, ReadError>>);

    ASSERT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        ReadError::failed);
  }

  TEST(ReadWriteTest, ReadSupportsEmptyBuffer)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    TestInput input(source);

    std::span<std::byte> empty;

    auto result =
        vix::read(input, empty);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 0);
  }

  TEST(ReadWriteTest, ReadSupportsConstInput)
  {
    const ConstTestInput input;

    std::array<std::byte, 1> buffer{};

    auto result =
        vix::read(input, buffer);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 1);
    EXPECT_EQ(
        buffer.front(),
        std::byte{0x2A});
  }

  TEST(ReadWriteTest, WriteDelegatesToOutput)
  {
    TestOutput output;

    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}};

    auto result =
        vix::write(output, data);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 3);

    ASSERT_EQ(output.bytes().size(), 3);

    EXPECT_EQ(
        output.bytes()[0],
        std::byte{0x01});

    EXPECT_EQ(
        output.bytes()[1],
        std::byte{0x02});

    EXPECT_EQ(
        output.bytes()[2],
        std::byte{0x03});
  }

  TEST(ReadWriteTest, WritePreservesPartialProgress)
  {
    PartialOutput output;

    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}};

    auto result =
        vix::write(output, data);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 1);
  }

  TEST(ReadWriteTest, WritePreservesOutputErrorType)
  {
    TestOutput output;
    output.fail();

    constexpr std::array data{
        std::byte{0x2A}};

    auto result =
        vix::write(output, data);

    static_assert(std::same_as<
                  decltype(result),
                  vix::Result<std::size_t, WriteError>>);

    ASSERT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        WriteError::failed);

    EXPECT_TRUE(output.bytes().empty());
  }

  TEST(ReadWriteTest, WriteSupportsEmptySpan)
  {
    TestOutput output;

    std::span<const std::byte> empty;

    auto result =
        vix::write(output, empty);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 0);
    EXPECT_TRUE(output.bytes().empty());
  }

  TEST(ReadWriteTest, WriteSupportsSeveralCalls)
  {
    TestOutput output;

    constexpr std::array first{
        std::byte{0x01},
        std::byte{0x02}};

    constexpr std::array second{
        std::byte{0x03},
        std::byte{0x04}};

    auto first_result =
        vix::write(output, first);

    auto second_result =
        vix::write(output, second);

    ASSERT_TRUE(first_result);
    ASSERT_TRUE(second_result);

    EXPECT_EQ(first_result.value(), 2);
    EXPECT_EQ(second_result.value(), 2);

    ASSERT_EQ(output.bytes().size(), 4);

    EXPECT_EQ(output.bytes()[0], std::byte{0x01});
    EXPECT_EQ(output.bytes()[1], std::byte{0x02});
    EXPECT_EQ(output.bytes()[2], std::byte{0x03});
    EXPECT_EQ(output.bytes()[3], std::byte{0x04});
  }

  TEST(ReadWriteTest, WriteSupportsConstOutput)
  {
    const ConstTestOutput output;

    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02}};

    auto result =
        vix::write(output, data);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 2);
  }

  TEST(ReadWriteTest, ReadAndWritePreserveIndependentErrors)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    TestInput input(source);
    TestOutput output;

    std::array<std::byte, 1> buffer{};

    auto read_result =
        vix::read(input, buffer);

    auto write_result =
        vix::write(output, buffer);

    static_assert(std::same_as<
                  decltype(read_result),
                  vix::Result<std::size_t, ReadError>>);

    static_assert(std::same_as<
                  decltype(write_result),
                  vix::Result<std::size_t, WriteError>>);

    ASSERT_TRUE(read_result);
    ASSERT_TRUE(write_result);

    EXPECT_EQ(read_result.value(), 1);
    EXPECT_EQ(write_result.value(), 1);
  }

} // namespace
