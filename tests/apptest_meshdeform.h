#ifndef CLASS_MYEVENTHANDLER
#define CLASS_MYEVENTHANDLER

#include <ncine/IAppEventHandler.h>
#include <ncine/IInputEventHandler.h>
#include <nctl/StaticArray.h>
#include <ncine/Vector2.h>
#include <nctl/UniquePtr.h>

namespace nctl {

class String;

}

namespace ncine {

class AppConfiguration;
class Texture;
class MeshSprite;
class SceneNode;
class Font;
class TextNode;

}

namespace nc = ncine;

/// My nCine event handler
class MyEventHandler :
	public nc::IAppEventHandler,
	public nc::IInputEventHandler
{
  public:
	void onPreInit(nc::AppConfiguration &config) override;
	void onInit() override;
	void onFrameStart() override;

#ifdef __ANDROID__
	void onTouchDown(const nc::TouchEvent &event) override;
	void onTouchMove(const nc::TouchEvent &event) override;
#endif
	void onKeyReleased(const nc::KeyboardEvent &event) override;
	void onMouseButtonPressed(const nc::MouseEvent &event) override;
	void onMouseMoved(const nc::MouseState &state) override;

	void onJoyMappedAxisMoved(const nc::JoyMappedAxisEvent &event) override;
	void onJoyMappedButtonReleased(const nc::JoyMappedButtonEvent &event) override;
	void onJoyDisconnected(const nc::JoyConnectionEvent &event) override;

  private:
	bool pause_;
	float angle_;

	nc::Vector2f scrollOrigin_;
	nc::Vector2f scrollMove_;
	nc::Vector2f joyVectorRight_;

	nctl::UniquePtr<nc::Texture> texture_;
	nctl::UniquePtr<nc::Font> font_;

	nctl::UniquePtr<nctl::String> debugString_;
	nctl::UniquePtr<nc::TextNode> debugText_;
	nctl::UniquePtr<nc::MeshSprite> meshSprite_;

	void checkClick(float x, float y);
};

#endif
