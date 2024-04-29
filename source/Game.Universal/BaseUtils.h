#pragma once

/* Includes */

#include <math.h>
#include <string>


/* Types */

typedef float float_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef char char8_t;
typedef unsigned char uchar8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int bool8_t;
typedef std::string string_t;


/* Constants */

constexpr float_t PI = 3.14159265359f;

/* Structs */

typedef struct  Coord2D
{
	float_t x{ 0 };
	float_t y{ 0 };

public:
	Coord2D() {};
	Coord2D(float_t X, float_t Y)
	{
		x = X;
		y = Y;
	};
	Coord2D(const Coord2D&) = default;
	Coord2D& operator=(const Coord2D&) = default;
	Coord2D(Coord2D&&) = default;
	Coord2D& operator=(Coord2D&&) = default;
	~Coord2D() = default;

	struct Coord2D operator+(const struct Coord2D& rhs) const noexcept
	{
		struct Coord2D result;
		result.x = x + rhs.x;
		result.y = y + rhs.y;

		return result;
	}

	struct Coord2D& operator+=(const struct Coord2D& rhs) noexcept
	{
		this->x += rhs.x;
		this->y += rhs.y;

		return *this;
	}

	struct Coord2D operator-(const struct Coord2D& rhs) const noexcept
	{
		struct Coord2D result;
		result.x = x - rhs.x;
		result.y = y - rhs.y;

		return result;
	}

	struct Coord2D& operator-=(const struct Coord2D& rhs) noexcept
	{
		this->x -= rhs.x;
		this->y -= rhs.y;

		return *this;
	}

	struct Coord2D operator*(const float_t rhs) const noexcept
	{
		struct Coord2D result;
		result.x = x * rhs;
		result.y = y * rhs;

		return result;
	}

	struct Coord2D& operator*=(const struct Coord2D& rhs) noexcept
	{
		this->x *= rhs.x;
		this->y *= rhs.y;

		return *this;
	}

	struct Coord2D operator/(const float_t rhs) const noexcept
	{
		struct Coord2D result;
		result.x = x / rhs;
		result.y = y / rhs;

		return result;
	}

	struct Coord2D& operator/=(const float_t& rhs) noexcept
	{
		this->x /= rhs;
		this->y /= rhs;

		return *this;
	}

	bool operator==(const struct Coord2D& rhs) const noexcept
	{
		return (x == rhs.x && y == rhs.y);
	}

	float_t sqrMagnitude() const noexcept
	{
		return (x * x) + (y * y);
	}

	float_t magnitude() const noexcept
	{
		return sqrtf(sqrMagnitude());
	}

	float_t dot(const struct Coord2D& rhs) const noexcept
	{
		float_t result = 0;
		result += x * rhs.x;
		result += y * rhs.y;

		return result;
	}

	struct Coord2D normalize() const noexcept
	{
		struct Coord2D result;
		result.x = x;
		result.y = y;

		float magnitude = result.magnitude();

		if (magnitude != 0)
		{
			result = result / magnitude;
		}

		return result;
	}

	struct Coord2D limit(float_t value) const noexcept
	{
		struct Coord2D result;
		result.x = x;
		result.y = y;

		const float_t magnitude = result.magnitude();

		if (magnitude > value)
		{
			result = result / magnitude;
			result = result * value;
		}

		return result;
	}
}Coord2D;

/* Methods */
namespace DirectXGame
{
	inline bool CompareColors(DirectX::XMFLOAT4 m1, DirectX::XMFLOAT4 m2)
	{
		return m1.x == m2.x && m1.y == m2.y && m1.z == m2.z;
	}
}