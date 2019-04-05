#include "Color.h"
#include "Colorf.h"
#include <nctl/algorithms.h>

namespace ncine {

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

const Color Color::Black(0, 0, 0, 255);
const Color Color::White(255, 255, 255, 255);
const Color Color::Red(255, 0, 0, 255);
const Color Color::Green(0, 255, 0, 255);
const Color Color::Blue(0, 0, 255, 255);

///////////////////////////////////////////////////////////
// CONSTRUCTORS and DESTRUCTOR
///////////////////////////////////////////////////////////

Color::Color()
{
	set(255, 255, 255, 255);
}

Color::Color(unsigned int red, unsigned int green, unsigned int blue)
	: Color(red, green, blue, 255)
{

}

Color::Color(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha)
{
	set(red, green, blue, alpha);
}

Color::Color(const unsigned int channels[4])
{
	setVec(channels);
}

Color::Color(const Colorf &color)
{
	channels_[0] = static_cast<unsigned char>(color.r() * 255);
	channels_[1] = static_cast<unsigned char>(color.g() * 255);
	channels_[2] = static_cast<unsigned char>(color.b() * 255);
	channels_[3] = static_cast<unsigned char>(color.a() * 255);
}

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void Color::set(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha)
{
	channels_[0] = static_cast<unsigned char>(red);
	channels_[1] = static_cast<unsigned char>(green);
	channels_[2] = static_cast<unsigned char>(blue);
	channels_[3] = static_cast<unsigned char>(alpha);
}

void Color::set(unsigned int red, unsigned int green, unsigned int blue)
{
	set(red, green, blue, 255);
}

void Color::setVec(const unsigned int channels[])
{
	set(channels[0], channels[1], channels[2], channels[3]);
}

void Color::setAlpha(unsigned int alpha)
{
	channels_[3] = static_cast<unsigned char>(alpha);
}

Color &Color::operator=(const Colorf &color)
{
	channels_[0] = static_cast<unsigned char>(color.r() * 255);
	channels_[1] = static_cast<unsigned char>(color.g() * 255);
	channels_[2] = static_cast<unsigned char>(color.b() * 255);
	channels_[3] = static_cast<unsigned char>(color.a() * 255);

	return *this;
}

bool Color::operator==(const Color &color) const
{
	return (r() == color.r() && g() == color.g() &&
	        b() == color.b() && a() == color.a());
}

Color &Color::operator*=(const Color &color)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		float channelValue = (color.channels_[i] / 255.0f) * channels_[i];
		channelValue = nctl::clamp(channelValue, 0.0f, 255.0f);
		channels_[i] = static_cast<unsigned char>(channelValue);
	}

	return *this;
}

Color &Color::operator*=(float scalar)
{
	for (unsigned int i = 0; i < 4; i++)
	{
		float channelValue = scalar * channels_[i];
		channelValue = nctl::clamp(channelValue, 0.0f, 255.0f);
		channels_[i] = static_cast<unsigned char>(channelValue);
	}

	return *this;
}

Color Color::operator*(const Color &color) const
{
	Color result;

	for (unsigned int i = 0; i < 4; i++)
	{
		float channelValue = (color.channels_[i] / 255.0f) * channels_[i];
		channelValue = nctl::clamp(channelValue, 0.0f, 255.0f);
		result.channels_[i] = static_cast<unsigned char>(channelValue);
	}

	return result;
}

Color Color::operator*(float scalar) const
{
	Color result;

	for (unsigned int i = 0; i < 4; i++)
	{
		float channelValue = scalar * channels_[i];
		channelValue = nctl::clamp(channelValue, 0.0f, 255.0f);
		result.channels_[i] = static_cast<unsigned char>(channelValue);
	}

	return result;
}

}

