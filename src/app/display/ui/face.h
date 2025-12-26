#pragma once

#include "../interface.h"
#include <Face.h>

class FaceDrawer : public DisplayDrawer {
public:
	FaceDrawer(U8G2* display = nullptr): _display(display), _width(128), _height(64) {
		_face = new Face(display, _width, _height, 40);
		_face->Expression.GoTo_Normal();
		_face->LookFront();

		// Normal emotions
		_face->Behavior.SetEmotion(eEmotions::Normal, 1.0);
		_face->Behavior.SetEmotion(eEmotions::Unimpressed, 1.0);
		_face->Behavior.SetEmotion(eEmotions::Focused, 1.0);
		_face->Behavior.SetEmotion(eEmotions::Skeptic, 1.0);

		// Happy emotions
		_face->Behavior.SetEmotion(eEmotions::Happy, 1.0);
		_face->Behavior.SetEmotion(eEmotions::Glee, 1.0);
		_face->Behavior.SetEmotion(eEmotions::Awe, 1.0);

		// Sad emotions
		_face->Behavior.SetEmotion(eEmotions::Sad, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Worried, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Sleepy, 0.2);

		// Other emotions
		_face->Behavior.SetEmotion(eEmotions::Angry, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Annoyed, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Surprised, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Frustrated, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Suspicious, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Squint, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Furious, 0.2);
		_face->Behavior.SetEmotion(eEmotions::Scared, 0.2);

		_face->Behavior.Timer.SetIntervalMillis(30000);
		_face->Blink.Timer.SetIntervalMillis(5000);
		_face->Look.Timer.SetIntervalMillis(15000);

		_face->RandomBlink = true;
		_face->RandomBehavior =
		_face->RandomLook =
			false;
	}

	~FaceDrawer() {
		delete _face;
	}

	inline void setExpression(eEmotions emotion) {
		_face->Behavior.GoToEmotion(emotion);
	}

	inline void lookFront() {
		_face->LookFront();
	}

	inline void lookRight() {
		_face->LookRight();
	}

	inline void lookLeft() {
		_face->LookLeft();
	}

	inline void lookTop() {
		_face->LookTop();
	}

	inline void lookBottom() {
		_face->LookBottom();
	}


	inline void draw() override {
		_display->clearBuffer();
		_face->Update();
	}

private:
	U8G2* _display;
	Face* _face;
	int _width;
	int _height;
};