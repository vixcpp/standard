/**
 *
 *  @file InputOutputTest.cpp
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

#include <vix/Input.hpp>
#include <vix/Output.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <type_traits>
#include <vector>

namespace
{
  enum class IoError
  {
    unavailable,
    failed
  };

  class MemoryInput
  {
  public:
    using error_type = IoError;

    explicit MemoryInput(
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
            error_type>::failure(IoError::failed);
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
          std::min(buffer.size(), remaining);

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

  class ConstMemoryInput
  {
  public:
    using error_type = IoError;

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

  class MemoryOutput
  {
  public:
    using error_type = IoError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data)
    {
      if (failed_)
      {
        return vix::Result<
            std::size_t,
            error_type>::failure(IoError::failed);
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
    using error_type = IoError;

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

  class ConstMemoryOutput
  {
  public:
    using error_type = IoError;

    [[nodiscard]] vix::Result<std::size_t, error_type> write(
        std::span<const std::byte> data) const
    {
      return vix::Result<
          std::size_t,
          error_type>::success(data.size());
    }
  };

  class InvalidInputWithoutErrorType
  {
  public:
    [[nodiscard]] vix::Result<std::size_t, IoError> read(
        std::span<std::byte>)
    {
      return vix::Result<
          std::size_t,
          IoError>::success(0);
    }
  };

  class InvalidInputReturnType
  {
  public:
    using error_type = IoError;

    [[nodiscard]] std::size_t read(
        std::span<std::byte>)
    {
      return 0;
    }
  };

  class InvalidOutputWithoutErrorType
  {
  public:
    [[nodiscard]] vix::Result<std::size_t, IoError> write(
        std::span<const std::byte>)
    {
      return vix::Result<
          std::size_t,
          IoError>::success(0);
    }
  };

  class InvalidOutputReturnType
  {
  public:
    using error_type = IoError;

    [[nodiscard]] std::size_t write(
        std::span<const std::byte>)
    {
      return 0;
    }
  };

  static_assert(vix::Input<MemoryInput>);
  static_assert(vix::Input<ConstMemoryInput>);

  static_assert(!vix::ConstInput<MemoryInput>);
  static_assert(vix::ConstInput<ConstMemoryInput>);

  static_assert(vix::Output<MemoryOutput>);
  static_assert(vix::Output<PartialOutput>);
  static_assert(vix::Output<ConstMemoryOutput>);

  static_assert(!vix::ConstOutput<MemoryOutput>);
  static_assert(vix::ConstOutput<ConstMemoryOutput>);

  static_assert(!vix::Input<InvalidInputWithoutErrorType>);
  static_assert(!vix::Input<InvalidInputReturnType>);

  static_assert(!vix::Output<InvalidOutputWithoutErrorType>);
  static_assert(!vix::Output<InvalidOutputReturnType>);

  static_assert(std::same_as<
                vix::input_error_t<MemoryInput>,
                IoError>);

  static_assert(std::same_as<
                vix::output_error_t<MemoryOutput>,
                IoError>);

  TEST(InputOutputTest, InputReadsIntoCallerOwnedBuffer)
  {
    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}};

    MemoryInput input(source);

    std::array<std::byte, 4> buffer{};

    auto result = input.read(buffer);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 4);

    EXPECT_EQ(buffer, source);
  }

  TEST(InputOutputTest, InputSupportsPartialReads)
  {
    constexpr std::array source{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}};

    MemoryInput input(source);

    std::array<std::byte, 2> first{};
    std::array<std::byte, 2> second{};

    auto first_result = input.read(first);
    auto second_result = input.read(second);

    ASSERT_TRUE(first_result);
    ASSERT_TRUE(second_result);

    EXPECT_EQ(first_result.value(), 2);
    EXPECT_EQ(second_result.value(), 2);

    EXPECT_EQ(first[0], std::byte{0x01});
    EXPECT_EQ(first[1], std::byte{0x02});

    EXPECT_EQ(second[0], std::byte{0x03});
    EXPECT_EQ(second[1], std::byte{0x04});
  }

  TEST(InputOutputTest, InputReturnsZeroAtEndOfSource)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    MemoryInput input(source);

    std::array<std::byte, 1> buffer{};

    auto first = input.read(buffer);
    auto second = input.read(buffer);

    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    EXPECT_EQ(first.value(), 1);
    EXPECT_EQ(second.value(), 0);
  }

  TEST(InputOutputTest, InputPreservesSourceSpecificError)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    MemoryInput input(source);
    input.fail();

    std::array<std::byte, 1> buffer{};

    auto result = input.read(buffer);

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), IoError::failed);
  }

  TEST(InputOutputTest, InputCanReadEmptyBuffer)
  {
    constexpr std::array source{
        std::byte{0x2A}};

    MemoryInput input(source);

    std::span<std::byte> empty;

    auto result = input.read(empty);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 0);
  }

  TEST(InputOutputTest, ConstInputSupportsConstReadOperation)
  {
    const ConstMemoryInput input;

    std::array<std::byte, 1> buffer{};

    auto result = input.read(buffer);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 1);
    EXPECT_EQ(buffer.front(), std::byte{0x2A});
  }

  TEST(InputOutputTest, OutputConsumesCallerOwnedBytes)
  {
    MemoryOutput output;

    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}};

    auto result = output.write(data);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 3);

    ASSERT_EQ(output.bytes().size(), 3);
    EXPECT_EQ(output.bytes()[0], std::byte{0x01});
    EXPECT_EQ(output.bytes()[1], std::byte{0x02});
    EXPECT_EQ(output.bytes()[2], std::byte{0x03});
  }

  TEST(InputOutputTest, OutputCanAccumulateSeveralWrites)
  {
    MemoryOutput output;

    constexpr std::array first{
        std::byte{0x01},
        std::byte{0x02}};

    constexpr std::array second{
        std::byte{0x03},
        std::byte{0x04}};

    auto first_result = output.write(first);
    auto second_result = output.write(second);

    ASSERT_TRUE(first_result);
    ASSERT_TRUE(second_result);

    ASSERT_EQ(output.bytes().size(), 4);

    EXPECT_EQ(output.bytes()[0], std::byte{0x01});
    EXPECT_EQ(output.bytes()[1], std::byte{0x02});
    EXPECT_EQ(output.bytes()[2], std::byte{0x03});
    EXPECT_EQ(output.bytes()[3], std::byte{0x04});
  }

  TEST(InputOutputTest, OutputPreservesDestinationSpecificError)
  {
    MemoryOutput output;
    output.fail();

    constexpr std::array data{
        std::byte{0x2A}};

    auto result = output.write(data);

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), IoError::failed);
    EXPECT_TRUE(output.bytes().empty());
  }

  TEST(InputOutputTest, OutputCanWriteEmptySpan)
  {
    MemoryOutput output;

    std::span<const std::byte> empty;

    auto result = output.write(empty);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 0);
    EXPECT_TRUE(output.bytes().empty());
  }

  TEST(InputOutputTest, OutputMayReportPartialProgress)
  {
    PartialOutput output;

    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}};

    auto result = output.write(data);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 1);
  }

  TEST(InputOutputTest, ConstOutputSupportsConstWriteOperation)
  {
    const ConstMemoryOutput output;

    constexpr std::array data{
        std::byte{0x01},
        std::byte{0x02}};

    auto result = output.write(data);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 2);
  }

  TEST(InputOutputTest, InputAndOutputKeepIndependentErrorTypes)
  {
    class CustomInput
    {
    public:
      enum class Error
      {
        exhausted
      };

      using error_type = Error;

      [[nodiscard]] vix::Result<std::size_t, Error> read(
          std::span<std::byte>)
      {
        return vix::Result<
            std::size_t,
            Error>::failure(Error::exhausted);
      }
    };

    class CustomOutput
    {
    public:
      enum class Error
      {
        rejected
      };

      using error_type = Error;

      [[nodiscard]] vix::Result<std::size_t, Error> write(
          std::span<const std::byte>)
      {
        return vix::Result<
            std::size_t,
            Error>::failure(Error::rejected);
      }
    };

    static_assert(vix::Input<CustomInput>);
    static_assert(vix::Output<CustomOutput>);

    static_assert(!std::same_as<
                  vix::input_error_t<CustomInput>,
                  vix::output_error_t<CustomOutput>>);

    SUCCEED();
  }

} // namespace
